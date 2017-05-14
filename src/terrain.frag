#version 150

out vec4 outColor;

in vec2 texCoord;
in vec3 exColor;

uniform sampler2D grass, water, dirt, map;

void main(void)
{
	vec4 m = texture(map, texCoord);
	outColor = exColor[0] * texture(grass, texCoord);
	//outColor = exColor[0] * (texture(grass, texCoord) * m.r + texture(water, texCoord) * m.g + texture(dirt, texCoord) * m.b);
	//outColor = exColor[0] * (texture(grass, texCoord) + texture(water, texCoord) + texture(dirt, texCoord));
}
