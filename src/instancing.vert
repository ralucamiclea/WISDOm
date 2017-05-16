#version 150


// "position" and "normal" are regular vertex attributes
in vec3 in_Position;
in vec3 particle_position;
in vec4 particle_color;

// Color is a per-instance attribute
//in vec4 color;

// model_matrix will be used as a per-instance transformation
// matrix. Note that a mat4 consumes 4 consecutive locations, so
// this will actually sit in locations, 3, 4, 5, and 6.
//in mat4 model_matrix;

//uniform mat4 model_matrix;
uniform mat4 frustum;
uniform mat4 camera;

out vec2 texCoord;
out vec4 attributes;
//out vec4 instance_color;

void main(void)
{
    // Transform position by the model-view matrix, then by the
    // projection matrix.

	vec4 column0 = vec4(1.0, 0.0, 0.0, 0.0);
	vec4 column1 = vec4(0.0, 1.0, 0.0, 0.0);
	vec4 column2 = vec4(0.0, 0.0, 1.0, 0.0);
	vec4 column3 = vec4(particle_position.x, particle_position.y, particle_position.z, 1.0);
	//column3.x += 10 * gl_InstanceID;
	mat4 model_matrix = mat4(column0, column1, column2, column3); // sets columns of matrix n

	gl_Position =  frustum * camera * model_matrix *  vec4(in_Position, 1.0);
	//instance_color = color;
	texCoord.s = in_Position.x+0.5;
	texCoord.t = in_Position.y-0.2;
	attributes = particle_color;
}
