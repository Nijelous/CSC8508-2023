#include "MainMenuScene.h"

#include <imgui/imgui.h>

#include "Debug.h"
#include "Keyboard.h"
#include "Window.h"
#include "imgui/imgui_internal.h"

using namespace NCL::CSC8503;

MainMenuScene::MainMenuScene() : Scene() {
	mCurrentOpenPanel = LevelSelection;
	ImGuiIO& imguiIO = ImGui::GetIO();
	mHeaderFont = imguiIO.Fonts->AddFontFromFileTTF("fonts/BebasNeue-Regular.ttf", 100.f, NULL, imguiIO.Fonts->GetGlyphRangesDefault());
	mButtonFont = imguiIO.Fonts->AddFontFromFileTTF("fonts/BebasNeue-Regular.ttf", 13.f, NULL, imguiIO.Fonts->GetGlyphRangesDefault());
	imguiIO.Fonts->Build();
	InitPanelDrawFuncMap();
	Scene::InitCamera();

}

MainMenuScene::~MainMenuScene() {
}

void MainMenuScene::UpdateGame(float dt) {
	Scene::UpdateGame(dt);
}

void MainMenuScene::DrawCanvas() {

	mPanelDrawFuncMap[mCurrentOpenPanel]();
}

void MainMenuScene::SetOpenPanel(MainMenuPanels panel) {
	mCurrentOpenPanel = panel;
}

MainMenuScene::MainMenuPanels MainMenuScene::GetOpenPanel() const {
	return mCurrentOpenPanel;
}

void MainMenuScene::InitPanelDrawFuncMap() {
	mPanelDrawFuncMap = {
		{ MainMenuPanels::LevelSelection, [this]() { DrawLevelSelectionPanel(); }},
		{ MainMenuPanels::MultiplayerLobby, [this]() { DrawMultiplayerLobby(); }}
	};
}

void MainMenuScene::DrawLevelSelectionPanel() {
	ImVec2 windowSize = ImGui::GetWindowSize();

	ImGui::PushFont(mButtonFont);

	ImGui::PushFont(mHeaderFont);
	ImGui::SetCursorPos(ImVec2(windowSize.x * .2f, windowSize.y * .1f));
	ImGui::TextColored(ImVec4(1, 0, 0, 1), "Let's name our game!");
	ImGui::PopFont();

	ImGui::SetCursorPos(ImVec2(windowSize.x * .4f, windowSize.y * .55f));
	if (ImGui::Button("Single-player", ImVec2(windowSize.x * .3f, windowSize.y * .1f))) {
		//mCurrentOpenPanel = MultiplayerLobby;
	}

	ImGui::SetCursorPos(ImVec2(windowSize.x * .4f, windowSize.y * .65f));
	if (ImGui::Button("Multi-player", ImVec2(windowSize.x * .3f, windowSize.y * .1f))) {
		mCurrentOpenPanel = MultiplayerLobby;
	}
	ImGui::PopFont();
}

void MainMenuScene::DrawMultiplayerLobby() {
	ImGui::PushFont(mButtonFont);
	ImVec2 windowSize = ImGui::GetWindowSize();
	ImGui::SetCursorPos(ImVec2(windowSize.x * .01f, windowSize.y * .0`1f));
	if (ImGui::Button("Close", ImVec2(windowSize.x * .1f , windowSize.y * .1f))) {
		mCurrentOpenPanel = MainMenuPanels::LevelSelection;
	}
	ImGui::PopFont();
}
