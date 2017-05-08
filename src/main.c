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

#define HEIGHT 600
#define WIDTH 600



// Globals
vec3 position = {0.0,50.0,0.0};
vec3 direction = {0.0,0.0,0.0};
vec3 right = {0.0,0.0,0.0};
vec3 up = {0.0,0.0,0.0};
float angle; //angle of rotation for the camera direction
float horizontalAngle = 3.14;
float verticalAngle = 0.0;

vec3 posObj; //position of the moving cube
mat4 frustum_matrix, camera_placement, rotation, translation, scaling, camera_skybox;
Model *dog, *terrain, *skybox, *octagon;
GLuint grass_tex, skybox_tex, dog_tex;
TextureData terrain_tex;

GLuint program, skybox_program, terrain_program; // Reference to shader program
GLfloat t; //time
float obj_t=0, cam_t=0;

int texWidth=0; //for calculateHeight function

int edge = 5; //for object instancing


//TERRAIN GENERATION function
Model* GenerateTerrain(TextureData *tex)
{
	int vertexCount = tex->width * tex->height;
	int triangleCount = (tex->width-1) * (tex->height-1) * 2;
	int x, z;

	GLfloat *vertexArray = malloc(sizeof(GLfloat) * 3 * vertexCount);
	GLfloat *normalArray = malloc(sizeof(GLfloat) * 3 * vertexCount);
	GLfloat *texCoordArray = malloc(sizeof(GLfloat) * 2 * vertexCount);
	GLuint *indexArray = malloc(sizeof(GLuint) * triangleCount*3);

	printf("bpp %d\n", tex->bpp);
	for (x = 0; x < tex->width; x++)
		for (z = 0; z < tex->height; z++)
		{
// Vertex array. You need to scale this properly
			vertexArray[(x + z * tex->width)*3 + 0] = x / 1.0;
			vertexArray[(x + z * tex->width)*3 + 1] = tex->imageData[(x + z * tex->width) * (tex->bpp/8)] / 30.0;
			vertexArray[(x + z * tex->width)*3 + 2] = z / 1.0;

			//for the calculateHeight funciton
			texWidth = tex->width;
			// Normal vectors. You need to calculate these.
						vec3 left;
						vec3 us;
						vec3 top;
						us =(vec3) {vertexArray[(x + z * tex->width)*3 + 0],
			                  vertexArray[(x + z * tex->width)*3 + 1],
			                  vertexArray[(x + z * tex->width)*3 + 2]};
						if(x - 1 < 0){
							left =(vec3){0, 1, 0};
						}
						else{
							left =(vec3) {vertexArray[(x-1 + z * tex->width)*3 + 0],
			                      vertexArray[(x-1 + z * tex->width)*3 + 1],
			                      vertexArray[(x-1 + z * tex->width)*3 + 2]};
						}
						if (z - 1 < 0){
							top=(vec3){0, 1, 0};
						}
						else {
							top = (vec3){vertexArray[(x + (z-1) * tex->width)*3 + 0],
			                     vertexArray[(x + (z-1) * tex->width)*3 + 1],
			                     vertexArray[(x + (z-1) * tex->width)*3 + 2]};
						}
						vec3 leftV = VectorSub(left, us);
						vec3 topV = VectorSub(top, us);
						vec3 normal = Normalize(CrossProduct(leftV, topV));

						normalArray[(x + z * tex->width)*3 + 0] = normal.x;
						normalArray[(x + z * tex->width)*3 + 1] = normal.y;
						normalArray[(x + z * tex->width)*3 + 2] = normal.z;
// Texture coordinates. You may want to scale them.
			texCoordArray[(x + z * tex->width)*2 + 0] = x; // (float)x / tex->width;
			texCoordArray[(x + z * tex->width)*2 + 1] = z; // (float)z / tex->height;
	}
	for (x = 0; x < tex->width-1; x++)
		for (z = 0; z < tex->height-1; z++)
		{
		// Triangle 1
			indexArray[(x + z * (tex->width-1))*6 + 0] = x + z * tex->width;
			indexArray[(x + z * (tex->width-1))*6 + 1] = x + (z+1) * tex->width;
			indexArray[(x + z * (tex->width-1))*6 + 2] = x+1 + z * tex->width;
		// Triangle 2
			indexArray[(x + z * (tex->width-1))*6 + 3] = x+1 + z * tex->width;
			indexArray[(x + z * (tex->width-1))*6 + 4] = x + (z+1) * tex->width;
			indexArray[(x + z * (tex->width-1))*6 + 5] = x+1 + (z+1) * tex->width;
		}

	// End of terrain generation

	// Create Model and upload to GPU:

	Model* model = LoadDataToModel(
			vertexArray,
			normalArray,
			texCoordArray,
			NULL,
			indexArray,
			vertexCount,
			triangleCount*3);

	return model;
}

