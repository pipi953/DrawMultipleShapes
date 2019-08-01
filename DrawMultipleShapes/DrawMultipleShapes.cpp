// DrawMultipleShapes.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"

#include <iostream>
#include <math.h>

# define M_PI           3.14159265358979323846  /* pi */

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

//	window settings
const unsigned int SCR_WIDTH = 720;
const unsigned int SCR_HEIGHT = 720;

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

	//	glad: load all OpenGL function pointers 
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	//	初始化着色器	
	Shader circleShader("circleShader.vert", "circleShader.frag");

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

	while (!glfwWindowShouldClose(window))
	{
		//	render
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		circleShader.use();

		glBindVertexArray(circleVAO);
		glDrawArrays(GL_TRIANGLE_FAN, 0, VERTEX_DATA_NUM * 2 + 4);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &circleVAO);
	glDeleteBuffers(1, &circleVBO);

	glfwTerminate();
	return 0;
}


//	glfw: 窗口大小发生变化（通过OS或用户调整大小）时，执行此回调函数
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	//	确保视口与新窗口尺寸匹配，请注意宽度和高度将远远大于视网膜显示器上指定的高度。
	glViewport(0, 0, width, height);
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
