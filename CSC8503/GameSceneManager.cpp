#include "GameSceneManager.h"
#include "GameWorld.h"
#include "TextureLoader.h"

#include "PlayerObject.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "AnimationObject.h"
#include "LevelManager.h"
#include "UI.h"

#include <irrKlang.h>

using namespace NCL;
using namespace CSC8503;
using namespace irrklang;

namespace {
	constexpr float PLAYER_MESH_SIZE = 3.0f;
	constexpr float PLAYER_INVERSE_MASS = 0.5f;
}

GameSceneManager::GameSceneManager(bool isInitingAssets) : mController(*Window::GetWindow()->GetKeyboard(), *Window::GetWindow()->GetMouse()) {
	mWorld = new GameWorld();
	mRenderer = new GameTechRenderer(*mWorld);
	mPhysics = new PhysicsSystem(*mWorld);
	mPhysics->UseGravity(true);
	mAnimation = new AnimationSystem(*mWorld);
	mLevelManager = new LevelManager();

	mWorld->GetMainCamera().SetController(mController);
	mController.MapAxis(0, "Sidestep");
	mController.MapAxis(1, "UpDown");
	mController.MapAxis(2, "Forward");
	mController.MapAxis(3, "XLook");
	mController.MapAxis(4, "YLook");

	if (isInitingAssets) {
		InitialiseAssets();
	}
}

GameSceneManager::~GameSceneManager() {
	delete mCubeMesh;
	delete mSphereMesh;
	delete mCapsuleMesh;
	delete mCharMesh;
	delete mEnemyMesh;
	delete mBonusMesh;
	delete mSoldierAnimation;
	delete mSoldierMaterial;
	delete mSoldierMesh;
	delete mSoldierShader;

	delete mBasicTex;
	delete mBasicShader;
	delete mKeeperAlbedo;
	delete mKeeperNormal;
	delete mFloorAlbedo;
	delete mFloorNormal;

	delete mPhysics;
	delete mRenderer;
	delete mWorld;
	delete mAnimation;

	delete mInventorySlotTex;
	delete mHighlightAwardTex;
	delete mLightOffTex;
	delete mMakingNoiseTex;
	delete mSilentRunTex;
	delete mSlowDownTex;
	delete mStunTex;
	delete mSwapPositionTex;

	delete mSuspensionBarTex;
	delete mSuspensionIndicatorTex;
}

void GameSceneManager::UpdateGame(float dt) {
	mWorld->GetMainCamera().UpdateCamera(dt);

	InitIcons();

	if (mGameState == MainMenuState)
		DisplayMainMenu();

	if (mGameState == VictoryScreenState)
		DisplayVictory();

	if (mGameState == DefeatScreenState)
		DisplayDefeat();

	if ((mUpdatableObjects.size() > 0) && mGameState == LevelState) {
		for (GameObject* obj : mUpdatableObjects) {
			obj->UpdateObject(dt);
		}
	}

	mWorld->UpdateWorld(dt);
	mRenderer->Update(dt);
	mPhysics->Update(dt);
	mAnimation->Update(dt);
	mRenderer->Render();
	Debug::UpdateRenderables(dt);
}

void GameSceneManager::InitialiseAssets() {
	mCubeMesh = mRenderer->LoadMesh("cube.msh");
	mSphereMesh = mRenderer->LoadMesh("sphere.msh");
	mCapsuleMesh = mRenderer->LoadMesh("Capsule.msh");
	mCharMesh = mRenderer->LoadMesh("goat.msh");
	mEnemyMesh = mRenderer->LoadMesh("Keeper.msh");
	mBonusMesh = mRenderer->LoadMesh("apple.msh");
	mCapsuleMesh = mRenderer->LoadMesh("capsule.msh");

	mBasicTex = mRenderer->LoadTexture("checkerboard.png");
	mKeeperAlbedo = mRenderer->LoadTexture("fleshy_albedo.png");
	mKeeperNormal = mRenderer->LoadTexture("fleshy_normal.png");
	mFloorAlbedo = mRenderer->LoadTexture("panel_albedo.png");
	mFloorNormal = mRenderer->LoadTexture("panel_normal.png");

	mBasicShader = mRenderer->LoadShader("scene.vert", "scene.frag");

	mSoldierMesh = mRenderer->LoadMesh("Role_T.msh");
	mSoldierAnimation = mRenderer->LoadAnimation("Role_T.anm");
	mSoldierMaterial = mRenderer->LoadMaterial("Role_T.mat");
	mSoldierShader = mRenderer->LoadShader("SkinningVertex.glsl", "scene.frag");

	//icons
	mInventorySlotTex = mRenderer->LoadTexture("InventorySlot.png");
	mHighlightAwardTex = mRenderer->LoadTexture("HighlightAward.png");
	mLightOffTex = mRenderer->LoadTexture("LightOff.png");
	mMakingNoiseTex = mRenderer->LoadTexture("MakingNoise.png");
	mSilentRunTex = mRenderer->LoadTexture("SilentRun.png");
	mSlowDownTex = mRenderer->LoadTexture("SlowDown.png");
	mStunTex = mRenderer->LoadTexture("Stun.png");
	mSwapPositionTex = mRenderer->LoadTexture("SwapPosition.png");

	mSuspensionBarTex = mRenderer->LoadTexture("SuspensionBar.png");
	mSuspensionIndicatorTex = mRenderer->LoadTexture("SuspensionPointer.png");


	InitCamera();
	//CreateLevel();
}

