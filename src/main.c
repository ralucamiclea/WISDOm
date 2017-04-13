#ifdef __APPLE__
	#include <OpenGL/gl3.h>
	// Linking hint for Lightweight IDE
	// uses framework Cocoa
#endif
#include "../common/Linux/MicroGlut.h"
#include "../common/GL_utilities.h"
#include "../common/loadobj.h"
#include "../common/LoadTGA.h"
#include "../common/VectorUtils3.h"
#include <math.h>

#define near 1.0
#define far 300.0
#define right 2
#define left -2
#define top 2
#define bottom -2

// Globals

float eyeX, eyeY, eyeZ; //camera position
float lookAtX, lookAtY, lookAtZ; //camera direction
float angle; //angle of rotation for the camera direction
float horizontalAngle;
float verticalAngle;

mat4 frustum_matrix, camera_placement, rotation, translation, scaling;
Model *dog, *ground, *skybox;
GLuint grass_tex, skybox_tex, dog_tex;

GLuint program, skybox_program; // Reference to shader program
GLfloat t; //time

void init(void)
{
		//load textures
		LoadTGATextureSimple("../tex/grass.tga", &grass_tex);
		LoadTGATextureSimple("../tex/skybox.tga", &skybox_tex);
		LoadTGATextureSimple("../tex/dog.tga", &dog_tex);

		//stationary objects
		ground = LoadModelPlus("../obj/ground.obj");
		dog = LoadModelPlus("../obj/dog.obj");
		skybox = LoadModelPlus("../obj/skybox.obj");

		dumpInfo();

		// GL inits
		glClearColor(0.2,0.2,0.6,0);
		glutInitDisplayMode(GLUT_DEPTH);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		printError("GL inits");

		// Load and compile shader
		program = loadShaders("main.vert", "main.frag");
		skybox_program = loadShaders("skybox.vert", "skybox.frag");


		printError("init skybox shader");
		glUseProgram(skybox_program);
		// Choose texture unit for skybox
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, skybox_tex);
		glUniform1i(glGetUniformLocation(skybox_program, "texUnit"), 0); // Texture unit 0

		printError("init main shader");
		glUseProgram(program);

		//grass texture
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, grass_tex);
		glUniform1i(glGetUniformLocation(program, "texUnit"), 1); // Texture unit 1

		//dog
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, dog_tex);
		glUniform1i(glGetUniformLocation(program, "texUnit"), 2); // Texture unit 2
}

void display(void)
{
		t = (GLfloat)glutGet(GLUT_ELAPSED_TIME);

		frustum_matrix = frustum(left, right, bottom, top, near, far);
		camera_placement = lookAt(eyeX, eyeY, eyeZ, eyeX + lookAtX, eyeY + lookAtY, eyeZ + lookAtZ, 0, 1, 0);
		//camera_placement = lookAt(eyeX, eyeY, eyeX, eyeX + lookAtX, eyeY + lookAtY, eyeX + lookAtZ, 0, 1, 0);
		//camera_placement = lookAt(eyeX, eyeY, eyeX, eyeX + cos(verticalAngle)*sin(horizontalAngle), sin(verticalAngle), eyeX + cos(verticalAngle)*cos(horizontalAngle), 0, 1, 0);

		//skybox
		glUseProgram(skybox_program);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		printError("skybox");
		glUniformMatrix4fv(glGetUniformLocation(skybox_program, "frustum"), 1, GL_TRUE, frustum_matrix.m);
		glUniformMatrix4fv(glGetUniformLocation(skybox_program, "camera"), 1, GL_TRUE, camera_placement.m);
		glUniform1i(glGetUniformLocation(skybox_program, "texUnit"), 0);
		DrawModel(skybox, skybox_program, "in_Position", 0L, "inTexCoord");

		glUseProgram(program);
		glEnable(GL_DEPTH_TEST);
		rotation = Ry(45);
		translation = T(0,0,0);
		scaling = S(3,3,3);

		// clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUniformMatrix4fv(glGetUniformLocation(program, "frustum"), 1, GL_TRUE, frustum_matrix.m);
		glUniformMatrix4fv(glGetUniformLocation(program, "camera"), 1, GL_TRUE, camera_placement.m);
		glUniformMatrix4fv(glGetUniformLocation(program, "rotation"), 1, GL_TRUE, rotation.m);
		glUniformMatrix4fv(glGetUniformLocation(program, "translation"), 1, GL_TRUE, translation.m);
		glUniformMatrix4fv(glGetUniformLocation(program, "scaling"), 1, GL_TRUE, scaling.m);

		//ground
		printError("ground");
		glUniform1i(glGetUniformLocation(program, "texUnit"), 1);
		DrawModel(ground, program, "in_Position", 0L, "inTexCoord");

		//dog
		scaling = S(0.1,0.1,0.1);
		translation = T(5, 6, -50);
		glUniformMatrix4fv(glGetUniformLocation(program, "scaling"), 1, GL_TRUE, scaling.m);
		glUniform1i(glGetUniformLocation(program, "texUnit"), 2);
		glUniformMatrix4fv(glGetUniformLocation(program, "translation"), 1, GL_TRUE, translation.m);
		DrawModel(dog, program, "in_Position", 0L, "inTexCoord");

		glutSwapBuffers();
}

