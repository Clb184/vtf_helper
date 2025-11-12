#include <iostream>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "GL/glew.h"
#include "glfw/glfw3.h"
#include <assert.h>

#include "VTFLib.h"
#include "TextureConvert.hpp"

int main() {

	// Init GLFW
	if(!glfwInit()) return -1;

	// Setup for GL 4.5
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
	int winw = 1280, winh = 960;
	// Create window
	GLFWwindow* window = glfwCreateWindow(winw, winh, "test", NULL, NULL);
	if(nullptr == window) { std::cout << "Failed creating window\n"; return -1; }

	glfwMakeContextCurrent(window);
	glewInit();
	glfwSwapInterval(1);
	
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	ImGui::StyleColorsDark();
	// Initialize ImGUI for OpenGL
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	// Configure viewport
	glViewport(0, 0, winw, winh);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	char buf[1024] = "";
	char output[1024] = "";
	

	// Initialize VTFLib
	vlInitialize();
	TextureConvert* texconv = new TextureConvert();

	while(!glfwWindowShouldClose(window)){
		// Process messages
		glfwPollEvents();

		// New ImGUI frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		
		// Window itself
		ImGui::SetNextWindowPos({0.0f, 0.0f});
		ImGui::SetNextWindowSize({(float)winw, (float)winh});
		ImGui::Begin("vtftool_root", nullptr,
			       	ImGuiWindowFlags_NoCollapse |
			       	ImGuiWindowFlags_NoResize |
			       	ImGuiWindowFlags_NoMove |
			       	ImGuiWindowFlags_MenuBar |
			       	ImGuiWindowFlags_NoTitleBar |
			       	ImGuiWindowFlags_NoBringToFrontOnFocus |
				ImGuiWindowFlags_NoSavedSettings
				);
		if(texconv) {
			if(!texconv->Move()) {
				delete texconv;
				texconv = nullptr;
			}
		}
		
		// End main window
		ImGui::End();

		// Render contents and draw
		ImGui::Render();
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Move the Swap Chain
		glfwSwapBuffers(window);
	}
	if(texconv)
	delete texconv;

	// Terminate all
	glfwDestroyWindow(window);
	window = nullptr;
	std::cout << "Ending program...\n";
	glfwTerminate();
}
