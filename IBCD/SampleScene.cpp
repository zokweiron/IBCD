#include "easylogging++.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include <GL\glew.h>
#include <GLFW\glfw3.h>

#include "ibcdexcept.h"
#include "Shader.h"
#include "SampleScene.h"

SampleScene::SampleScene(int width, int height, const char * title) :
	Scene(width, height, title)
{
	this->width = width;
	this->height = height;
}

void SampleScene::initializeScene()
{
	
	phongShader = Shader::LoadShaderFromFile("shader/basic");

	std::string objFile = "models/armadillo.model";
	std::string err = tinyobj::LoadObj(shapes, materials, objFile.c_str());
	if (!err.empty()) {
		LOG(INFO) << err;
		throw std::runtime_error("model load failed.");
	}
	
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(unsigned int) *  shapes[0].mesh.indices.size(), shapes[0].mesh.indices.data(), GL_STATIC_DRAW);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * (shapes[0].mesh.positions.size() + shapes[0].mesh.normals.size()), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * shapes[0].mesh.positions.size(), shapes[0].mesh.positions.data());
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(float) * shapes[0].mesh.positions.size(), sizeof(float) * shapes[0].mesh.normals.size(), shapes[0].mesh.normals.data());

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)(sizeof(float) * shapes[0].mesh.positions.size()));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	
}

void SampleScene::process(double t)
{
	time = t;
	
}

void SampleScene::render()
{
	
	glfwMakeContextCurrent(window);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glm::vec3 cameraPos{ 5 * glm::cos(time),0,5 * glm::sin(time) };
	glm::mat4 ModelView = glm::lookAt(cameraPos, glm::vec3{ 0,0,0 }, glm::vec3{ 0,1,0 });
	glm::mat3 NormalMatrix = glm::transpose(glm::inverse(glm::mat3(ModelView)));
	glm::mat4 Projection = glm::perspective(glm::radians(45.0f), width / (float)height, 0.1f, 100.f);
	glm::mat4 MVP = Projection * ModelView;

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	glUseProgram(phongShader);
	glm::vec4 lightPos{ 0,3,0,1};
	glm::vec3 lightIntensity{ 1,1,1 };
	glUniform4fv(0, 1, glm::value_ptr(lightPos)); //light position
	glUniform3fv(1, 1, materials[0].diffuse); //Kd
	glUniform3fv(2, 1, glm::value_ptr(lightIntensity)); //Ld

	glUniformMatrix4fv(3, 1, false, glm::value_ptr(ModelView)); //MV
	glUniformMatrix3fv(4, 1, false, glm::value_ptr(NormalMatrix)); //Normal
	glUniformMatrix4fv(5, 1, false, glm::value_ptr(Projection)); // P
	glUniformMatrix4fv(6, 1, false, glm::value_ptr(MVP)); // MVP

	glBindVertexArray(VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glDrawElements(GL_TRIANGLES, shapes[0].mesh.indices.size(), GL_UNSIGNED_INT, NULL);



	glfwSwapBuffers(window);
}