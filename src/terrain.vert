#version 150

in  vec3 in_Position;
in  vec3 in_Normal;
in vec2 inTexCoord;


out vec2 texCoord;
out vec3 exColor;

uniform mat4 camera;
uniform mat4 frustum;
uniform mat4 translation;

void main(void)
{
	// Light
	const vec3 light = normalize(vec3(-1, -1, 1));
	float shade;
	shade = dot(normalize(in_Normal), light);
	shade = clamp(shade, 0, 1);
	exColor = vec3(shade);

	texCoord = inTexCoord;
	gl_Position = frustum * camera * translation * vec4(in_Position, 1.0);
}
