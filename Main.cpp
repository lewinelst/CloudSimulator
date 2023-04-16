#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "shader.h"
#include "model.h"
#include "camera.h"

#include "CloudScape.h"


void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void framebuffer_resize(GLFWwindow* window, int width, int height);
unsigned int loadTexture(char const* path);
unsigned int genTextureArray(std::vector <const char*> paths);
std::vector<std::vector<std::vector<glm::vec3>>> genVectorField();
unsigned int loadSkybox(std::vector<std::string> faces);

const int WIDTH = 1280;
const int HEIGHT = 720;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
bool firstMouse = true; // Keeps track of if mouse has been used yet
float lastX = WIDTH / 2; // Keeps track of mouse since last frame
float lastY = HEIGHT / 2;  // Keeps track of mouse since last frame

float deltaTime = 0.0f; // Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

int main()
{


	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // was 3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6); // was 3
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Clouds", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "GLFW Window could not be created" << std::endl;
		glfwTerminate();
		return -1;
	}
	
	glfwMakeContextCurrent(window); // Makes context on current thread. 
	glfwSetFramebufferSizeCallback(window, framebuffer_resize); // Sets resizing function
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwWindowHint(GLFW_SAMPLES, 8); // multisample buffer (4 Samples)
	
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "GLAD failed to load" << std::endl;
		return -1;
	}

	stbi_set_flip_vertically_on_load(false);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// skybox 

	float skyboxVertices[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};


	unsigned int skyVAO, skyVBO;
	glGenVertexArrays(1, &skyVAO);
	glGenBuffers(1, &skyVBO);
	glBindVertexArray(skyVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	std::vector<std::string> skyboxFaces
	{
		"Resources/Textures/skybox/right.png",
		"Resources/Textures/skybox/left.png",
		"Resources/Textures/skybox/top.png",
		"Resources/Textures/skybox/bottom.png",
		"Resources/Textures/skybox/front.png",
		"Resources/Textures/skybox/back.png"
	};
	unsigned int skyboxTexture = loadSkybox(skyboxFaces);

	Shader skyboxShader("Shaders/skybox.vert", "Shaders/skybox.frag");

	// end of skybox code 

	Shader testShader("Shaders/test.vert", "Shaders/test.frag");

	// textures
	// Transparency for textures 50 (close), 30 (mid), 15 (far) (Calk 17 pixels is brush name on photoshop) 
	// texture array
	std::vector<char const*> paths;
	//paths.push_back("Resources/Textures/Test-Textures/Red.png");
	//paths.push_back("Resources/Textures/Test-Textures/Yellow.png");
	//paths.push_back("Resources/Textures/Test-Textures/Green.png");

	//paths.push_back("Resources/Textures/Test-Textures/Test2-Close.png");
	//paths.push_back("Resources/Textures/Test-Textures/Test2-Mid.png");
	//paths.push_back("Resources/Textures/Test-Textures/Test2-Far.png");

	paths.push_back("Resources/Textures/Test-Textures/Test2-Close.png");
	paths.push_back("Resources/Textures/Test-Textures/Test2-Mid.png");
	paths.push_back("Resources/Textures/Test-Textures/Test2-Mid-Far.png");
	paths.push_back("Resources/Textures/Test-Textures/Test2-Very-Far.png");


	unsigned int textureArray = genTextureArray(paths);


	// sky
	std::vector<std::vector<std::vector<glm::vec3>>> vectorField = genVectorField();

	Model rock("Resources/Models/rock/rock.obj");
	Cloudscape sky(150, 75, 150, 0, 20, 5, textureArray, 3, vectorField);
	


	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);

		//glClearColor(0.53f, 0.81f, 0.92f, 1.0f);
		glClearColor(0.455f, 0.553f, 0.765f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// rendering commands
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)WIDTH / (float)HEIGHT, 0.1f, 500.0f); // far plane controlled from here (last value)
		glm::mat4 view = camera.GetViewMatrix();

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(80.0f, 25.0f, 80.0f));
		model = glm::scale(model, glm::vec3(5.0f, 5.0f, 5.0f));

		testShader.use();
		testShader.setMat4("projection", projection);
		testShader.setMat4("view", view);
		testShader.setMat4("model", model);

		//rock.Draw(testShader);

		sky.drawScape(deltaTime, glfwGetTime(), projection, view, camera);
		
		// skybox

		/*
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_DEPTH_CLAMP);
		skyboxShader.use();
		view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
		skyboxShader.setMat4("view", view);
		skyboxShader.setMat4("projection", projection);
		glBindVertexArray(skyVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDisable(GL_DEPTH_CLAMP);
		glDepthFunc(GL_LESS);
		*/

		glfwSwapBuffers(window);
		glfwPollEvents();

		std::cout << deltaTime << std::endl;
	}

	glfwTerminate();
	return 0;
}

unsigned int loadTexture(char const* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else {
		std::cout << "Texture could not be loaded at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

// only works with PNGs atm
unsigned int genTextureArray(std::vector <const char*> paths) 
{
	unsigned int textureID;

	int layerCount = paths.size();
	int mipCount = 1;



	// load data
	std::vector<unsigned char*> data;
	int width, height, nrComponents;
	for (const char* path : paths)
	{
		data.push_back(stbi_load(path, &width, &height, &nrComponents, 0));
	}

	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);

	glTexStorage3D(GL_TEXTURE_2D_ARRAY, mipCount, GL_RGBA8, width, height, layerCount); 

	// load up data
	for (int i = 0; i < data.size(); i++) { 
		glTextureSubImage3D(textureID, 0, 0, 0, i, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, data.at(i));
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//glBindTextureUnit(0, textureID);

	for (int i = 0; i < data.size(); i++) {
		stbi_image_free(data.at(i));
	}

	return textureID;

}

void processInput(GLFWwindow* window)
{

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);

}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void framebuffer_resize(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

std::vector<std::vector<std::vector<glm::vec3>>> genVectorField()
{
	std::vector<std::vector<std::vector<glm::vec3>>> vectorField(100, std::vector<std::vector<glm::vec3>>(100, std::vector<glm::vec3>(100, glm::vec3(0.0f, 0.0f, 0.0f))));
	//std::vector<std::vector<std::vector<glm::vec3>>> vectorField;
	for (int x = 0; x < 100; x++)
	{
		for (int y = 0; y < 100; y++)
		{
			for (int z = 0; z < 100; z++)
			{
				if ((x >= 30) && (x < 50))
				{
					if ((z >= 30) && (z < 50))
					{
						//vectorField[x][y][z] = glm::vec3(2.0f, 0.0f, (((float)x - 30) / 20) * -2.0f);
					}
					else if ((z >= 50) && (z < 70))
					{
						//vectorField[x][y][z] = glm::vec3(2.0f, 0.0f, (((float)x - 30) / 20) * 2.0f);
					}
					//std::cout << glm::to_string(glm::vec3(2.0f, 0.0f, (((float)i - 40) / 20) * -5.0f)) << std::endl;
				}
			
				else 
				{
					//vectorField[x][y][z] = glm::vec3(2.0f, 0.0f, 0.0f);
				}
			}
		}
	}

	return vectorField;
}

unsigned int loadSkybox(std::vector<std::string> faces)
{
	unsigned int ID;
	glGenTextures(1, &ID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, ID);

	int width, height, nrComponents;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrComponents, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Skybox could not be loaded" << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return ID;
}
