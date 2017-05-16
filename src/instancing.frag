#version 150

out vec4 out_Color;
uniform sampler2D tex;

in vec2 texCoord;
in vec4 attributes;


void main(void)
{

	out_Color = texture(tex, texCoord) - attributes;
	//out_Color = attributes;
}
