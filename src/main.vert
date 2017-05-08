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

void main() {

	// Texture
	exTexCoord = inTexCoord;

	int row = gl_InstanceID / edge;
	vec4 offs = vec4(gl_InstanceID % edge, gl_InstanceID / edge, 0, 0);

	gl_Position = frustum * camera  * translation * rotation * scaling *  vec4(in_Position, 1.0) + offs * 5.0;;

	// Model to View
	//gl_Position = frustum * camera  * translation * rotation * scaling * vec4(in_Position, 1.0);
	//gl_Position = frustum * camera  * rotation * translation * vec4(in_Position, 1.0);
	}
