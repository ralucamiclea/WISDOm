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

#define HEIGHT 1000
#define WIDTH 1000



// Globals
vec3 position = {0.0,50.0,0.0};
vec3 direction = {0.0,0.0,0.0};
vec3 right = {0.0,0.0,0.0};
vec3 up = {0.0,0.0,0.0};

static float speed = 0.2f;
static float player_height = 0.8f;
bool collision_ground, collision_object, jumping;
int time_air;

float angle; //angle of rotation for the camera direction
float horizontalAngle = 0.0;
float verticalAngle = 0.0;

vec3 posObj; //position of the moving cube
mat4 frustum_matrix, camera_placement, rotation, translation, scaling, camera_skybox;
Model *dog, *terrain, *skybox, *octagon, *tree, *bunny;
GLuint grass_tex, skybox_tex, dog_tex, tree_tex, wood_tex;
GLuint skyboxup_tex,skyboxdn_tex, skyboxlf_tex, skyboxrt_tex, skyboxbk_tex, skyboxft_tex;
TextureData terrain_tex;

GLuint program, skybox_program, terrain_program; // Reference to shader program
GLfloat t; //time
float obj_t=0, t_cam=0;

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
			vertexArray[(x + z * tex->width)*3 + 1] = tex->imageData[(x + z * tex->width) * (tex->bpp/8)] / 20.0;
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
	LoadTGATextureData("../tex/fft-terrain.tga", &terrain_tex);
	terrain = GenerateTerrain(&terrain_tex);
	LoadTGATextureSimple("../tex/grass.tga", &grass_tex);
	LoadTGATextureSimple("../tex/skybox.tga", &skybox_tex);
	/*
	LoadTGATextureSimple("../tex/niceday2up.tga", &skyboxup_tex);
	LoadTGATextureSimple("../tex/niceday2dn.tga", &skyboxdn_tex);
	LoadTGATextureSimple("../tex/niceday2lf.tga", &skyboxlf_tex);
	LoadTGATextureSimple("../tex/niceday2rt.tga", &skyboxrt_tex);
	LoadTGATextureSimple("../tex/niceday2ft.tga", &skyboxft_tex);
	LoadTGATextureSimple("../tex/niceday2bk.tga", &skyboxbk_tex);
	*/
	LoadTGATextureSimple("../tex/dog.tga", &dog_tex);
	LoadTGATextureSimple("../tex/greenleaves.tga", &tree_tex);
	LoadTGATextureSimple("../tex/woodplanks.tga", &wood_tex);

	//load objects
	dog = LoadModelPlus("../obj/dog.obj");
	//skybox = LoadModelPlus("../obj/skybox1.obj");
	skybox = LoadModelPlus("../obj/skybox.obj");
	octagon = LoadModelPlus("../obj/octagon.obj"); //moving cube
	tree = LoadModelPlus("../obj/nicetree.obj");
	bunny = LoadModelPlus("../obj/bunny.obj");

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
	/*
	glUniform1i(glGetUniformLocation(skybox_program, "texUnit1"), 0);
	glUniform1i(glGetUniformLocation(skybox_program, "texUnit2"), 0);
	glUniform1i(glGetUniformLocation(skybox_program, "texUnit3"), 0);
	glUniform1i(glGetUniformLocation(skybox_program, "texUnit4"), 0);
	glUniform1i(glGetUniformLocation(skybox_program, "texUnit5"), 0);
	glUniform1i(glGetUniformLocation(skybox_program, "texUnit6"), 0);
	*/

	//terrain texture
	glUseProgram(terrain_program);
	glUniformMatrix4fv(glGetUniformLocation(terrain_program, "frustum"), 1, GL_TRUE, frustum_matrix.m);
	glUniform1i(glGetUniformLocation(terrain_program, "texUnit"), 0);

	//dog texture
	glUseProgram(program);
	glUniformMatrix4fv(glGetUniformLocation(program, "frustum"), 1, GL_TRUE, frustum_matrix.m);
	glUniform1i(glGetUniformLocation(program, "texUnit"), 0);
}

//DRAW objects as specified
void draw(int edge_val, int offset, vec3 pos, int rotangle, GLuint texture, Model *obj, GLuint program){

	edge = edge_val;
	int count = 1 * edge;
	translation = T(pos.x, calculateHeight(terrain, pos), pos.z);
	rotation = Ry(rotangle);

	glUniform1i(glGetUniformLocation(program, "edge"), edge);
	glUniform1i(glGetUniformLocation(program, "count"), count);
	glUniform1i(glGetUniformLocation(program, "distance"), offset);
	glUniformMatrix4fv(glGetUniformLocation(program, "scaling"), 1, GL_TRUE, scaling.m);
	glUniformMatrix4fv(glGetUniformLocation(program, "translation"), 1, GL_TRUE, translation.m);
	glUniformMatrix4fv(glGetUniformLocation(program, "rotation"), 1, GL_TRUE, rotation.m);

	glBindTexture(GL_TEXTURE_2D, texture);

	//instancing
	DrawModelInstanced(obj, program, "in_Position", 0L, "inTexCoord", count);
}

