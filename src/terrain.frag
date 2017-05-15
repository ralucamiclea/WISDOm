#version 150

out vec4 outColor;

in vec2 texCoord;
in vec3 exColor;

uniform sampler2D grass, water, stones, map;

void main(void)
{
	vec4 m = texture(map, texCoord*0.0045);
	outColor = exColor[0] * (texture(grass, texCoord) * m.r + texture(water, texCoord) * m.g + texture(stones, texCoord) * m.b);
}
