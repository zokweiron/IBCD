

#include "DepthPeelingScene.h"

#define DIVIDE 1

DPScene::DPScene(int width, int height, const char * title):
	Scene(width, height, title)
{
	this->width = width;
	this->height = height;
}

void DPScene::mouseButtonCallback(int button, int action, int mods)
{
	double x, y;
	glfwGetCursorPos(window, &x, &y);
	camera.SetPos(button, action, x, y);
}

void DPScene::cursorPositionCallback(double x, double y)
{
	camera.Move2D(x, y);
}

void DPScene::keyCallback(int key, int scancode, int action, int mods)
{
	switch (key)
	{
	case GLFW_KEY_W:
		camera.Move(FORWARD);
		break;
	case GLFW_KEY_A:
		camera.Move(LEFT);
		break;
	case GLFW_KEY_S:
		camera.Move(BACK);
		break;
	case GLFW_KEY_D:
		camera.Move(RIGHT);
		break;
	case GLFW_KEY_LEFT_CONTROL:
		camera.Move(DOWN);
		break;
	case GLFW_KEY_SPACE:
		camera.Move(UP);
		break;
	}
}

void DPScene::initializeScene()
{
	ADSShader = Shader::LoadShaderFromFile("shader/ADS");
	models.push_back(std::unique_ptr<Model>{new Model{ "models/", "armadillo.model" }});
	models.push_back(std::unique_ptr<Model>{new Model{ "models/", "teapot.model" }});
	models[0]->setPosition(glm::vec3{ 3,0,0 });

	light.eyecoords = glm::vec3{ 0,3,0 };
	light.La = glm::vec3{ 0.5,0.5,0.5 };
	light.Ld = glm::vec3{ 0.8,0.8,0.8 };
	light.Ls = glm::vec3{ 0.7,0.8,0.7 };

	camera.SetClipping(0.1, 100);
	camera.SetPosition(glm::vec3{ 0,0,5 });
	camera.SetViewport(0, 0, width, height);

	glClearColor(119 / 255.0, 136 / 255.0, 153 / 255.0, 1);

	glGenFramebuffers(1, &FrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, FrameBuffer);
	glGenTextures(1, &colorTexture);
	glBindTexture(GL_TEXTURE_2D, colorTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width / DIVIDE, height / DIVIDE, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	/*GLuint depthRenderBuffer;
	glGenRenderbuffers(1, &depthRenderBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width / DIVIDE, height / DIVIDE);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderBuffer);*/
	glGenTextures(1, &depthTexture);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width / DIVIDE, height / DIVIDE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, colorTexture, 0);
	GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, DrawBuffers);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		throw std::runtime_error("Framebuffer status");
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DPScene::process(double t)
{
	camera.Update();
}

void DPScene::render()
{
	glfwMakeContextCurrent(window);
	

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_STENCIL_TEST);

	glUseProgram(ADSShader);

	glUniform3fv(0, 1, glm::value_ptr(light.eyecoords));
	glUniform3fv(1, 1, glm::value_ptr(light.La)); 
	glUniform3fv(2, 1, glm::value_ptr(light.Ld));
	glUniform3fv(3, 1, glm::value_ptr(light.Ls));

	glBindFramebuffer(GL_FRAMEBUFFER, FrameBuffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glViewport(0, 0, width / DIVIDE, height / DIVIDE);
	models[0]->render(ADSShader, camera);
	models[1]->render(ADSShader, camera);
	glViewport(0, 0, width, height);
	//Copy
	glBindFramebuffer(GL_READ_FRAMEBUFFER, FrameBuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glBlitFramebuffer(0, 0, width / DIVIDE, height / DIVIDE, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);


	glfwSwapBuffers(window);
}
