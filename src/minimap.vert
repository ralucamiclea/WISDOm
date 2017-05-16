#version 150

in  vec3 in_Position;
in vec2 inTexCoord;
uniform mat4 translation;
uniform mat4 frustum;
uniform float angle;
uniform float slope;
out vec2 texCoord;

void main(void)
{
	mat4 r;
	float a = angle + gl_InstanceID * 0.5;
	float rr = 1.0 - slope * gl_InstanceID * 0.01;
	r[0] = rr*vec4(cos(a), -sin(a), 0, 0);
	r[1] = rr*vec4(sin(a), cos(a), 0, 0);
	r[2] = vec4(0, 0, 1, 0);
	r[3] = vec4(0, 0, 0, 1);
	texCoord.s = in_Position.x + 0.5;
	texCoord.t = in_Position.y - 0.5;
	gl_Position =  r * translation *  vec4(in_Position, 1.0);
}
