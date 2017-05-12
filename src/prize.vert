#version 150

in  vec3 in_Normal;
in  vec3 in_Position;
in  vec2 inTexCoord;

out vec2 exTexCoord;
out vec3 exNormal; // Phong
out vec3 exSurface; // Phong (specular)
out vec3 camera_pos;

uniform mat4 translation;
uniform mat4 rotation;
uniform mat4 camera;
uniform mat4 frustum;
uniform vec3 xyz;


mat4 modelviewMatrix;


void main() {

	modelviewMatrix = camera  * translation  * rotation;
	
	// Texture
	exTexCoord = inTexCoord;
	
	//camera direction
	camera_pos = xyz;

	exNormal = inverse(transpose(mat3(modelviewMatrix))) * in_Normal;
	exSurface = vec3(modelviewMatrix * vec4(in_Position, 1.0));

	gl_Position = frustum * modelviewMatrix * vec4(in_Position, 1.0);

}
