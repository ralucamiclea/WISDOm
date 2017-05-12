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

#define SPRINT 8.0f
#define BACKWARDS 0.5f
#define SIDEWARDS 0.7f

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

mat4 frustum_matrix, camera_placement, rotation, translation, scaling, camera_skybox;
Model *dog, *terrain, *octagon, *tree, *bunny;
GLuint grass_tex, dog_tex, tree_tex, wood_tex, dirt_tex;
Model *tree2, *ring, *house, *deer, *bear, *boar, *wolf, *bird;
GLuint deer_tex, bear_tex, boar_tex, wolf_tex;
TextureData terrain_tex;
GLuint program, skybox_program, terrain_program, prize_program; // Reference to shader program

//LIGHT sources
Point3D lightSourcesColorsArr[] = { {1.0f, 0.0f, 0.0f}, // Red light
                                 {0.0f, 1.0f, 0.0f}, // Green light
                                 {0.0f, 0.0f, 1.0f}, // Blue light
                                 {1.0f, 1.0f, 1.0f} }; // White light

GLfloat specularExponent[] = {10.0, 20.0, 60.0, 5.0};
GLint isDirectional[] = {0,0,1,1};

Point3D lightSourcesDirectionsPositions[] = { {10.0f, 5.0f, 0.0f}, // Red light, positional
                                       {0.0f, 5.0f, 10.0f}, // Green light, positional
                                       {-1.0f, 0.0f, 0.0f}, // Blue light along X
                                       {0.0f, 0.0f, -1.0f} }; // White light along Z

//SKYBOX
Model *box[6];
GLfloat vertices[6][6*3] =
{
	{ // +x
		0.5,-0.5,-0.5,		// 1
		0.5,0.5,-0.5,		// 2
		0.5,0.5,0.5,		// 6
		0.5,-0.5,0.5,		// 5
	},
	{ // -x
		-0.5,-0.5,-0.5,		// 0 -0
		-0.5,-0.5,0.5,		// 4 -1
		-0.5,0.5,0.5,		// 7 -2
		-0.5,0.5,-0.5,		// 3 -3
	},
	{ // +y
		0.5,0.5,-0.5,		// 2 -0
		-0.5,0.5,-0.5,		// 3 -1
		-0.5,0.5,0.5,		// 7 -2
		0.5,0.5,0.5,		// 6 -3
	},
	{ // -y
		-0.5,-0.5,-0.5,		// 0
		0.5,-0.5,-0.5,		// 1
		0.5,-0.5,0.5,		// 5
		-0.5,-0.5,0.5		// 4
	},
	{ // +z
		-0.5,-0.5,0.5,		// 4
		0.5,-0.5,0.5,		// 5
		0.5,0.5,0.5,		// 6
		-0.5,0.5,0.5,		// 7
	},
	{ // -z
		-0.5,-0.5,-0.5,	// 0
		-0.5,0.5,-0.5,		// 3
		0.5,0.5,-0.5,		// 2
		0.5,-0.5,-0.5,		// 1
	}
};

GLfloat texcoord[6][6*2] =
{
	{
		1.0, 1.0,
		1.0, 0.0, // left OK
		0.0, 0.0,
		0.0, 1.0,
	},
	{
		0.0, 1.0, // right OK
		1.0, 1.0,
		1.0, 0.0,
		0.0, 0.0,
	},
	{
		1.0, 0.0, // top OK
		0.0, 0.0,
		0.0, 1.0,
		1.0, 1.0,
	},
	{
		0.0, 1.0,
		1.0, 1.0,
		1.0, 0.0, // bottom
		0.0, 0.0,
	},
	{
		0.0, 1.0,
		1.0, 1.0,
		1.0, 0.0, // back OK
		0.0, 0.0,
	},
	{
		1.0, 1.0,
		1.0, 0.0, // front OK
		0.0, 0.0,
		0.0, 1.0,
	}
};
GLuint indices[6][6] =
{
	{0, 2, 1, 0, 3, 2},
	{0, 2, 1, 0, 3, 2},
	{0, 2, 1, 0, 3, 2},
	{0, 2, 1, 0, 3, 2},
	{0, 2, 1, 0, 3, 2},
	{0, 2, 1, 0, 3, 2}
};

