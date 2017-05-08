#version 150

in  vec3 in_Position;
in  vec2 inTexCoord;

out vec2 exTexCoord;

uniform mat4 camera;
uniform mat4 frustum;
uniform mat4 translation;

void main() {

	// Texture
	exTexCoord = inTexCoord;
	
	gl_Position = frustum * camera  *  translation * vec4(in_Position, 1.0);
}
