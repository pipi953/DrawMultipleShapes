// DrawMultipleShapes.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "Shader.h"
//#include "ModelData.h"

#include <iostream>
#include <math.h>
#include <vector>

# define M_PI   3.14159265358979323846  /* pi */

//	使用宏定义 决定使用传统渲染 或 模板缓冲区渲染
//#define USE_STENCIL_BUFFER
#define NOT_USE_STENCIL_BUFFER
 
//	callback function
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void inputProcess(GLFWwindow *window);

float* ModelVerts;       //	模型的顶点数组
std::vector<float> ve;
//	标记模型加载是否完毕
bool mark_ModelLoadEnd = false;
int sizeOfModelVerts = 0;    //	模型的顶点数组的大小
//	加载模型
void loadModels(char const* path);
//  加载图片
GLuint loadTexture(char const* path);

//	window settings
const unsigned int SCR_WIDTH = 720;
const unsigned int SCR_HEIGHT = 720;

// 摄像机信息
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

bool firstMouse = true;
float yaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch = 0.0f;
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
float fov = 45.0f;

// 键盘移动操作的 计时器
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

//	渲染速度测试的 计时器
LARGE_INTEGER t1, t2, tc;	//	 计时器声明
int count = 0;	//	渲染次数
float useTime = 0;	//	一次绘制的耗时
float useTimeSum = 0;	//	消耗时间总和

// 圆形分割的数量，分成 360 份，可由 360 个线段组成空心圆，也可以由 360 个三角形组成实心圆
int VERTEX_DATA_NUM = 360;
// 360 个顶点的位置，因为有 x 和 y 坐标，所以 double 一下，再加上中心点 和 闭合的点
float* circleVertex = new float[VERTEX_DATA_NUM * 2 + 4];
// 分成 360 份，每一份的弧度
float radian = (float)(2 * M_PI / VERTEX_DATA_NUM);
// 绘制的半径
float radius = 0.5f;

// 初始化圆形的顶点数据
void initVertexData() {
	// 中心点
	circleVertex[0] = 0.0f;
	circleVertex[1] = 0.0f;
	// 圆的 360 份的顶点数据
	for (int i = 0; i < VERTEX_DATA_NUM; i++) {
		circleVertex[2 * i + 2] = (float)(radius * cos(radian * i));
		circleVertex[2 * i + 1 + 2] = (float)(radius * sin(radian * i));
	}
	// 闭合点
	circleVertex[VERTEX_DATA_NUM * 2 + 2] = (float)(radius * cos(radian));
	circleVertex[VERTEX_DATA_NUM * 2 + 3] = (float)(radius * sin(radian));
}

int main()
{
	//	程序开始， 首先读取模型的坐标信息与纹理坐标信息
	loadModels("Resources/Models/Stanford_Bunny/bunny.obj");

	if (mark_ModelLoadEnd)
	{
		//	glfw init
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif // __APPLE

		//	create a window with glfw
		GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "StencilTesting", NULL, NULL);
		if (window == NULL)
		{
			std::cout << "Failed to create GLFW window" << std::endl;
			glfwTerminate();
			return -1;
		}
		glfwMakeContextCurrent(window);
		glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
		glfwSetCursorPosCallback(window, mouse_callback);
		glfwSetScrollCallback(window, scroll_callback);

		// tell GLFW to capture our mouse
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		//	glad: load all OpenGL function pointers 
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			std::cout << "Failed to initialize GLAD" << std::endl;
			return -1;
		}

		//	1. 配置全局opengl状态  -- 开启深度测试
		glEnable(GL_DEPTH_TEST);

