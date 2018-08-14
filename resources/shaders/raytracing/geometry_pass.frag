#version 430 core

in vec3 fragNormal;
in vec3 fragPosition;
in vec2 fragTexCoord;
in mat3 normalMapTransform;

uniform sampler2D opacityTexture;

uniform bool hasNormalMap;
uniform bool hasOpacityMap;
uniform bool hasEmmisiveTexture;

layout (location = 0) out vec4 positionGBuffer;

void main(){
    float opacity = 1.0f;
    if(hasOpacityMap)
    {
        opacity = texture(opacityTexture, fragTexCoord).r;
        if(opacity < 1.0f)
            discard;
    }

    positionGBuffer = vec4(fragPosition, opacity);
}
