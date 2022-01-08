#include <vector>
#include <string>

#include "glad/include/glad.h"
#include "GL/include/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "obj_loader.h"

#include "mesh.h"
#include "light.h"

using namespace std;
using namespace glm;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

const unsigned int SCREEN_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

const char *vertexShaderSource = "#version 330 core\n"
	"layout (location = 0) in vec3 aPos;\n"
	"layout (location = 1) in vec3 aNormal;\n"
	"layout (location = 2) in vec2 aTexCoords;\n"
	"uniform mat4 model;\n"
	"uniform mat4 view;\n"
	"uniform mat4 projection;\n"
	"out vec3 FragPos;\n"
	"out vec3 Normal;\n"
	"out vec2 TexCoords;\n"
	"void main()\n"
	"{\n"
	"  gl_Position = projection * view * model * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
	"  FragPos = vec3(model * vec4(aPos, 1.0));\n"
	"  Normal = mat3(transpose(inverse(model))) * aNormal;\n"
	"  TexCoords = aTexCoords;\n"
	"}\0";

const char *fragmentShaderSource = "#version 330 core\n"
	"out vec4 FragColor;\n"
	"in vec3 FragPos;\n"
	"in vec3 Normal;\n"
	"in vec2 TexCoords;\n"
	"uniform sampler2D diffuse;\n"
	"uniform sampler2D specular;\n"
	"uniform float shininess;\n"
	"uniform vec3 lightPos;\n"
	"uniform vec3 lightColor;\n"
	"uniform vec3 viewPos;\n"
	"void main()\n"
	"{\n"
	"  vec3 norm = normalize(Normal);\n"
	"  vec3 lightDir = normalize(lightPos - FragPos);\n"
	"  vec3 viewDir = normalize(viewPos - FragPos);\n"
	"  vec3 reflectDir = reflect(-lightDir, norm);\n"
	"  float diff = max(dot(norm, lightDir), 0.0);\n"
	"  float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);\n"
	"  vec3 a_res = vec3(0.2) * vec3(1.0, 1.0, 1.0) * vec3(texture(diffuse, TexCoords));\n"
	"  vec3 d_res = vec3(diff) * vec3(1.0, 1.0, 1.0) * vec3(texture(diffuse, TexCoords));\n"
	"  vec3 s_res = vec3(spec) * vec3(1.0, 1.0, 1.0) * vec3(texture(specular, TexCoords));\n"
	"  FragColor = vec4(a_res + d_res + s_res, 1.0);\n"
	"}\0";

const char *vLightShaderSource = "#version 330 core\n"
	"layout (location = 0) in vec3 aPos;\n"
	"uniform mat4 transform;\n"
	"void main()\n"
	"{\n"
	"  gl_Position = transform * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
	"}\0";

const char *fLightShaderSource = "#version 330 core\n"
	"out vec4 FragColor;\n"
	"uniform vec3 lightColor;\n"
	"void main()\n"
	"{\n"
	"   FragColor = vec4(lightColor, 1.0f);\n"
	"}\0";

unsigned int loadTexture(char const * path) {
	unsigned int textureID;
	int width      = 0;
	int height     = 0;
	int components = 0;
	glGenTextures(1, &textureID);

	unsigned char* data = stbi_load(path, &width, &height, &components, 0);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_image_free(data);
	return textureID;
}

float rot_coef = 1;