//HEIGHT function
GLfloat calculateHeight(Model *groundMap, vec3 position){
	
	GLfloat height = 0;
	height = groundMap->vertexArray[((int)position.x + (int)position.z * texWidth)*3 + 1];
	return height;
}

//SMOOTH Movement
float smoothen(float speed, float smoothing, vec3 posObj, float old_height){
	
	//interpolate inbetween height stages of map
	//posObj smoothing steps ahead
	float slope = 0;
	vec3 newposObj = posObj;
	//new position
	newposObj.x = posObj.x+(speed*smoothing);
	newposObj.z = posObj.z+(speed*smoothing);
	//calculate slope
	slope = 1.3*(calculateHeight(terrain, newposObj)-old_height);
	posObj.y = slope*speed + old_height;
	old_height = posObj.y;
	return posObj.y;
}

//INSTANCING function
void DrawModelInstanced(Model *m, GLuint program, char* vertexVariableName, char* normalVariableName, char* texCoordVariableName, int count)
{
	if (m != NULL)
	{
		GLint loc;
		
		glBindVertexArray(m->vao);	// Select VAO

		glBindBuffer(GL_ARRAY_BUFFER, m->vb);
		loc = glGetAttribLocation(program, vertexVariableName);
		if (loc >= 0)
		{
			glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, 0); 
			glEnableVertexAttribArray(loc);
		}
		else
			fprintf(stderr, "DrawModel warning: '%s' not found in shader!\n", vertexVariableName);
		
		if (normalVariableName!=NULL)
		{
			loc = glGetAttribLocation(program, normalVariableName);
			if (loc >= 0)
			{
				glBindBuffer(GL_ARRAY_BUFFER, m->nb);
				glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
				glEnableVertexAttribArray(loc);
			}
			else
				fprintf(stderr, "DrawModel warning: '%s' not found in shader!\n", normalVariableName);
		}
	
		// VBO for texture coordinate data NEW for 5b
		if ((m->texCoordArray != NULL)&&(texCoordVariableName != NULL))
		{
			loc = glGetAttribLocation(program, texCoordVariableName);
			if (loc >= 0)
			{
				glBindBuffer(GL_ARRAY_BUFFER, m->tb);
				glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, 0, 0);
				glEnableVertexAttribArray(loc);
			}
			else
				fprintf(stderr, "DrawModel warning: '%s' not found in shader!\n", texCoordVariableName);
		}

		glDrawElementsInstanced(GL_TRIANGLES, m->numIndices, GL_UNSIGNED_INT, 0L, count);
	}
}

void init(void)
{
	//load textures
	LoadTGATextureSimple("../tex/grass.tga", &grass_tex);
	LoadTGATextureSimple("../tex/skybox.tga", &skybox_tex);
	LoadTGATextureSimple("../tex/dog.tga", &dog_tex);
	LoadTGATextureData("../tex/fft-terrain.tga", &terrain_tex);
	terrain = GenerateTerrain(&terrain_tex);

	//load objects
	dog = LoadModelPlus("../obj/dog.obj");
	skybox = LoadModelPlus("../obj/skybox.obj");
	octagon = LoadModelPlus("../obj/octagon.obj"); //moving cube

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
	terrain_program = loadShaders("terrain.vert", "terrain.frag");
	
	//set frustrum
	frustum_matrix = frustum(-0.1, 0.1, -0.1, 0.1, 0.2, 1000.0);

	//skybox texture
	glUseProgram(skybox_program);
	glUniformMatrix4fv(glGetUniformLocation(skybox_program, "frustum"), 1, GL_TRUE, frustum_matrix.m);
	glUniform1i(glGetUniformLocation(skybox_program, "texUnit"), 0);
	
	//dog texture
	glUseProgram(program);
	glUniformMatrix4fv(glGetUniformLocation(program, "frustum"), 1, GL_TRUE, frustum_matrix.m);
	glUniform1i(glGetUniformLocation(program, "texUnit"), 0);
	
	//terrain texture
	glUseProgram(terrain_program);
	glUniformMatrix4fv(glGetUniformLocation(terrain_program, "frustum"), 1, GL_TRUE, frustum_matrix.m);
	glUniform1i(glGetUniformLocation(terrain_program, "texUnit"), 0);

}