#ifdef USE_STENCIL_BUFFER
		/*
		//	glDepthFunc(GLenum Func) 函数说明
		*	通过目标像素与当前像素在 Z方向上值大小 的比较是否满足参数指定的条件，来决定在深度（Z方向）上是否绘制该目标像素。
		*	该函数只有开启 “深度测试” 时才有效
		*/
		glDepthFunc(GL_LESS);	//	深度测试函数，如果目标像素 z值 < 当前像素 z值， 则绘制目标像素
		glEnable(GL_STENCIL_TEST);	//	开启模板测试

		//	2. 设置模板函数
		/*	//	模板函数
		 *	和深度测试一样，我们也有几个不同控制权，决定何时模板测试通过或失败，以及它将怎样影响模板缓冲，
		 * 一共有两种函数可供我们去配置模板测试：glStencilFunc() 和 glStencil0p()
		 */

		 /*	//	glStencilFunc(GLenum func, GLint ref, GLuint mask):
		 *	参数含义：
		 *	func： 设置模板测试函数，这个测试函数将会应用到已经储存的模板值 和 glStencilFunc的 ref值上
		 *	ref： 设置了模板测试的参考值，模板缓冲的内容会与这个值对比
		 * mask： 指定一个遮罩，在模板测试对比引用值和储存的值前，对他们进行按位与(and)操作，初始值为1
		 */
		glStencilFunc(GL_NOTEQUAL, 0, 0xFF);	//	代码含义： 只有参考值 != 模板缓冲区的值&mask时才通过	0xFF: 允许写入缓冲区

		//	3. 设置模板测试通过与否的后续执行操作
		/*	//	void glStencilOp(GLenum sfail, GLenum zfail, GLenum zpass)，
		*	参数含义：
		*	sfail：当模板测试失败时所执行的操作
		*	zfail：当模板测试通过，但深度测试失败时所执行的操作
		*	zpass：当模板测试 与 深度测试都通过时执行的操作
		*/
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);	//	GL_KEEP： 保持当前模板缓冲区的值, GL_REPLACE：用 glStencilFunc()函数指定的 ref参考值 替换模板缓区的值

