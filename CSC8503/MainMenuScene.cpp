#include "MainMenuScene.h"

#include <imgui/imgui.h>

#include "Debug.h"
#include "Keyboard.h"
#include "SceneManager.h"
#include "Window.h"
#include "imgui/imgui_internal.h"

using namespace NCL::CSC8503;

MainMenuScene::MainMenuScene() : Scene() {
	mCurrentOpenPanel = LevelSelection;
	mMultiplayerLobbyState = Lobby;
	mIsMultiplayerLobbyOnHost = false;
	ImGuiIO& imguiIO = ImGui::GetIO();
	mHeaderFont = imguiIO.Fonts->AddFontFromFileTTF("fonts/BebasNeue-Regular.ttf", 100.f, NULL, imguiIO.Fonts->GetGlyphRangesDefault());
	mButtonFont = imguiIO.Fonts->AddFontFromFileTTF("fonts/BebasNeue-Regular.ttf", 13.f, NULL, imguiIO.Fonts->GetGlyphRangesDefault());
	mInputFont = imguiIO.Fonts->AddFontFromFileTTF("fonts/BebasNeue-Regular.ttf", 26.f, NULL, imguiIO.Fonts->GetGlyphRangesDefault());
	imguiIO.Fonts->Build();
	InitPanelDrawFuncMap();
	Scene::InitCamera();
	InitIpAddress();
}

MainMenuScene::~MainMenuScene() {
}

void MainMenuScene::UpdateGame(float dt) {
#ifdef USEPRESPERO
	Debug::Print("1-) Start Single Player", Vector2(30, 70));
	Debug::Print("2-) Start Multi Player (Server)", Vector2(30, 75));
	Debug::Print("3-) Start Multi Player (Client)", Vector2(30, 80));
#endif

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

MainMenuScene::MultiplayerLobbyPanelStates MainMenuScene::GetMultiplayerLobbyState() const {
	return mMultiplayerLobbyState;
}

void MainMenuScene::SetMultiplayerLobbyState(MultiplayerLobbyPanelStates state) {
	mMultiplayerLobbyState = state;
}

MainMenuScene::LevelSelectionPanelStates MainMenuScene::GetLevelSelectionPanelState() const {
	return mLevelSelectionState;
}

void MainMenuScene::SetLevelSelectionPanelState(LevelSelectionPanelStates state) {
	mLevelSelectionState = state;
}

int* MainMenuScene::GetIpAdressToConnect()  {
	return mIpAddress;
}

const std::string& MainMenuScene::GetPlayerName() const {
	return mPlayerName;
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
	ImGui::SetCursorPos(ImVec2(windowSize.x * .3f, windowSize.y * .1f));
	ImGui::TextColored(ImVec4(1, 0, 0, 1), "Let's name our game!");
	ImGui::PopFont();

	ImGui::SetCursorPos(ImVec2(windowSize.x * .35f, windowSize.y * .55f));
	if (ImGui::Button("Single-player", ImVec2(windowSize.x * .3f, windowSize.y * .1f))) {
		mLevelSelectionState = LevelSelectionPanelStates::StartSingleplayer;
	}

	ImGui::SetCursorPos(ImVec2(windowSize.x * .35f, windowSize.y * .65f));
	if (ImGui::Button("Multi-player", ImVec2(windowSize.x * .3f, windowSize.y * .1f))) {
		mCurrentOpenPanel = MultiplayerLobby;
	}

	ImGui::SetCursorPos(ImVec2(windowSize.x * .35f, windowSize.y * .75f));
	if (ImGui::Button("Exit", ImVec2(windowSize.x * .3f, windowSize.y * .1f))) {
		SceneManager::GetSceneManager()->SetIsForceQuit(true);
	}
	ImGui::PopFont();
}

void MainMenuScene::DrawMultiplayerLobby() {
	ImGui::PushFont(mInputFont);
	ImVec2 windowSize = ImGui::GetWindowSize();
	ImGui::SetCursorPos(ImVec2(windowSize.x * .01f, windowSize.y * .01f));

	if (ImGui::Button("Close", ImVec2(windowSize.x * .1f , windowSize.y * .1f))) {
		mCurrentOpenPanel = MainMenuPanels::LevelSelection;
	}

	ImGui::SetCursorPos(ImVec2(windowSize.x * .3f, windowSize.y * .01f));
	if (ImGui::Button("Host", ImVec2(windowSize.x * .3f, windowSize.y * .1f))) {
		mIsMultiplayerLobbyOnHost = true;
	}

	ImGui::SetCursorPos(ImVec2(windowSize.x * .6f, windowSize.y * .01f));
	if (ImGui::Button("Join", ImVec2(windowSize.x * .3f, windowSize.y * .1f))) {
		mIsMultiplayerLobbyOnHost = false;
	}

	const ImVec2 ipInputSize(windowSize.x * .3f, windowSize.y * .020f);

	ImGui::PushItemWidth(ipInputSize.x);

	ImGuiStyle& style = ImGui::GetStyle();
	const float previousFramePadding = style.FramePadding.y;
	style.FramePadding.y = ipInputSize.y;

	ImGui::SetCursorPos(ImVec2(windowSize.x * .4f, windowSize.y * .3f));

	ImGui::InputText("Player Name", mNameInputBuffer, IM_ARRAYSIZE(mIpAdressInputBuffer), 0);

	if (!mIsMultiplayerLobbyOnHost) {
		ImGui::SetCursorPos(ImVec2(windowSize.x * .4f, windowSize.y * .5f));

		ImGui::InputText("IP Address", mIpAdressInputBuffer, IM_ARRAYSIZE(mIpAdressInputBuffer), 0);
	}

	style.FramePadding.y = previousFramePadding;

	ImGui::SetCursorPos(ImVec2(windowSize.x * .4f, windowSize.y * .8f));
	if (ImGui::Button("Enter", ImVec2(windowSize.x * .3f, windowSize.y * .2f))) {
		if (!mIsMultiplayerLobbyOnHost) {
			TranslateIpAddress();
		}
		mPlayerName = std::string(mNameInputBuffer);
		mMultiplayerLobbyState = mIsMultiplayerLobbyOnHost ? MultiplayerLobbyPanelStates::StartAsServer : MultiplayerLobbyPanelStates::StartAsClient;
	}

	ImGui::PopItemWidth();
	ImGui::PopFont();
}

void MainMenuScene::InitIpAddress() {
	mIpAddress[0] = 127;
	mIpAddress[1] = 0;
	mIpAddress[2] = 0;
	mIpAddress[3] = 1;
}

void MainMenuScene::TranslateIpAddress() {
	//(erendgrmnc): Chatgpt wrote this for me :P

	// Use strtok_s to split the IP address string
	char* token;
	char* nextToken;

	// The first call to strtok_s requires passing the input buffer and getting the first token
	token = strtok_s(mIpAdressInputBuffer, ".", &nextToken);

	int index = 0;

	while (token != NULL && index < 4) {
		// Convert the token to an integer and assign it to the ip array
		mIpAddress[index] = atoi(token);

		// Subsequent calls to strtok_s require passing NULL as the input buffer
		token = strtok_s(NULL, ".", &nextToken);
		index++;
	}
}

void MainMenuScene::TranslatePlayerName() {
	
}