GLuint cubemap;

// Select texture set by setting this constant to 0 or 6
#define TEXTURE_OFFSET 0

char *textureFileName[12] =
{
// 0-5:
  "../tex/petomavar-skybox/skyrender0004.tga",
  "../tex/petomavar-skybox/skyrender0001.tga",
  "../tex/petomavar-skybox/skyrender0003.tga",
  "../tex/petomavar-skybox/skyrender0006.tga",
  "../tex/petomavar-skybox/skyrender0002.tga",
  "../tex/petomavar-skybox/skyrender0005.tga",
// 6-11
  "../tex/skybox/left.tga",
  "../tex/skybox/right.tga",
  "../tex/skybox/top.tga",
  "../tex/skybox/bottom.tga",
  "../tex/skybox/back.tga",
  "../tex/skybox/front.tga",
};

TextureData tx[6];
// END SKYBOX stuff

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

//LOAD texture
void loadTextures()
{
	int i;

	glGenTextures(1, &cubemap);	// Generate OpenGL texture IDs
	glActiveTexture(GL_TEXTURE0); // Just make sure the texture unit match

	// Note all operations on GL_TEXTURE_CUBE_MAP, not GL_TEXTURE_2D

	// Load texture data and create ordinary texture objects (for skybox)
	for (i = 0; i < 6; i++)
	{
		printf("Loading texture %s\n", textureFileName[i+TEXTURE_OFFSET]);
		LoadTGATexture(textureFileName[i+TEXTURE_OFFSET], &tx[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	// Load to cube map
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, tx[0].w, tx[0].h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tx[0].imageData);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, tx[1].w, tx[1].h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tx[1].imageData);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, tx[2].w, tx[2].h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tx[2].imageData);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, tx[3].w, tx[3].h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tx[3].imageData);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, tx[4].w, tx[4].h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tx[4].imageData);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, tx[5].w, tx[5].h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tx[5].imageData);

	//	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// MIPMAPPING
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
}


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
			0L,
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
	if (m != 0L)
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

		if (normalVariableName!=0L)
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
		if ((m->texCoordArray != 0L)&&(texCoordVariableName != 0L))
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
	int i;

	//load textures
	LoadTGATextureData("../tex/fft-terrain.tga", &terrain_tex);
	terrain = GenerateTerrain(&terrain_tex);
	LoadTGATextureSimple("../tex/grass.tga", &grass_tex);
	LoadTGATextureSimple("../tex/dog.tga", &dog_tex);
	LoadTGATextureSimple("../tex/greenleaves.tga", &tree_tex);
	LoadTGATextureSimple("../tex/woodplanks.tga", &wood_tex);
	LoadTGATextureSimple("../tex/dirt.tga", &dirt_tex);
	LoadTGATextureSimple("../tex/deer.tga", &deer_tex);
	LoadTGATextureSimple("../tex/bear.tga", &bear_tex);
	LoadTGATextureSimple("../tex/boar.tga", &boar_tex);
	LoadTGATextureSimple("../tex/wolf.tga", &wolf_tex);

	//load skybox
	for (i = 0; i < 6; i++)
	{
		box[i] = LoadDataToModel(
			vertices[i],
			0L,
			texcoord[i],
			0L,
			indices[i],
			4,
			6);
	}
	//load other objects
	dog = LoadModelPlus("../obj/dog.obj");
	octagon = LoadModelPlus("../obj/octagon.obj");
	tree = LoadModelPlus("../obj/nicetree.obj");
	bunny = LoadModelPlus("../obj/bunny.obj");
	tree2 = LoadModelPlus("../obj/tree.obj");
	ring = LoadModelPlus("../obj/tree.obj");
	house = LoadModelPlus("../obj/tree.obj");
	deer = LoadModelPlus("../obj/deer-obj.obj");
	bear = LoadModelPlus("../obj/bear-obj.obj");
	boar = LoadModelPlus("../obj/boar-obj.obj");
	wolf = LoadModelPlus("../obj/wolf-obj.obj");
	bird = LoadModelPlus("../obj/bird.obj");
	ring = LoadModelPlus("../obj/ring.obj");

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
	prize_program = loadShaders("prize.vert", "prize.frag");

	//set frustrum
	frustum_matrix = frustum(-0.1, 0.1, -0.1, 0.1, 0.2, 300.0);

	glUseProgram(skybox_program);
	glUniform1i(glGetUniformLocation(skybox_program, "texUnit"), 0);
	glUniformMatrix4fv(glGetUniformLocation(skybox_program, "frustum"), 1, GL_TRUE, frustum_matrix.m);

	glUseProgram(terrain_program);
	glUniformMatrix4fv(glGetUniformLocation(terrain_program, "frustum"), 1, GL_TRUE, frustum_matrix.m);
	
	glUseProgram(prize_program);
	glUniformMatrix4fv(glGetUniformLocation(prize_program, "frustum"), 1, GL_TRUE, frustum_matrix.m);

	glUseProgram(program);
	glUniformMatrix4fv(glGetUniformLocation(program, "frustum"), 1, GL_TRUE, frustum_matrix.m);

	glEnable(GL_DEPTH_TEST);
}

