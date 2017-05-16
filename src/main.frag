#version 150

out vec4 out_Color;

in vec2 exTexCoord;

uniform sampler2D texUnit1,texUnit2;


void main(void)
{
	out_Color = texture(texUnit1, exTexCoord) * 0.7 + texture(texUnit2, exTexCoord) * 0.3;
}
