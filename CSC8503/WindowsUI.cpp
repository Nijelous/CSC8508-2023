#include "WindowsUI.h"

#include "Win32Window.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_win32.h"

WindowsUI::WindowsUI() {
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;    // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer backends
	const HWND windowHandle = NCL::Win32Code::Win32Window::windowHandle;
	ImGui_ImplWin32_InitForOpenGL(windowHandle);
	ImGui_ImplOpenGL3_Init();
}

WindowsUI::~WindowsUI() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}


void WindowsUI::RenderUI(std::function<void()> callback) {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();


	// Next window to be created will cover the entire screen
	NCL::Maths::Vector2i windowSize = NCL::Window::GetWindow()->GetScreenSize();
	int windowWidth = windowSize.x;
	int windowHeight = windowSize.y;

	ImVec2 size(windowWidth, windowHeight);
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(size);

	ImGui::Begin("Background", NULL,
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoBackground |
		ImGuiWindowFlags_NoResize
	);

	if (callback != nullptr) {
		callback();
	}
	//CLEAR

	ImGui::End();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