void display(void)
{
	vec3 pos = {0,0,0};

	//set up the camera
	right = SetVector(sin(horizontalAngle-3.14/2.0), 0, cos(horizontalAngle-3.14/2.0));
	up = CrossProduct(right, direction);

	//smooth movement of the camera
	position.y = smoothen(0.5, 1, position, t_cam) + 3;
	t_cam = position.y;

	camera_placement = lookAt(position.x, position.y, position.z, position.x + direction.x, position.y + direction.y, position.z + direction.z, 0, 1, 0);

	// clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//get the time
	t = (GLfloat)glutGet(GLUT_ELAPSED_TIME);

	//draw skybox
	printError("skybox");
	glUseProgram(skybox_program);
	glDisable(GL_DEPTH_TEST);
	//glDisable(GL_CULL_FACE);

	camera_skybox = lookAt(position.x, position.y, position.z, position.x + direction.x, position.y + direction.y, position.z + direction.z, 0, 1, 0);
	camera_skybox.m[3]=0;
	camera_skybox.m[7]=0;
	camera_skybox.m[11]=0;

	translation = T(0, 0, 0);

	glUniformMatrix4fv(glGetUniformLocation(skybox_program, "camera"), 1, GL_TRUE, camera_skybox.m);
	glUniformMatrix4fv(glGetUniformLocation(skybox_program, "translation"), 1, GL_TRUE, translation.m);
	glBindTexture(GL_TEXTURE_2D, skybox_tex);
	/*
	glBindTexture(GL_TEXTURE_2D, skyboxft_tex);
	glBindTexture(GL_TEXTURE_2D, skyboxbk_tex);
	glBindTexture(GL_TEXTURE_2D, skyboxup_tex);
	glBindTexture(GL_TEXTURE_2D, skyboxdn_tex);
	glBindTexture(GL_TEXTURE_2D, skyboxlf_tex);
	glBindTexture(GL_TEXTURE_2D, skyboxrt_tex);
	*/
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
	glUniformMatrix4fv(glGetUniformLocation(program, "camera"), 1, GL_TRUE, camera_placement.m);

	//dog
	scaling = S(0.05,0.05,0.05);
	pos.x = 100;
	pos.y = 0;
	pos.z = 200;
	draw(5,6,pos,60,dog_tex,dog,program);

	//trees
	scaling = S(0.05,0.05,0.05);
	pos.x = 200;
	pos.y = 0;
	pos.z = 200;
	draw(3,55,pos,0,tree_tex,tree,program);
	pos.z = 150;
	draw(4,35,pos,45,tree_tex,tree,program);
	pos.z = 80;
	scaling = S(0.03,0.03,0.03);
	draw(2,75,pos,90,tree_tex,tree,program);

	//bunny
	scaling = S(2,2,2);
	pos.x = 100;
	pos.y = 0;
	pos.z = 100;
	draw(1,6,pos,t/1000,wood_tex,bunny,program);

	glutSwapBuffers();
}

//CONTROLS
void OnTimer(int value)
{
	glutTimerFunc(20, &OnTimer, value);
	glutPostRedisplay();

	float ground_height = smoothen(speed, 1, position, t_cam) + player_height;
	t_cam = position.y;
	if (position.y < ground_height) {
		collision_ground = true;
		time_air = 0;
		jumping = false;
	}


	if (glutKeyIsDown('e') && glutKeyIsDown('w')){ // Sprint (Shift)
		position.x += direction.x * speed * 2;
		position.z += direction.z * speed * 2;
	}

	if (glutKeyIsDown('w')){ //move camera forward
		position.x += direction.x * speed;
		position.z += direction.z * speed;
	}

	if (glutKeyIsDown('s')){ //move camera backwards
		position.x -= direction.x * speed*0.5;
		position.z -= direction.z * speed*0.5;
	}

	if (glutKeyIsDown('a')){ //rotate camera left around Y axis
		position.x += direction.z * speed*0.7;
		position.z -= direction.x * speed*0.7;
	}

	if (glutKeyIsDown('d')){ //rotate camera right around Y axis
		position.x -= direction.z * speed*0.7;
		position.z += direction.x * speed*0.7;
	}

	if (glutKeyIsDown('c')){ // Crouching

	}


	if (glutKeyIsDown(32)){ // jump (Space)
		if (collision_ground) // Jump only if touching ground
		{
				jumping = true;
				collision_ground = false;
				time_air = 0;
		}
	}
	else
	{

	}

	if (glutKeyIsDown('p'))
	{
		exit(0);
	}


	// Jump

	if (!collision_ground) {
		time_air++;
		if (jumping){
				position.y -= (float)(time_air - 20.0)/100.0;
		}
		else {// falling
				position.y -= (float)(time_air)/100.0;
		}
	}
	else {
		position.y = smoothen(speed, 1, position, t_cam) + player_height;
		t_cam = position.y;
	}

}

void mouseMove(int x, int y) {

	// update deltaAngle
	horizontalAngle += (x - WIDTH/2) * 0.001f;
	verticalAngle += (y - HEIGHT/2) * 0.001f;

	direction.x = cos(verticalAngle)*sin(horizontalAngle);
	direction.z = -cos(verticalAngle)*cos(horizontalAngle);

	verticalAngle = verticalAngle > 4.67 ? 4.67 : verticalAngle;
	verticalAngle = verticalAngle < 1.6 ? 1.6 : verticalAngle;

	direction.y = sin(verticalAngle);

	if(x!=WIDTH/2 || y!=HEIGHT/2){
		glutWarpPointer(WIDTH/2, HEIGHT/2 );
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
