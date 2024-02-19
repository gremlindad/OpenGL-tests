
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "myCamClass.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include<map>


#define STB_IMAGE_IMPLEMENTATION
#include "shaderClass.h"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "model.h"


#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"





void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);


const double myPI = (4 * atan(1));
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

Camera myCam(glm::vec3(0.0f, 3.0f, 10.0f));

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float lastFrame = 0.0f;
float deltaTime = 0.0f;
float currentFrame = 0.0f;


int main()
{


	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);//set major and minor version
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);//set major and minor version
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);



	//create window
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "open gl tutorial", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "failed to create glfw window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	//glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	void checkCompile(unsigned int id, std::string type);
	unsigned int loadCubemap(std::vector<std::string> faces);


	//IMGUI INIT


	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls



	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
	ImGui_ImplOpenGL3_Init();





	//initialize glad

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "failed to load glad" << std::endl;
		return -1;
	}

	stbi_set_flip_vertically_on_load(true);
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glEnable(GL_DEPTH_TEST);




	Shader sh("../resources/shaders/psxshaders/VERTEXmushroom.txt",
		"../resources/shaders/psxshaders/FRAGMENTmushroom.txt");

	Shader sh2("../resources/shaders/psxshaders/VERTEXgeneralPSX.txt",
		"../resources/shaders/psxshaders/FRAGMENTgeneralPSX.txt");

	Shader screenShader("../resources/shaders/ScreenQuadShader/ScreenQuadVertShader.txt",
		"../resources/shaders/ScreenQuadShader/ScreenQuadFragShader.txt");
	screenShader.setInt("screenTex", 0);



	Model mo("../resources/models/somedude/somedude.obj");
	Model lightmodel("../resources/models/lightpoint/lightpoint.obj");



	//screen quad 

	float screenQuadVerts[] =
	{
		// positions   // texCoords
			-1.0f,  1.0f,  0.0f, 1.0f,
			-1.0f, -1.0f,  0.0f, 0.0f,
			 1.0f, -1.0f,  1.0f, 0.0f,

			-1.0f,  1.0f,  0.0f, 1.0f,
			 1.0f, -1.0f,  1.0f, 0.0f,
			 1.0f,  1.0f,  1.0f, 1.0f
	};

	//create screen quad vbo vao

	unsigned int VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	//send verts to gpu
	glBufferData(GL_ARRAY_BUFFER, sizeof(screenQuadVerts), screenQuadVerts, GL_STATIC_DRAW);
	//set attribute pointers
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	//FBO for resolution downsize
	// 
	unsigned int FBO;
	glGenFramebuffers(1, &FBO);

	//bind fbo
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	//color attachment
	unsigned int screenQuadTexture;
	glGenTextures(1, &screenQuadTexture);
	glBindTexture(GL_TEXTURE_2D, screenQuadTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH/2, SCR_HEIGHT/2, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenQuadTexture, 0);


	//depth/stencil attachments
	unsigned int RBO;
	glGenRenderbuffers(1, &RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 600);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);


	//check if valid

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);











	//render loop

	while (!glfwWindowShouldClose(window))
	{

	//GET DELTA
	currentFrame = static_cast<float>(glfwGetTime());
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;



	//input
	processInput(window);


	//imgui init
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGui::Begin("Menu");
	ImGui::MenuItem("BG Colour", NULL, true);



	static float color[3] = { 0.5f,0.5f,0.5f };
	ImGui::ColorEdit3("BG Colour", color);
	
	static float modRot = 0.0f;
	ImGui::SliderFloat("Model Rotation", &modRot, 0, 2 * myPI);


	static float ambLi[3] = { 0.5f,0.5f,0.5f };
	ImGui::ColorEdit3("Ambient Light Col", ambLi); 

	static float yliPos = 3.0f;
	ImGui::SliderFloat("Light Y Pos", &yliPos, 0, 6);


	static float radius = 4.0f;
	ImGui::SliderFloat("Light Radius Pos", &radius, 2, 5);

	static float LightRotPos = 2.0f;
	ImGui::SliderFloat("Light ", &LightRotPos, 0, 2 * myPI);

	static float lightCol[3] = { 0.8f, 0.8f,0.8f};
	ImGui::ColorEdit3("Light Color", lightCol);

	static float lightInten = 0.09f;
	ImGui::SliderFloat("Light intensity ", &lightInten, 0, 3);


	glm::vec3 lightPosition = glm::vec3(sin(LightRotPos)*radius, yliPos, cos(LightRotPos)*radius);

	ImGui::MenuItem(" ", NULL, true);







	//RENDERING////////////////////////////

	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glViewport(0, 0, SCR_WIDTH/2, SCR_HEIGHT/2);


	//glfw refresh
	glEnable(GL_DEPTH_TEST);
	glClearColor(color[0], color[1], color[2], 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



	glm::mat4 model = glm::mat4(1.0f);
	model = glm::rotate(model,modRot,glm::vec3(0,1,0));


	glm::mat4 view = myCam.GetViewMatrix();
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

	sh.use();

	float scaleChange = (sin(glfwGetTime()) / 50.0) + 1;

	model = glm::translate(model, glm::vec3(0, scaleChange, 0));
	model = glm::scale(model, glm::vec3(scaleChange));

	sh.setMat4("model", model);
	sh.setMat4("view", view);
	sh.setMat4("projection", projection);
	sh.setVec3("lite.position",lightPosition);
	sh.setVec3("lite.ambient", ambLi[0],ambLi[1],ambLi[2]);
	sh.setVec3("lite.diffuse", lightCol[0],lightCol[1],lightCol[2]);
	sh.setVec3("lite.specular",1,1,1);

	sh.setFloat("lite.quad", 0.032);
	sh.setFloat("lite.lin", 0.09);
	sh.setFloat("lite.con", lightInten);

	mo.Draw(sh);


	sh2.use();

	model = glm::mat4(1.0f);
	model = glm::translate(model, lightPosition);
	model = glm::scale(model, glm::vec3(0.3));

	sh2.setMat4("model", model);
	sh2.setMat4("view", view);
	sh2.setMat4("projection", projection);
	sh2.setVec3("basicColor", 1.0f, 1.0f, 1.0f);

	lightmodel.Draw(sh2);


	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	//return to defualt frame buffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT);

	//bind the screen texture

	screenShader.use();
	glBindVertexArray(VAO);
	glBindTexture(GL_TEXTURE_2D, screenQuadTexture);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	//draw the quad



	
	ImGui::End();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());



	//glfw 

	glfwSwapBuffers(window);
	glfwPollEvents();

}



	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glDeleteFramebuffers(1, &FBO);

	glfwTerminate();
	return 0;

}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	//glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}


	const float cameraSpeed = 2.5f * deltaTime;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		myCam.ProcessKeyboard(FORWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		myCam.ProcessKeyboard(BACKWARD, deltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)//left
	{
		myCam.ProcessKeyboard(LEFT, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)//left
	{
		myCam.ProcessKeyboard(RIGHT, deltaTime);
	}

	//myCam.Position.y = 0.0f;

}
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{

	if (firstMouse) // initially set to true
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;//subtract vector of last position from current position
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	const float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	myCam.ProcessMouseMovement(xoffset,yoffset);


}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	myCam.ProcessMouseScroll(static_cast<float>(yoffset));
}




unsigned int loadCubemap(std::vector<std::string> faces)
{
unsigned int textureID;
glGenTextures(1, &textureID);
glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

int width, height, nrChannels;
for (unsigned int i = 0; i < faces.size(); i++)
{
unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
if (data)
{
glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
);
stbi_image_free(data);
}
else
{
std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
stbi_image_free(data);
}
}
glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

return textureID;
}
