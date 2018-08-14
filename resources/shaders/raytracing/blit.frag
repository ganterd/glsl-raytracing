#version 430 core

in vec2 fragPosition;
out vec4 outColour;

uniform sampler2D newFrame;
uniform vec4 screenPatch;

void main()
{
    vec2 p = (fragPosition + vec2(1.0f)) * 0.5f;

    vec4 colour = texture(newFrame, p);
    if(p.x > screenPatch.x && p.y > screenPatch.y && p.x < screenPatch.z && p.y < screenPatch.w)
        colour += vec4(0.2f, 0.0f, 0.0f, 0.0f);

    vec4 gammaCorrected = pow(colour, vec4(1.0f / 2.2f));
    outColour = vec4(gammaCorrected.rgb, colour.a);
}
