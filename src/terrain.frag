#version 150

out vec4 outColor;

in vec2 texCoord;
in vec3 exColor;

uniform sampler2D texUnit;

void main(void)
{
	outColor = exColor[0] * texture(texUnit, texCoord);
}