#endif // USE_STENCIL_BUFFER



	//	初始化 圆形着色器（孔洞）	红色
		Shader circleShader("circleShader.vert", "circleShader.frag");
		//	初始化 方形着色器（物体）   黄色
		Shader squareShader("squareShader.vert", "squareShader.frag");

		circleShader.use();
		//	初始化 圆坐标数据
		initVertexData();

		unsigned int circleVAO, circleVBO;
		glGenVertexArrays(1, &circleVAO);
		glGenBuffers(1, &circleVBO);
		glBindVertexArray(circleVAO);
		glBindBuffer(GL_ARRAY_BUFFER, circleVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * (VERTEX_DATA_NUM * 2 + 4), circleVertex, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);


		squareShader.use();
		/*
		//	初始化	正方形坐标数据
		float squareVertex[] = {
			////	三角形 1
			////	position
			//-0.5, 0.5,
			//-0.5, -0.5,
			//0.5, -0.5,
			////	三角形 2
			//0.5, -0.5,
			//0.5, 0.5,
			//-0.5, 0.5

			// positions          // texture Coords
			-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
			 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
			 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
			 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
			-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
			-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

			-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
			 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
			 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
			 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
			-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
			-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

			-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
			-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
			-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
			-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
			-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
			-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

			 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
			 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
			 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
			 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
			 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
			 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

			-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
			 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
			 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
			 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
			-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
			-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

			-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
			 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
			 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
			 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
			-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
			-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
		};
		 */

		unsigned int squareVAO, squareVBO;
		glGenVertexArrays(1, &squareVAO);
		glGenBuffers(1, &squareVBO);
		glBindVertexArray(squareVAO);
		glBindBuffer(GL_ARRAY_BUFFER, squareVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeOfModelVerts * sizeof(float), ModelVerts, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		//unsigned int cubeTexture = loadTexture("Resources/Textures/metal.jpg");
		unsigned int cubeTexture = loadTexture("Resources/Models/Stanford_Bunny/bunny.jpg");

		squareShader.use();
		squareShader.setInt("texture1", 0);


		//	计时
		FILE *fp;
		fopen_s(&fp, "useTime.h", "w");
		fprintf(fp, "float time_30[] = {\n");


		while (!glfwWindowShouldClose(window))
		{
			//	每帧时间 逻辑
			float currentFrame = glfwGetTime();
			deltaTime = currentFrame - lastFrame;
			lastFrame = currentFrame;

			//	输入事件的处理
			inputProcess(window);

			//	测试时间	
			QueryPerformanceFrequency(&tc);
			QueryPerformanceCounter(&t1);


#ifdef NOT_USE_STENCIL_BUFFER
			glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// 清除颜色缓冲区 深度缓冲区 模板缓冲区

			 //	----------------------- 传统绘制 -----------------------------------

			 //	设置相机参数，并绑定到着色器
			glm::mat4 projection = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
			squareShader.setMat4("projection", projection);	//	绑定投影
			// camera/view transformation
			glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
			squareShader.setMat4("view", view);	//	绑定视图

			// render the loaded model
			glm::mat4 model = glm::mat4(1.0f);
			//model = glm::translate(model, glm::vec3(0.0f, -1.75f, 0.0f)); // translate it down so it's at the center of the scene
			//model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));	// it's a bit too big for our scene, so scale it down
			squareShader.setMat4("model", model);

			circleShader.use();
			//	画圆
			glBindVertexArray(circleVAO);
			glDrawArrays(GL_TRIANGLE_FAN, 0, VERTEX_DATA_NUM * 2 + 4);

			squareShader.use();	//	激活 正方形着色器 (shader)
			glBindVertexArray(squareVAO);
			glDrawArrays(GL_TRIANGLES, 0, sizeOfModelVerts);

			glBindVertexArray(0);

#endif // NOT_USE_STENCIL_BUFFER






#ifdef USE_STENCIL_BUFFER

			//	render
			glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);	// 清除颜色缓冲区 深度缓冲区 模板缓冲区

			//	设置相机参数，并绑定到着色器
			glm::mat4 model = glm::mat4(1.0f);
			glm::mat4 projection = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
			squareShader.setMat4("projection", projection);	//	绑定投影
			// camera/view transformation
			glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
			squareShader.setMat4("view", view);	//	绑定视图

			//	关闭线框模式（绘制圆孔）
			//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			//	------------------------ 使用指定模板缓冲区的绘制 ------------------------------------
			//  1. 渲染 红色圆形图案，并写入模板缓冲区
			circleShader.use(); //  激活圆形的着色器，并设置参数
			glStencilFunc(GL_ALWAYS, 1, 0xFF);  //  设置其数据总是通过模板缓冲区，将1首先填充缓冲区
			glStencilMask(0xFF);    //  绘制物体时，允许实时写入模板缓冲区
			glBindVertexArray(circleVAO);
			glDrawArrays(GL_TRIANGLE_FAN, 0, VERTEX_DATA_NUM * 2 + 4);
			glBindVertexArray(0);

			//	开启线框模式（绘制正方体）
			//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

			//	2. 渲染过程，现在对正方形绘制，此次禁止模板写入
			squareShader.use(); //  激活方形着色器
			glStencilFunc(GL_NOTEQUAL, 0, 0xFF);	//	参考值不等于模板缓冲区的值时，允许通过
			glStencilMask(0x00);	//	绘制物体时，禁止修改深度缓冲区
			glDisable(GL_DEPTH_TEST);

			float scale = 1;
			model = glm::mat4(1.0f);
			model = glm::scale(model, glm::vec3(scale, scale, scale));
			squareShader.setMat4("model", model);
			glBindVertexArray(squareVAO);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, cubeTexture);
			glDrawArrays(GL_TRIANGLES, 0, sizeOfModelVerts);

			glBindVertexArray(0);

			glStencilMask(0xFF);	//	深度缓冲区 关闭修改
			glEnable(GL_DEPTH_TEST);	//	开启深度测试

#endif // USE_STENCIL_BUFFER


		//	测定渲染时间 -- 计时器
			//printf("~~~~~~~~ count = %d\n", count);
			QueryPerformanceCounter(&t2);
			if (count >= 100 && count < 500)
			{
				useTime = (t2.QuadPart - t1.QuadPart)*1.0 / tc.QuadPart;
				useTimeSum += useTime;
				if (count != 100 && count % 5 == 0)
					fprintf(fp, "\n");
				fprintf(fp, "%f,  ", useTime);
				if (count == 499) {
					fprintf(fp, "\n};\n\n");
					fprintf(fp, "float useTimeSum = %f, AveTime = %f;", useTimeSum, useTimeSum / 400);
					fclose(fp);
				}
			}


			glfwSwapBuffers(window);

			//	渲染计数器
			count++;

			glfwPollEvents();
		}

		glDeleteVertexArrays(1, &circleVAO);
		glDeleteBuffers(1, &circleVBO);
		glDeleteVertexArrays(1, &squareVAO);
		glDeleteBuffers(1, &squareVBO);

		glfwTerminate();
	}
	return 0;
}


