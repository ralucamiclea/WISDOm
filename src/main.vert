#version 150

in  vec3 in_Position;
in  vec2 inTexCoord;

out vec2 exTexCoord;

uniform mat4 scaling;
uniform mat4 translation;
uniform mat4 rotation;
uniform mat4 camera;
uniform mat4 frustum;



void main() {

  // Texture
  exTexCoord = inTexCoord;

  // Model to View
  gl_Position = frustum * camera  * rotation * scaling * translation * vec4(in_Position, 1.0);
  //gl_Position = frustum * camera  * rotation * translation * vec4(in_Position, 1.0);
}
