#version 430 core

in vec2 fragPosition;
out vec4 outColour;

layout (binding = 0) uniform sampler2D positionsTexture;
layout (binding = 1) uniform sampler2D previousCast;

uniform vec3 cameraPosition;
uniform float gameTime;
uniform int frames;
uniform vec4 screenPatch;

struct PointLight {
  vec4 position;
  vec4 colour;
};
layout(std430, binding = 9) buffer PointLightsBuffer{ PointLight pointLights[]; };
uniform int numLights;

struct Tri{
    vec4 v0;
    vec4 v1;
    vec4 v2;
    vec4 pad;
};
layout(std430, binding = 10) buffer TrisBuffer{ Tri tris[]; };

struct Ray
{
    vec3 origin;
    vec3 direction;
};

struct RayHit
{
    float distance;
    vec3 position;
    vec3 normal;
    vec3 tangent;
    int hits;
};

struct AABB
{
    vec4 min;
    vec4 max;
};

struct BVHNode {
    AABB aabb;
    int leftIdx;
    int rightIdx;
    int tri2;
    int tri3;
    int isLeaf;
    int parentIdx;
    int pad0_, pad1_;
};
layout(std430, binding = 11) buffer BVHNodesBuffer{ BVHNode nodes[]; };

float random(float co)
{
   return fract(sin(co * fragPosition.x * fragPosition.y * 12.9898) * 43758.5453);
}

bool testAABB(const Ray ray, const AABB aabb)
{
    float tmin, tmax;
    vec3 t0, t1;
    vec3 invDir = 1.0f / ray.direction;
    t0 = (vec3(aabb.min) - ray.origin) * invDir;
    t1 = (vec3(aabb.max) - ray.origin) * invDir;


    tmin = min(t0.x, t1.x);
    tmax = max(t0.x, t1.x);

    tmin = max(tmin, min(t0.y, t1.y));
    tmax = min(tmax, max(t0.y, t1.y));

    tmin = max(tmin, min(t0.z, t1.z));
    tmax = min(tmax, max(t0.z, t1.z));

    return tmax >= tmin;
}

bool testTri(in int triIdx, in const Ray ray, inout RayHit hit)
{
    if(triIdx < 0)
        return false;

    const float e = 0.000001f;
    vec3 v0, edge1, edge2;
	vec3 tvec, pvec, qvec;
	float det, inv_det;

    v0 = vec3(tris[triIdx].v0);
    edge1 = vec3(tris[triIdx].v1) - v0;
    edge2 = vec3(tris[triIdx].v2) - v0;

	pvec = cross(ray.direction, edge2);
	det = dot(edge1, pvec);

	if(det > -e && det < e)
		return false;
	inv_det = 1.0f / det;

	tvec = ray.origin - v0;
	float u = dot(tvec, pvec) * inv_det;
	if(u < 0.0f || u > 1.0f)
		return false;

	qvec = cross(tvec, edge1);
	float v = dot(ray.direction, qvec) * inv_det;
	if(v < 0.0f || (u + v > 1.0f))
		return false;

	float d = dot(edge2, qvec) * inv_det;
	if(d < e)
		return false;

    if(d > hit.distance)
        return true;

    hit.hits++;
	hit.distance = d;
    hit.position = ray.origin + ray.direction * d;
    hit.normal = normalize(cross(edge1, edge2));
    hit.tangent = edge1;
	return true;
}

/**
 * Function that traverses back up the BVH stack to find the next branch
 * to traverse down. Can be determined by which side of the branch we're
 * coming up from. If we're coming up from the left branch, take the right
 * branch as the next search node. Otherwise, if we're coming up from the
 * right branch, go and repeat this process for the next parent up. Do this
 * until we hit a valid search node, or we hit the root node. In which case
 * the BVH was exhausted.
 */
int getNextSearchNode(int currentNode)
{
    while(true)
    {
        int parentIdx = nodes[currentNode].parentIdx;
        if(parentIdx == -1)
            return -1;

        if(nodes[parentIdx].leftIdx == currentNode)
        {
            return nodes[parentIdx].rightIdx;
        }
        else if(nodes[parentIdx].rightIdx == currentNode)
        {
            currentNode = parentIdx;
        }
    }
}

