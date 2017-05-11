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

#define COLLISION_DIST 1.0f



// Globals
vec3 position = {3.0,10.0,3.0};
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
GLuint grass_tex, skybox_tex, dog_tex, tree_tex, wood_tex, dirt_tex;
GLuint skyboxup_tex,skyboxdn_tex, skyboxlf_tex, skyboxrt_tex, skyboxbk_tex, skyboxft_tex;
TextureData terrain_tex;

GLuint program, skybox_program, terrain_program; // Reference to shader program
GLfloat t; //time
float obj_t=0, t_cam=0;

int texWidth=0; //for calculateHeight function

int edge = 5; //for object instancing


struct wall_hitbox {
	float vertices [6];
	float height;
};

int n_walls;


struct wall_hitbox walls [1000] = {};
float test [5] = {0,1};


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
	LoadTGATextureSimple("../tex/niceday2up.tga", &skyboxup_tex);
	LoadTGATextureSimple("../tex/niceday2dn.tga", &skyboxdn_tex);
	LoadTGATextureSimple("../tex/niceday2lf.tga", &skyboxlf_tex);
	LoadTGATextureSimple("../tex/niceday2rt.tga", &skyboxrt_tex);
	LoadTGATextureSimple("../tex/niceday2ft.tga", &skyboxft_tex);
	LoadTGATextureSimple("../tex/niceday2bk.tga", &skyboxbk_tex);

	LoadTGATextureSimple("../tex/dog.tga", &dog_tex);
	LoadTGATextureSimple("../tex/greenleaves.tga", &tree_tex);
	LoadTGATextureSimple("../tex/woodplanks.tga", &wood_tex);
	LoadTGATextureSimple("../tex/dirt.tga", &dirt_tex);

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

	glUseProgram(skybox_program);
	glUniformMatrix4fv(glGetUniformLocation(skybox_program, "frustum"), 1, GL_TRUE, frustum_matrix.m);

	glUseProgram(terrain_program);
	glUniformMatrix4fv(glGetUniformLocation(terrain_program, "frustum"), 1, GL_TRUE, frustum_matrix.m);

	glUseProgram(program);
	glUniformMatrix4fv(glGetUniformLocation(program, "frustum"), 1, GL_TRUE, frustum_matrix.m);

}

//DRAW objects as specified

void draw(int edge_val, int distance_offset, int origin_offset, vec3 pos, int rotangle, Model *obj, GLuint program){

	edge = edge_val;
	int count = 1 * edge;
	int i;
	translation = T(pos.x, calculateHeight(terrain, pos) + origin_offset, pos.z);
	rotation = Ry(rotangle);

	glUniform1i(glGetUniformLocation(program, "edge"), edge);
	glUniform1i(glGetUniformLocation(program, "count"), count);
	glUniform1i(glGetUniformLocation(program, "distance"), distance_offset);
	glUniformMatrix4fv(glGetUniformLocation(program, "scaling"), 1, GL_TRUE, scaling.m);
	glUniformMatrix4fv(glGetUniformLocation(program, "translation"), 1, GL_TRUE, translation.m);
	glUniformMatrix4fv(glGetUniformLocation(program, "rotation"), 1, GL_TRUE, rotation.m);

	//instancing
	DrawModelInstanced(obj, program, "in_Position", 0L, "inTexCoord", count);
}

