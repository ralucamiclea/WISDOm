#version 150

in  vec3 in_Position;
in  vec2 inTexCoord;

out vec2 exTexCoord;

uniform mat4 camera;
uniform mat4 frustum;
uniform mat4 move_skybox;

void main() {

  // Texture
  exTexCoord = inTexCoord;

  gl_Position = frustum * camera  *  move_skybox * vec4(in_Position, 1.0);
}