bool traverse(in const Ray ray, inout RayHit hit, in bool anyOcclusion)
{
    int nodeIdx = 0;
    bool checkLeft = true;
    while(nodeIdx >= 0)
    {
        if(!testAABB(ray, nodes[nodeIdx].aabb))
        {
            nodeIdx = getNextSearchNode(nodeIdx);
        }
        else
        {
            if(nodes[nodeIdx].isLeaf != 0)
            {
                testTri(nodes[nodeIdx].leftIdx, ray, hit);
                testTri(nodes[nodeIdx].rightIdx, ray, hit);
                testTri(nodes[nodeIdx].tri2, ray, hit);
                testTri(nodes[nodeIdx].tri3, ray, hit);

                if(hit.hits > 0 && anyOcclusion)
                    // You might think that returning eary would optimise this
                    // function, but don't forget that the GPU shader groups
                    // operate in lockstep, so divergence is bad.
                    return true;
                else
                    nodeIdx = getNextSearchNode(nodeIdx);
            }
            else
            {
                nodeIdx = nodes[nodeIdx].leftIdx;
            }
        }
    }
    return hit.hits > 0;
}

bool traverse(in const Ray ray, inout RayHit hit)
{
    return traverse(ray, hit, false);
}

bool occluded(in vec3 a, in vec3 b)
{
    Ray ray;
    ray.direction = b - a;
    float d = length(ray.direction);
    ray.direction = normalize(ray.direction);
    ray.origin = a;
    RayHit hit;
    hit.hits = 0;
    hit.distance = d;
    return traverse(ray, hit, true);
}

vec3 lightAccumulation(in const RayHit hit)
{
    vec3 colour = vec3(0.0f);

    float lightRadius = 0.5f;
    int lightSamples = 16;

    for(int l = 0; l < numLights; ++l)
    {
        PointLight light = pointLights[l];
        for(int i = 0; i < lightSamples; ++i)
        {
            float rx = random(float(l) + float(i) * gameTime);
            float ry = random(float(l) + float(i) * gameTime + 0.1f);
            float rz = random(float(l) + float(i) * gameTime + 0.2f);
            vec3 randomOffset = vec3(rx, ry, rz);
            randomOffset *= 2.0f;
            randomOffset -= vec3(1.0f);
            randomOffset *= lightRadius;

            vec3 lightPosition = vec3(light.position) + randomOffset;
            vec3 lightDirection = lightPosition - hit.position;
            float lightDistanceRcp = 1.0f / length(lightPosition - hit.position);
            lightDirection = lightDirection * lightDistanceRcp;

            if(!occluded(hit.position + lightDirection * 0.0001f, lightPosition))
            {
                float lightPower = light.colour.w;
                float falloff = lightPower * lightDistanceRcp;
                float lambertian = clamp(dot(hit.normal, lightDirection), 0.0f, 1.0f);
                vec3 diffuseTerm = vec3(0.8f, 0.8f, 0.8f) * falloff * vec3(light.colour) * lambertian;
                colour += diffuseTerm * (1.0f / float(lightSamples));
            }
        }
    }
    return colour;
}

vec3 CosineSampleHemisphere(float u1, float u2)
{
    float r = sqrt(u1);
    float theta = 2.0f * 3.142f * u2;

    float x = r * cos(theta);
    float y = r * sin(theta);

    return vec3(x, y, sqrt(max(0.0f, 1.0f - u1)));
}

void main()
{
    vec2 p = (fragPosition + vec2(1.0f)) * 0.5f;

    if(p.x < screenPatch.x || p.x > screenPatch.z || p.y < screenPatch.y || p.y > screenPatch.w)
        discard;

    vec3 position = vec3(texture(positionsTexture, p));


    vec3 lightPosition = vec3(pointLights[0].position);

    Ray ray;
    ray.direction = normalize(position - cameraPosition);
    ray.origin = cameraPosition;

    RayHit hit;
    hit.hits = 0;
    hit.distance = 1.0 / 0.0; // Infinity

    const int bounces = 1;
    int currentBounce = 0;
    float currentBounceWeight = 1.0f;
    vec3 finalColour;
    for(int currentBounce = 0; currentBounce <= bounces; ++currentBounce)
    {
        hit.hits = 0;
        hit.distance = 1.0 / 0.0; // Infinity

        if(traverse(ray, hit))
        {
            hit.position = ray.direction * hit.distance + ray.origin - ray.direction * 0.01f;

            finalColour += lightAccumulation(hit) * currentBounceWeight;

            float rx = random(gameTime);
            float ry = random(gameTime + 0.01f);
            vec3 randomHemisphereVector = CosineSampleHemisphere(rx, ry);

            ray.origin = hit.position;
            vec3 biTangent = cross(hit.normal, hit.tangent);
            mat3 normalTransform = mat3(hit.tangent, biTangent, hit.normal);
            ray.direction = normalize(normalTransform * randomHemisphereVector);

            currentBounceWeight *= 1.0f / (2.0f * 3.142f);
        }
        else
        {
            break;
        }
    }

    finalColour += vec3(texture(previousCast, p)) * float(frames);
    finalColour /= float(frames + 1);
    outColour = vec4(finalColour, 1.0f);
}