//	glfw: 窗口大小发生变化（通过OS或用户调整大小）时，执行此回调函数
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	//	确保视口与新窗口尺寸匹配，请注意宽度和高度将远远大于视网膜显示器上指定的高度。
	glViewport(0, 0, width, height);
}


//	处理所有输入：查询GLFW是否按下/释放相关键并相应地做出反应
void inputProcess(GLFWwindow *window) {
	float cameraSpeed = 2.0f * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cameraPos += cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cameraPos -= cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}


// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (fov >= 1.0f && fov <= 45.0f)
		fov -= yoffset;
	if (fov <= 1.0f)
		fov = 1.0f;
	if (fov >= 45.0f)
		fov = 45.0f;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.1f; // change this value to your liking
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	// make sure that when pitch is out of bounds, screen doesn't get flipped
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);
}

GLuint loadTexture(char const* path) {
	//	创建纹理并加载
	unsigned int textureID;
	glGenTextures(1, &textureID);

	//	加载图片，	创建纹理，并生成mipmap
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);		//	告诉stb_image.h，在y轴上反转加载纹理。
	unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
	if (data)
	{
		GLenum format;
		if (nrChannels == 1)
			format = GL_RED;
		else if (nrChannels == 3)
			format = GL_RGB;
		else if (nrChannels == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		//	设置其他参数
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//	释放资源
		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}
    return textureID;
}


//	加载模型
void loadModels(char const* path) {

	mark_ModelLoadEnd = false;
	sizeOfModelVerts = 0;

	FILE* fp;
	fopen_s(&fp, "ModelData.h", "w");
	fprintf(fp, "float ModelVerts[] = {\n");

	Assimp::Importer importer;
	importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);
	importer.SetPropertyInteger(AI_CONFIG_PP_PTV_NORMALIZE, true);

	//unsigned flags = aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals;
	unsigned flags = aiProcess_Triangulate | aiProcess_GenNormals;

	const aiScene* scene = importer.ReadFile(path, flags);

	if (!scene) {
		printf("model load error.\n");
		system("pause");
		return;
	}
	const aiMesh* mesh = scene->mMeshes[0];

	uint32_t numFaces = 0;
	std::unique_ptr<uint32_t[]> vertsIndex(new uint32_t[3 * mesh->mNumFaces]);

	numFaces = mesh->mNumFaces;
	for (unsigned i = 0; i < numFaces; i++) {
		vertsIndex[3 * i] = mesh->mFaces[i].mIndices[0];
		vertsIndex[3 * i + 1] = mesh->mFaces[i].mIndices[1];
		vertsIndex[3 * i + 2] = mesh->mFaces[i].mIndices[2];
	}

	sizeOfModelVerts += mesh->mNumVertices * 5;
	std::cout << "~~~~~~~~~~" << mesh->mNumFaces << " " << std::endl;

	for (unsigned i = 0; i < mesh->mNumVertices; i++) {
		aiVector3D tmp = mesh->mVertices[vertsIndex[i]];
		ve.push_back(tmp.x);
		ve.push_back(tmp.y);
		ve.push_back(tmp.z);

		if (mesh->mTextureCoords[0]) {	//	网格是否包含纹理坐标
			// 顶点最多可包含8个不同的纹理坐标。 因此，我们假设我们不会使用顶点可以具有多个纹理坐标的模型，因此我们总是采用第一个集合（0）。
			ve.push_back(mesh->mTextureCoords[0][i].x);
			ve.push_back(mesh->mTextureCoords[0][i].y);
		}
		else
		{
			ve.push_back(0.0);
			ve.push_back(0.0);
		}

		//std::cout << tmp.x << " , " << tmp.y << " , " << tmp.z << std::endl;
		fprintf(fp, "%ff, %ff, %ff, ", tmp.x, tmp.y, tmp.z);

		fprintf(fp, " %ff, %ff,      ", mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
		if ((i + 1) % 3 == 0)
			fprintf(fp, "\n");
	}

	fprintf(fp, "};\n\n");
	fclose(fp);

	ModelVerts = new float[sizeOfModelVerts * sizeof(float)];
	if (!ve.empty()) {
		memcpy(ModelVerts, &ve[0], ve.size() * sizeof(float));
	}
	
	mark_ModelLoadEnd = true;

}


// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门提示: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