//DRAW objects as specified

void draw(int edge_val, int distance_offset, int origin_offset, vec3 pos, int rotangle, int rotax, Model *obj, GLuint program, int normal){

	edge = edge_val;
	int count = 1 * edge;
	int i;
	translation = T(pos.x, calculateHeight(terrain, pos) + origin_offset, pos.z);
	
	if(rotax == 0)
		rotation = Rx(rotangle);
	if(rotax == 1)
		rotation = Ry(rotangle);
	if(rotax == 2)
		rotation = Rz(rotangle);

	glUniform1i(glGetUniformLocation(program, "edge"), edge);
	glUniform1i(glGetUniformLocation(program, "count"), count);
	glUniform1i(glGetUniformLocation(program, "distance"), distance_offset);
	glUniformMatrix4fv(glGetUniformLocation(program, "camera"), 1, GL_TRUE, camera_placement.m);
	glUniformMatrix4fv(glGetUniformLocation(program, "scaling"), 1, GL_TRUE, scaling.m);
	glUniformMatrix4fv(glGetUniformLocation(program, "translation"), 1, GL_TRUE, translation.m);
	glUniformMatrix4fv(glGetUniformLocation(program, "rotation"), 1, GL_TRUE, rotation.m);

	//instancing
	if(normal != 0){
		DrawModelInstanced(obj, program, "in_Position", "in_Normal", "inTexCoord", count);
	}
	else {
		DrawModelInstanced(obj, program, "in_Position", 0L, "inTexCoord", count);
	}		
}

