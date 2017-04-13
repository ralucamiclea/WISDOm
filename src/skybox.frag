#version 150

out vec4 out_Color;
in vec2 exTexCoord;

uniform sampler2D texUnit;

void main(void)
{
		out_Color = texture(texUnit, exTexCoord);
}
