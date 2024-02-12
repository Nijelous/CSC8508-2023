#include "GameSceneManager.h"
#include "GameWorld.h"
#include "TextureLoader.h"

#include "PlayerObject.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "AnimationObject.h"
#include "LevelManager.h"

#include <irrKlang.h>

using namespace NCL;
using namespace CSC8503;
using namespace irrklang;

namespace {
	constexpr float PLAYER_MESH_SIZE = 3.0f;
	constexpr float PLAYER_INVERSE_MASS = 0.5f;
}

GameManager::GameManager() : mController(*Window::GetWindow()->GetKeyboard(), *Window::GetWindow()->GetMouse()) {
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

	InitialiseAssets();
}

GameManager::~GameManager() {
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
}

void GameManager::UpdateGame(float dt) {
	mWorld->GetMainCamera().UpdateCamera(dt);

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

void GameManager::InitialiseAssets() {
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



	InitCamera();
	CreateLevel();
}

void GameManager::InitCamera() {
	mWorld->GetMainCamera().SetNearPlane(0.1f);
	mWorld->GetMainCamera().SetFarPlane(500.0f);
	mWorld->GetMainCamera().SetPitch(-15.0f);
	mWorld->GetMainCamera().SetYaw(315.0f);
	mWorld->GetMainCamera().SetPosition(Vector3(-60, 40, 60));
}

void GameManager::CreateLevel() {
	mLevelManager->LoadLevel(0, mWorld, mCubeMesh, mFloorAlbedo, mFloorNormal, mBasicShader);
}

void GameManager::DisplayMainMenu() {
	// to be replaced by proper UI
	mWorld->ClearAndErase();
	mPhysics->Clear();
	std::cout << "WELCOME" << std::endl;
	std::cout << "PRESS SPACE TO PLAY" << std::endl;
}

void GameManager::DisplayVictory() {
	// to be replaced by proper UI
	mWorld->ClearAndErase();
	mPhysics->Clear();
	std::cout << "VICTORY!!!!!!!" << std::endl;
}

void GameManager::DisplayDefeat() {
	// to be replaced by proper UI
	mWorld->ClearAndErase();
	mPhysics->Clear();
	std::cout << "defeat :(((((((" << std::endl;
}

PlayerObject* GameManager::AddPlayerToWorld(const Vector3 position, const std::string& playerName) {
	return nullptr;
}

void GameManager::CreatePlayerObjectComponents(PlayerObject& playerObject, const Vector3& position) const {
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

GuardObject* GameManager::AddGuardToWorld(const Vector3 position, const std::string& guardName) {
	return nullptr;
}

void GameManager::CreateGuardObjectComponents(PlayerObject& playerObject, const Vector3& position) const {
}

GameObject* GameManager::AddFloorToWorld(const Vector3& position, const std::string& objectName) {
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
