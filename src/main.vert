#version 150

in  vec3 in_Position;
in  vec2 inTexCoord;

out vec2 exTexCoord;

uniform mat4 scaling;
uniform mat4 translation;
uniform mat4 rotation;
uniform mat4 camera;
uniform mat4 frustum;

uniform int edge;
uniform int count;
uniform int distance;

void main() {

	// Texture
	exTexCoord = inTexCoord;

	int row = gl_InstanceID / edge;
	vec4 offs = vec4(gl_InstanceID % edge, gl_InstanceID / edge, 0, 0);
	
	vec4 position = vec4(in_Position, 1.0) + offs * distance;

	gl_Position = frustum * camera  * translation * rotation * scaling *  position;
	}