void display(void)
{
	vec3 pos = {0,0,0};

	//set up the camera
	right = SetVector(sin(horizontalAngle-3.14/2.0), 0, cos(horizontalAngle-3.14/2.0));
	up = CrossProduct(right, direction);

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

	//skybox texture

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, skybox_tex);
	glUniform1i(glGetUniformLocation(skybox_program, "texUnit1"), 1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, dirt_tex);
	glUniform1i(glGetUniformLocation(skybox_program, "texUnit2"), 0);

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
	glUniform1i(glGetUniformLocation(terrain_program, "texUnit"), 0);
	DrawModel(terrain, terrain_program, "in_Position", "in_Normal", "inTexCoord");


	//draw objects
	glUseProgram(program);
	glUniformMatrix4fv(glGetUniformLocation(program, "camera"), 1, GL_TRUE, camera_placement.m);

	//dog
	scaling = S(0.05,0.05,0.05);
	pos.x = 100;
	pos.y = 0;
	pos.z = 200;
	glBindTexture(GL_TEXTURE_2D, dog_tex);
	glUniform1i(glGetUniformLocation(program, "texUnit"), 0);
	draw(5,6,0,pos,60,dog,program);

	//trees
	scaling = S(0.05,0.05,0.05);
	pos.x = 200;
	pos.y = 0;
	pos.z = 200;
	glBindTexture(GL_TEXTURE_2D, tree_tex);
	glUniform1i(glGetUniformLocation(program, "texUnit"), 0);
	draw(3,55,0,pos,0,tree,program);
	pos.z = 150;
	draw(4,35,0,pos,45,tree,program);
	pos.z = 80;
	scaling = S(0.03,0.03,0.03);
	draw(2,75,0,pos,90,tree,program);

	//bunny
	scaling = S(2,2,2);
	pos.x = 100;
	pos.y = 0;
	pos.z = 100;
	glBindTexture(GL_TEXTURE_2D, dirt_tex);
	glUniform1i(glGetUniformLocation(program, "texUnit"), 0);
	draw(1,6,1,pos,t/100,bunny,program);

	glutSwapBuffers();
}


vec3 check_collision_objects(float dist)
{
	int i;
	for (i = 0; i < n_walls; i++)
	{
		// x = az + b
		float y = walls[i].vertices[1];
		float x1 = walls[i].vertices[0];
		float x2 = walls[i].vertices[3];
		float z1 = walls[i].vertices[2];
		float z2 = walls[i].vertices[5];

		if (z2 != z1) {

			float a = (x2 - x1)/(z2 - z1); // equation
			float b = x1 - a*z1;

			float point = a * position.z + b;
			if ((point > position.x && point < position.x + dist) || (point < position.x && point > position.x - dist)) // If player is within a "dist" distance from the line
			{
				if (position.y > y && position.y < y+walls[i].height) {
					printf("COLLISION\n");
					vec3 out = {x2-x1,y,z2-z1};
					return Normalize(out);
				}
			}
		}
		else // Wall is on x axis
		{
			if ((position.z > z1 && position.z < z1 + dist) || (position.z < z1 && position.z > z1 - dist))
			{
				printf("COLLISION\n");
				vec3 out = {1,y,0};
				return Normalize(out);
			}
		}

	}
	printf("\n");
	vec3 out = {};
	return out;
}


//CONTROLS
void OnTimer(int value)
{
	glutTimerFunc(20, &OnTimer, value);
	glutPostRedisplay();

	check_collision_objects(COLLISION_DIST);

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

	if (glutKeyIsDown('q'))
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

void create_wall(float a, float b, float c, float d, float e, float f, float height)
{
	struct wall_hitbox wall1;
	wall1.vertices[0] = a;
	wall1.vertices[1] = b;
	wall1.vertices[2] = c;
	wall1.vertices[3] = d;
	wall1.vertices[4] = e;
	wall1.vertices[5] = f;
	wall1.height = height;
	walls[n_walls] = wall1;
	n_walls++;
}

int main(int argc, char *argv[]){
	time_air = 0;
	collision_ground = false;
	collision_object = false;
	jumping = false;
	n_walls = 0;

	// 4 World limits
	create_wall(0, -20, 0, 0, -20, 500, 100);
	create_wall(0, -20, 0, 500, -20, 0, 100);
	create_wall(255, -20, 0, 255, -20, 500, 100);
	create_wall(0, -20, 255, 500, -20, 255, 100);


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