void display(void)
{
	vec3 pos = {0,0,0};
	int i;

	// clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//get the time
	t = (GLfloat)glutGet(GLUT_ELAPSED_TIME);

	//set up the camera
	right = SetVector(sin(horizontalAngle-3.14/2.0), 0, cos(horizontalAngle-3.14/2.0));
	up = CrossProduct(right, direction);

	camera_placement = lookAt(position.x, position.y, position.z, position.x + direction.x, position.y + direction.y, position.z + direction.z, 0, 1, 0);

	//draw skybox
	printError("skybox");
	glUseProgram(skybox_program);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	camera_skybox = lookAt(position.x, position.y, position.z, position.x + direction.x, position.y + direction.y, position.z + direction.z, 0, 1, 0);
	camera_skybox.m[3]=0;
	camera_skybox.m[7]=0;
	camera_skybox.m[11]=0;

	translation = T(0, 0, 0);

	glUniformMatrix4fv(glGetUniformLocation(skybox_program, "camera"), 1, GL_TRUE, camera_skybox.m);
	glUniformMatrix4fv(glGetUniformLocation(skybox_program, "translation"), 1, GL_TRUE, translation.m);

	//skybox texture
	printError("skybox texture");
	for (i = 0; i < 6; i++)
	{
		glBindTexture(GL_TEXTURE_2D, tx[i].texID);
		DrawModel(box[i], skybox_program, "in_Position", 0L, "inTexCoord");
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// Binding again just to be sure the texture unit match
	glActiveTexture(GL_TEXTURE0);

	//draw terrain
	printError("terrain");
	glUseProgram(terrain_program);
	translation = T(0,0,0);
	glUniformMatrix4fv(glGetUniformLocation(terrain_program, "translation"), 1, GL_TRUE, translation.m);
	glUniformMatrix4fv(glGetUniformLocation(terrain_program, "camera"), 1, GL_TRUE, camera_placement.m);
	glBindTexture(GL_TEXTURE_2D, grass_tex);
	glUniform1i(glGetUniformLocation(terrain_program, "texUnit"), 0);
	DrawModel(terrain, terrain_program, "in_Position", "in_Normal", "inTexCoord");
	
	
	//draw reward
	printError("reward");
	glUseProgram(prize_program);
	glUniform3f(glGetUniformLocation(prize_program, "xyz"), position.x, position.y, position.z); //camera position
	glUniform3fv(glGetUniformLocation(prize_program, "lightSourcesDirPosArr"), 4, &lightSourcesDirectionsPositions[0].x);
	glUniform3fv(glGetUniformLocation(prize_program, "lightSourcesColorArr"), 4, &lightSourcesColorsArr[0].x);
	glUniform1fv(glGetUniformLocation(prize_program, "specularExponent"), 4, specularExponent);
	glUniform1iv(glGetUniformLocation(prize_program, "isDirectional"), 4, isDirectional);
	scaling = S(2,2,2);
	pos.x = 100;
	pos.y = 0;
	pos.z = 65;
	draw(1,6,10,pos,t/100,0,ring,prize_program,1);
	pos.x = 70;
	pos.y = 0;
	pos.z = 100;
	draw(1,6,10,pos,t/100,2,ring,prize_program,1);
	pos.x = 185;
	pos.y = 0;
	pos.z = 185;
	draw(1,6,10,pos,t/100,0,ring,prize_program,1);
	pos.x = 85;
	pos.y = 0;
	pos.z = 200;
	draw(1,6,10,pos,t/100,2,ring,prize_program,1);
	pos.x = 10;
	pos.y = 0;
	pos.z = 215;
	draw(1,6,10,pos,t/100,2,ring,prize_program,1);


	//draw objects
	glUseProgram(program);

	//dog
	glBindTexture(GL_TEXTURE_2D, dog_tex);
	glUniform1i(glGetUniformLocation(program, "texUnit"), 0);
	scaling = S(0.05,0.05,0.05);
	pos.x = 80;
	pos.y = 0;
	pos.z = 250;
	draw(2,6,0,pos,60,1,dog,program,0);

	//trees
	glBindTexture(GL_TEXTURE_2D, tree_tex);
	glUniform1i(glGetUniformLocation(program, "texUnit"), 0);
	scaling = S(0.05,0.05,0.05);
	pos.x = 200;
	pos.y = 0;
	pos.z = 60;
	draw(3,55,-1,pos,0,1,tree,program,0);
	pos.x = 190;
	pos.z = 65;
	draw(2,35,-1,pos,45,1,tree,program,0);
	pos.z = 70;
	scaling = S(0.03,0.03,0.03);
	draw(1,75,-1,pos,90,1,tree,program,0);

	//bunny
	glBindTexture(GL_TEXTURE_2D, dirt_tex);
	glUniform1i(glGetUniformLocation(program, "texUnit"), 0);
	scaling = S(2,2,2);
	pos.x = 180;
	pos.y = 0;
	pos.z = 68;
	draw(1,6,1,pos,t/200,1,bunny,program,0);

	//deer
	glBindTexture(GL_TEXTURE_2D, deer_tex);
	glUniform1i(glGetUniformLocation(program, "texUnit"), 0);
	scaling = S(0.2,0.2,0.2);
	pos.x = 3;
	pos.y = 0;
	pos.z = 10;
	draw(1,6,1,pos,45,1,deer,program,0);

	//bear
	glBindTexture(GL_TEXTURE_2D, bear_tex);
	glUniform1i(glGetUniformLocation(program, "texUnit"), 0);
	scaling = S(0.2,0.2,0.2);
	pos.x = 15;
	pos.y = 0;
	pos.z = 10;
	draw(1,6,1,pos,45,1,bear,program,0);

	//boar
	glBindTexture(GL_TEXTURE_2D, boar_tex);
	glUniform1i(glGetUniformLocation(program, "texUnit"), 0);
	scaling = S(0.2,0.2,0.2);
	pos.x = 10;
	pos.y = 0;
	pos.z = 4;
	draw(1,6,1,pos,45,1,boar,program,0);

	//wolf
	glBindTexture(GL_TEXTURE_2D, wolf_tex);
	glUniform1i(glGetUniformLocation(program, "texUnit"), 0);
	scaling = S(0.2,0.2,0.2);
	pos.x = 10;
	pos.y = 0;
	pos.z = 10;
	draw(1,6,1,pos,45,1,wolf,program,0);

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
					vec3 out = {x2-x1,0,z2-z1};
					collision_object = true;
					return Normalize(out);
				}
			}
		}
		else // Wall is on x axis
		{
			if ((position.z > z1 && position.z < z1 + dist) || (position.z < z1 && position.z > z1 - dist))
			{
				vec3 out = {x2-x1,0,0}; // Necessary for the orientation of the wall
				out = Normalize(out);
				collision_object = true;
				return Normalize(out);
			}
		}

	}
	vec3 out = Normalize(direction);
	collision_object = false;
	return out;
}

