#version 430

in vec2 vPosition;

out vec2 fragPosition;

void main()
{
	fragPosition = vPosition;
	gl_Position = vec4(vPosition, 0.0f, 1.0f);
}