int main() {
	glfwInit();

	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCR_HEIGHT, "OpenGL", NULL, NULL);
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	glEnable(GL_DEPTH_TEST);

	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	unsigned int shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	unsigned int vLightShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vLightShader, 1, &vLightShaderSource, NULL);
	glCompileShader(vLightShader);

	unsigned int fLightShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fLightShader, 1, &fLightShaderSource, NULL);
	glCompileShader(fLightShader);

	unsigned int lightShaderProgram = glCreateProgram();
	glAttachShader(lightShaderProgram, vLightShader);
	glAttachShader(lightShaderProgram, fLightShader);
	glLinkProgram(lightShaderProgram);

	unsigned int ice_diff = loadTexture("./ice_diff.jpg");
	unsigned int ice_spec = loadTexture("./ice_spec.jpg");

	unsigned int stone_diff = loadTexture("./stone_diff.jpg");
	unsigned int stone_spec = loadTexture("./stone_spec.jpg");

	Light light(vec3(1.0, 1.0, 1.0), vec3(0.0, 0.0, 3.0));

	vector<Mesh*> meshes;
	Mesh cube1("./cube.obj", stone_diff, stone_spec, 1000.f, vec3(1.0, 1.0, 1.0), vec3(-3.0, 3.0, 0.0));
	Mesh cube2("./cube.obj", ice_diff, ice_spec, 1000.f, vec3(1.0, 1.0, 1.0), vec3(3.0, -3.0, 0.0));
	Mesh pyramid1("./pyramid.obj", ice_diff, ice_spec, 1000.f, vec3(1.0, 1.0, 1.0), vec3(3.0, 3.0, 0.0));
	Mesh sword("./sword.obj", ice_diff, ice_spec, 1000.f, vec3(1.0, 1.0, 1.0), vec3(1.5, -1.0, 0.0));
	Mesh pyramid2("./pyramid.obj", stone_diff, stone_spec, 1000.f, vec3(1.0, 1.0, 1.0), vec3(-3.0, -3.0, 0.0));
	Mesh cube3("./cube.obj", stone_diff, stone_spec, 1000.f, vec3(0.77, 1.2, 1.0), vec3(-3.0, -3.0, 0.0));

	meshes.push_back(&cube1);
	meshes.push_back(&sword);
	meshes.push_back(&cube2);
	meshes.push_back(&pyramid1);
	meshes.push_back(&pyramid2);
	meshes.push_back(&cube3);

	vec3 cameraPos = vec3(0.0f, 0.0f, 2.0f);
	vec3 cameraTarget = vec3(0.0f, 0.0f, 0.0f);
	vec3 cameraDirection = normalize(cameraPos - cameraTarget);
	vec3 up = vec3(0.0f, 1.0f, 0.0f);
	vec3 cameraRight = normalize(cross(up, cameraDirection));
	vec3 cameraUp = cross(cameraDirection, cameraRight);
	mat4 view = lookAt(cameraPos, cameraDirection, cameraUp);

	mat4 projection = ortho( -5.f, 5.f, -5.f, 5.f, -5.f, 5.f ); // l r b t n f

	while (!glfwWindowShouldClose(window))
	{
		processInput(window);

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUniform3f(glGetUniformLocation(shaderProgram, "lightPos"), light.pos.x, light.pos.y, light.pos.z);
		glUniform3f(glGetUniformLocation(shaderProgram, "lightColor"), light.color.x, light.color.y, light.color.z);
		glUniform3f(glGetUniformLocation(shaderProgram, "viewPos"), cameraPos.x, cameraPos.y, cameraPos.z);


		for (auto m : meshes)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m->specular);
			glUniform1i(glGetUniformLocation(shaderProgram, "specular"), 0);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, m->diffuse);
			glUniform1i(glGetUniformLocation(shaderProgram, "diffuse"), 1);
			glUniform1f(glGetUniformLocation(shaderProgram, "shininess"), m->shininess);

			mat4 model = mat4(1.0f);
			model = scale(model, m->scale);
			model = translate(model, m->translation);
			model = rotate(model, (float)glfwGetTime()*rot_coef, vec3(0.0f, 1.0f, 0.0f));
			glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &model[0][0]);
			glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, &view[0][0]);
			glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);

			glUseProgram(shaderProgram);
			m->drawMesh();
		}

		glUseProgram(lightShaderProgram);
		glUniform3fv(glGetUniformLocation(lightShaderProgram, "lightColor"), 1, &light.color[0]);
		mat4 transform = mat4(1.0f);
		transform = translate(transform, light.obj.translation);
		transform = scale(transform, light.obj.scale);
		transform = projection * view * transform;
		glUniformMatrix4fv(glGetUniformLocation(lightShaderProgram, "transform"), 1, GL_FALSE, &transform[0][0]);
		light.obj.drawMesh();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

void processInput(GLFWwindow *window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	else if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		rot_coef *= 4;
	else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		rot_coef /= 4;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}