void display(void)
{
	//for object instancing
	int count = 1 * edge; 

	//set up the camera
	direction = SetVector(cos(verticalAngle)*sin(horizontalAngle), sin(verticalAngle), cos(verticalAngle)*cos(horizontalAngle));
	right = SetVector(sin(horizontalAngle-3.14/2.0), 0, cos(horizontalAngle-3.14/2.0));
	up = CrossProduct(right, direction);
	
	//smooth movement of the camera
	position.y = smoothen(0.5, 1, position, cam_t) + 3;
	cam_t = position.y;
	
	camera_placement = lookAtv(position, VectorAdd(position,direction),up);
	
	// clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//get the time
	t = (GLfloat)glutGet(GLUT_ELAPSED_TIME);
	
	//draw skybox
	printError("skybox");
	glUseProgram(skybox_program);
	glDisable(GL_DEPTH_TEST);
	//glDisable(GL_CULL_FACE);
	
	camera_skybox = lookAtv(position, VectorAdd(position,direction),up);
	camera_skybox.m[3]=0;
	camera_skybox.m[7]=0;
	camera_skybox.m[11]=0;
	
	translation = T(0, 0, 0);
	
	glUniformMatrix4fv(glGetUniformLocation(skybox_program, "camera"), 1, GL_TRUE, camera_skybox.m);
	glUniformMatrix4fv(glGetUniformLocation(skybox_program, "translation"), 1, GL_TRUE, translation.m);
	
	glBindTexture(GL_TEXTURE_2D, skybox_tex);
	DrawModel(skybox, skybox_program, "in_Position", 0L, "inTexCoord");

	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);
	
	//draw terrain
	glUseProgram(terrain_program);
	translation = T(0,0,0);
	glUniformMatrix4fv(glGetUniformLocation(terrain_program, "translation"), 1, GL_TRUE, translation.m);
	glUniformMatrix4fv(glGetUniformLocation(terrain_program, "camera"), 1, GL_TRUE, camera_placement.m);
	glBindTexture(GL_TEXTURE_2D, grass_tex);	
	DrawModel(terrain, terrain_program, "in_Position", "in_Normal", "inTexCoord");
	
	
	//draw objects
	glUseProgram(program);
	glUniform1i(glGetUniformLocation(program, "edge"), edge);
	glUniform1i(glGetUniformLocation(program, "count"), count);
	glUniformMatrix4fv(glGetUniformLocation(program, "camera"), 1, GL_TRUE, camera_placement.m);
	
	//dog
	scaling = S(0.1,0.1,0.1);
	vec3 dog_position = {100, 0, 100};
	translation = T(dog_position.x, calculateHeight(terrain, dog_position), dog_position.z);
	rotation = Ry(45);
	glUniformMatrix4fv(glGetUniformLocation(program, "scaling"), 1, GL_TRUE, scaling.m);
	glUniformMatrix4fv(glGetUniformLocation(program, "translation"), 1, GL_TRUE, translation.m);
	glUniformMatrix4fv(glGetUniformLocation(program, "rotation"), 1, GL_TRUE, rotation.m);
	glBindTexture(GL_TEXTURE_2D, dog_tex);	
	//DrawModel(dog, program, "in_Position", 0L, "inTexCoord");
	
	//instance
	DrawModelInstanced(dog, program, "in_Position", 0L, "inTexCoord", count);
	
	glutSwapBuffers();
}

//CONTROLS
void OnTimer(int value)
{
	glutTimerFunc(20, &OnTimer, value);
	glutPostRedisplay();

	if(glutKeyIsDown('d')){
		position=VectorAdd(position, ScalarMult(right,0.5));
	}
	if(glutKeyIsDown('w')){
		position.x = VectorAdd(position, ScalarMult(direction,0.5)).x;
		position.z = VectorAdd(position, ScalarMult(direction,0.5)).z;
	}
	if(glutKeyIsDown('s')){
		position.x = VectorSub(position, ScalarMult(direction,0.5)).x;
		position.z = VectorSub(position, ScalarMult(direction,0.5)).z;
	}
	if(glutKeyIsDown('a')){
		position=VectorSub(position, ScalarMult(right,0.5));
	}
	if (glutKeyIsDown('q')){
			exit(0);
	}
}

void mouseMove(int x, int y) {
	
	// update deltaAngle
	horizontalAngle -= (x - 300) * 0.001f;
	verticalAngle -= (y - 300) * 0.001f;

	printf("%f\n", horizontalAngle );
	printf("%f\n", verticalAngle );

	if(x!=300 || y!=300){
		glutWarpPointer(300, 300 );
	}
}

int main(int argc, char *argv[]){
	
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
	glutInitContextVersion(3, 2);
	glutInitWindowSize (HEIGHT, WIDTH);
	glutDisplayFunc(display);
	glutCreateWindow ("Cool 3D World");
	glutPassiveMotionFunc(mouseMove);
	init();
	glutTimerFunc(20, &OnTimer, 0);
	glutMainLoop();
	return 0;
}