float dot(vec3 v1, vec3 v2)
{
	return v1.x * v2.x + v1.z * v2.z;
}


//CONTROLS
void OnTimer(int value)
{

	vec3 rotated_direction;

	glutTimerFunc(20, &OnTimer, value);
	glutPostRedisplay();

	vec3 collision_vector;
	collision_vector = check_collision_objects(COLLISION_DIST);

	float collision_factor, sideways_factor;
	if (collision_object)
	{
		rotated_direction.x = -direction.z;
		rotated_direction.y = 0;
		rotated_direction.z = direction.x;
		rotated_direction = Normalize(rotated_direction);
		collision_factor = dot(collision_vector, direction);
		sideways_factor = dot(collision_vector,rotated_direction);
	}
	else
	{
		collision_factor = 1;
		sideways_factor = 1;
	}


	float ground_height = smoothen(speed, 1, position, t_cam) + player_height;
	t_cam = position.y;
	if (position.y < ground_height) {
		collision_ground = true;
		time_air = 0;
		jumping = false;
	}


	if (glutKeyIsDown('e') && glutKeyIsDown('w')){ // Sprint (Shift)
		if (dot(rotated_direction,collision_vector) > 0)
		{
			collision_factor = 1;
			sideways_factor = 1;
			collision_object = false;
		}
		if (collision_object)
		{
			position.x += direction.x * speed * SPRINT * sqrt(pow(collision_factor,2)) * sqrt(pow(collision_vector.x,2));
			position.z += direction.z * speed * SPRINT * sqrt(pow(collision_factor,2)) * sqrt(pow(collision_vector.z,2));
		}
		else
		{
			position.x += direction.x * speed * SPRINT * collision_factor;
			position.z += direction.z * speed * SPRINT * collision_factor;
		}
	}

	if (glutKeyIsDown('w')){ //move camera forward
		if (dot(rotated_direction,collision_vector) > 0)
		{
			collision_factor = 1;
			sideways_factor = 1;
			collision_object = false;
		}
		if (collision_object)
		{
			position.x += direction.x * speed * sqrt(pow(collision_factor,2)) * sqrt(pow(collision_vector.x,2));
			position.z += direction.z * speed * sqrt(pow(collision_factor,2)) * sqrt(pow(collision_vector.z,2));
		}
		else
		{
			position.x += direction.x * speed * collision_factor;
			position.z += direction.z * speed * collision_factor;
		}
	}

	if (glutKeyIsDown('s')){ //move camera backwards
		if (dot(rotated_direction,collision_vector) < 0)
		{
			collision_factor = 1;
			sideways_factor = 1;
			collision_object = false;
		}
		if (collision_object)
		{
			position.x -= direction.x * speed * BACKWARDS * sqrt(pow(collision_factor,2)) * sqrt(pow(collision_vector.x,2));
			position.z -= direction.z * speed * BACKWARDS * sqrt(pow(collision_factor,2)) * sqrt(pow(collision_vector.z,2));
		}
		else
		{
			position.x -= direction.x * speed * collision_factor;
			position.z -= direction.z * speed * collision_factor;
		}
	}

	if (glutKeyIsDown('a')){ // Move left
		if (dot(direction,collision_vector) > 0)
		{
			collision_factor = 1;
			sideways_factor = 1;
			collision_object = false;
		}
		if (collision_object)
		{
			position.x += direction.z * speed * SIDEWARDS * sqrt(pow(sideways_factor,2)) * sqrt(pow(collision_vector.x,2));
			position.z -= direction.x * speed * SIDEWARDS * sqrt(pow(sideways_factor,2)) * sqrt(pow(collision_vector.z,2));
		}
		else
		{
			position.x += direction.z * speed * SIDEWARDS;
			position.z -= direction.x * speed * SIDEWARDS;
		}
	}

	if (glutKeyIsDown('d')){ // Move right
		if (dot(direction,collision_vector) < 0)
		{
			collision_factor = 1;
			sideways_factor = 1;
			collision_object = false;
		}
		if (collision_object)
		{
			position.x -= direction.z * speed * SIDEWARDS * sqrt(pow(sideways_factor,2)) * sqrt(pow(collision_vector.x,2));
			position.z += direction.x * speed * SIDEWARDS * sqrt(pow(sideways_factor,2)) * sqrt(pow(collision_vector.z,2));
		}
		else
		{
			position.x -= direction.z * speed * SIDEWARDS;
			position.z += direction.x * speed * SIDEWARDS;
		};
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


/**
*		create_wall:
*			Used to add a wall to the list of all hitboxes that the player cannot cross
*			IMPORTANT: Walls can be crossed from one side.
*
**/
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


	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
	glutInitContextVersion(3, 2);
	glutInitWindowSize (HEIGHT, WIDTH);
	glutDisplayFunc(display);
	glutCreateWindow ("Cool 3D World");
	glutPassiveMotionFunc(mouseMove);
	init();


	printf("TEXWIDTH: %d\n", texWidth);
	// 4 World limits
	create_wall(0, -20, 0, 0, -20, texWidth-1, 100); // Z Axis from origin
	create_wall(texWidth-1, -20, 0, 0, -20, 0, 100); // X Axis from origin
	create_wall(texWidth-1, -20, texWidth-1, texWidth-1, -20, 0, 100); // Z Axis
	create_wall(0, -20, texWidth-1, texWidth-1, -20, texWidth-1, 100); // X Axis

	loadTextures(); //for skybox
	glutTimerFunc(20, &OnTimer, 0);
	glutMainLoop();
	return 0;
}