void OnTimer(int value)
{
		glutPostRedisplay();
		glutTimerFunc(20, &OnTimer, value);

		if (glutKeyIsDown('w')){ //move camera forward
		  eyeX += lookAtX * 0.1f;
		  eyeZ += lookAtZ * 0.1f;
		}

		if (glutKeyIsDown('s')){ //move camera backwards
		  eyeX -= lookAtX * 0.1f;
		  eyeZ -= lookAtZ * 0.1f;
		}

		if (glutKeyIsDown('r')){ //move camera up
			angle -= 0.01f;
			eyeY += lookAtY * 0.1f;
			lookAtY = -cos(angle);
		}

		if (glutKeyIsDown('f')){ //move camera down
			angle += 0.01f;
			eyeY -= lookAtY * 0.1f;
			lookAtY = -cos(angle);
		}

		if (glutKeyIsDown('a')){ //rotate camera left around Y axis
		  angle -= 0.01f;
		  lookAtX = sin(angle);
		  lookAtZ = -cos(angle);
		}

		if (glutKeyIsDown('d')){ //rotate camera right around Y axis
		  angle += 0.01f;
		  lookAtX = sin(angle);
		  lookAtZ = -cos(angle);
		}

}

/*
void SpecialKeysMove(unsigned char key, int x, int y) {

		switch (key) {
			case GLUT_KEY_LEFT :
				angle -= 0.01f;
				lookAtX = sin(angle);
				lookAtZ = -cos(angle);
				break;
			case GLUT_KEY_RIGHT :
				angle += 0.01f;
				lookAtX = sin(angle);
				lookAtZ = -cos(angle);
				break;
			case GLUT_KEY_UP :
				eyeX += lookAtX * 0.1f;
				eyeZ += lookAtZ * 0.1f;
				break;
			case GLUT_KEY_DOWN :
				eyeX -= lookAtX * 0.1f;
				eyeZ -= lookAtZ * 0.1f;
				break;
		}
}

void mouseMove(int x, int y) {

		// update deltaAngle
		horizontalAngle -= (x - 300) * 0.001f;
		verticalAngle -= (y -300) * 0.001f;

		if(x!=300 && y!=300){
			glutWarpPointer(	300 , 300 );
		}
}
*/

int main(int argc, char *argv[])
{
  eyeX = 0.0f; eyeY = 6.0f; eyeZ = 6.0f;
  lookAtX = 0.0f; lookAtY = 0.0f; lookAtZ = -1.0f;
	horizontalAngle = 3.14;
	verticalAngle = 0.0;

  glutInit(&argc, argv);
  glutInitContextVersion(3, 2);
  glutDisplayFunc(display);
  glutCreateWindow ("Cool 3D World");
	//glutPassiveMotionFunc(mouseMove);
	//glutKeyboardFunc(SpecialKeysMove);
  init();
  glutTimerFunc(20, &OnTimer, 0);
  glutMainLoop();
  return 0;
}
