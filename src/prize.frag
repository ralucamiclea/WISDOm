#version 150

out vec4 out_Color;

in vec2 exTexCoord; //texture
in vec3 exNormal; // Phong
in vec3 exSurface; // Phong (specular)
in vec3 camera_pos;
in mat4 lookAtMatrix;

uniform sampler2D texUnit;
uniform mat4 camera;

//light
uniform vec3 lightSourcesDirPosArr[4];
uniform vec3 lightSourcesColorArr[4];
uniform float specularExponent[4];
uniform bool isDirectional[4];

vec3 lightSourcesDirPosArrView[4];

void main(void)
{

	//const vec3 light = vec3(0.58, 0.58, 0.58);
	float diffuse = 0, specular = 0, shade = 0;
	vec3 vshade = vec3(0,0,0);
	vec3 vdiffuse = vec3(0,0,0);
	vec3 vspecular = vec3(0,0,0);
	
	for(int i = 0; i < 4; i++) {
		lightSourcesDirPosArrView[i] = mat3(camera) * lightSourcesDirPosArr[i];
	}
	
	// Diffuse
	for(int i = 0; i < 4; i++) {
		diffuse = dot(normalize(exNormal), lightSourcesDirPosArrView[i]);
		diffuse = max(0.0, diffuse); // No negative light
		vdiffuse += diffuse * lightSourcesColorArr[i];
	}

	// Specular
	vec3 r;
	for(int i = 0; i < 4; i++) {
		
		if (isDirectional[i]) {
			r = reflect(lightSourcesDirPosArrView[i], normalize(exNormal));
		}
		else {
			r= reflect(normalize(exSurface-lightSourcesDirPosArrView[i]),  normalize(exNormal));
		}
		
		vec3 v = normalize(exSurface); // View direction
		
		specular = dot(r, v);
		if (specular > 0.0)
			specular = pow(specular, specularExponent[i]);
		specular = max(specular, 0.0);
		vspecular += specular * lightSourcesColorArr[i];
	}
	
	vshade = 0.3 *vdiffuse + 0.7 * vspecular;
	out_Color = vec4(vshade, 1.0); //* texture(texUnit, exTexCoord);
}
