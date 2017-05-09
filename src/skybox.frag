#version 150

out vec4 out_Color;
in vec2 exTexCoord;

uniform sampler2D texUnit;
/*
uniform sampler2D texUnit1;
uniform sampler2D texUnit2;
uniform sampler2D texUnit3;
uniform sampler2D texUnit4;
uniform sampler2D texUnit5;
uniform sampler2D texUnit6;
*/

void main(void)
{
	out_Color = texture(texUnit, exTexCoord);
	//out_Color = texture(texUnit1, exTexCoord) + texture(texUnit2, exTexCoord) + texture(texUnit3, exTexCoord) + texture(texUnit4, exTexCoord) + texture(texUnit5, exTexCoord) + texture(texUnit6, exTexCoord);
}