void GameSceneManager::InitCamera() {
	mWorld->GetMainCamera().SetNearPlane(0.1f);
	mWorld->GetMainCamera().SetFarPlane(500.0f);
	mWorld->GetMainCamera().SetPitch(-15.0f);
	mWorld->GetMainCamera().SetYaw(315.0f);
	mWorld->GetMainCamera().SetPosition(Vector3(-60, 40, 60));
}

void GameSceneManager::InitIcons() {
	UI::Icon mInventoryIcon1 = UI::AddIcon(Vector2(45, 90), 4.5, 8, mInventorySlotTex);
	UI::Icon mInventoryIcon2 = UI::AddIcon(Vector2(50, 90), 4.5, 8, mInventorySlotTex);

	UI::Icon mHighlightAwardIcon = UI::AddIcon(Vector2(3, 84), 4.5, 7, mHighlightAwardTex, false);
	UI::Icon mLightOffIcon = UI::AddIcon(Vector2(8, 84), 4.5, 7, mLightOffTex, false);
	UI::Icon mMakingNoiseIcon = UI::AddIcon(Vector2(13, 84), 4.5, 7, mMakingNoiseTex, false);
	UI::Icon mSilentRunIcon = UI::AddIcon(Vector2(18, 84), 4.5, 7, mSilentRunTex, false);
	UI::Icon mSlowDownIcon = UI::AddIcon(Vector2(3, 92), 4.5, 7, mSlowDownTex, false);
	UI::Icon mStunIcon = UI::AddIcon(Vector2(8, 92), 4.5, 7, mStunTex, false);
	UI::Icon mSwapPositionIcon = UI::AddIcon(Vector2(13, 92), 4.5, 7, mSwapPositionTex, false);

	UI::Icon mSuspensionBarIcon = UI::AddIcon(Vector2(90, 16), 12, 75, mSuspensionBarTex);
	UI::Icon mSuspensionIndicatorIcon = UI::AddIcon(Vector2(93, 86), 5, 5, mSuspensionIndicatorTex);
}

void GameSceneManager::CreateLevel() {
	mLevelManager->LoadLevel(0, mWorld, mCubeMesh, mFloorAlbedo, mFloorNormal, mBasicShader);
}

void GameSceneManager::DisplayMainMenu() {
	// to be replaced by proper UI
	mWorld->ClearAndErase();
	mPhysics->Clear();
	std::cout << "WELCOME" << std::endl;
	std::cout << "PRESS SPACE TO PLAY" << std::endl;
}

void GameSceneManager::DisplayVictory() {
	// to be replaced by proper UI
	mWorld->ClearAndErase();
	mPhysics->Clear();
	std::cout << "VICTORY!!!!!!!" << std::endl;
}

void GameSceneManager::DisplayDefeat() {
	// to be replaced by proper UI
	mWorld->ClearAndErase();
	mPhysics->Clear();
	std::cout << "defeat :(((((((" << std::endl;
}

PlayerObject* GameSceneManager::AddPlayerToWorld(const Vector3 position, const std::string& playerName) {
	return nullptr;
}

void GameSceneManager::CreatePlayerObjectComponents(PlayerObject& playerObject, const Vector3& position) const {
	CapsuleVolume* volume  = new CapsuleVolume(1.4f, 1.0f);

	playerObject.SetBoundingVolume((CollisionVolume*)volume);

	playerObject.GetTransform()
		.SetScale(Vector3(PLAYER_MESH_SIZE, PLAYER_MESH_SIZE, PLAYER_MESH_SIZE))
		.SetPosition(position);

	playerObject.SetRenderObject(new RenderObject(&playerObject.GetTransform(), mEnemyMesh, mKeeperAlbedo, mKeeperNormal, mBasicShader, PLAYER_MESH_SIZE));
	playerObject.SetPhysicsObject(new PhysicsObject(&playerObject.GetTransform(), playerObject.GetBoundingVolume(), 1, 1, 5));


	playerObject.GetPhysicsObject()->SetInverseMass(PLAYER_INVERSE_MASS);
	playerObject.GetPhysicsObject()->InitSphereInertia(false);
}

GuardObject* GameSceneManager::AddGuardToWorld(const Vector3 position, const std::string& guardName) {
	return nullptr;
}

void GameSceneManager::CreateGuardObjectComponents(PlayerObject& playerObject, const Vector3& position) const {
}

GameObject* GameSceneManager::AddFloorToWorld(const Vector3& position, const std::string& objectName) {
	GameObject* floor = new GameObject(objectName);

	Vector3 floorSize = Vector3(120, 2, 120);
	AABBVolume* volume = new AABBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(floorSize * 2)
		.SetPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), mCubeMesh, mFloorAlbedo, mFloorNormal, mBasicShader, 120));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume(), 1, 2, 2));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	floor->GetRenderObject()->SetColour(Vector4(0.2f, 0.2f, 0.2f, 1));

	mWorld->AddGameObject(floor);

	return floor;
}
