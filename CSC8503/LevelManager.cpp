#include "LevelManager.h"

#include "GameWorld.h"
#include "RecastBuilder.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "AnimationObject.h"

#include "PlayerObject.h"
#include "GuardObject.h"
#include "Helipad.h"
#include "Vent.h"
#include "Door.h"
#include "PrisonDoor.h"

#include "InventoryBuffSystem/FlagGameObject.h"
#include "InventoryBuffSystem/PickupGameObject.h"
#include "InventoryBuffSystem/InventoryBuffSystem.h"
#include "InventoryBuffSystem/SoundEmitter.h"
#include "UI.h"
#include "SoundManager.h"
#include <filesystem>

using namespace NCL::CSC8503;

LevelManager* LevelManager::instance = nullptr;

LevelManager::LevelManager() {
	mBuilder = new RecastBuilder();
	mWorld = new GameWorld();
	mRenderer = new GameTechRenderer(*mWorld);
	mPhysics = new PhysicsSystem(*mWorld);
	mPhysics->UseGravity(true);
	mAnimation = new AnimationSystem(*mWorld);
	mUi = new UI();
	mInventoryBuffSystemClassPtr = new InventoryBuffSystemClass();
	mSuspicionSystemClassPtr = new SuspicionSystemClass();

	mRoomList = std::vector<Room*>();
	for (const auto& entry : std::filesystem::directory_iterator("../Assets/Levels/Rooms")) {
		Room* newRoom = new Room(entry.path().string());
		mRoomList.push_back(newRoom);
	}
	mLevelList = std::vector<Level*>();
	for (const auto& entry : std::filesystem::directory_iterator("../Assets/Levels/Levels")) {
		Level* newLevel = new Level(entry.path().string());
		mLevelList.push_back(newLevel);
	}
	mActiveLevel = -1;

	SoundManager* a = new SoundManager();
	
	InitialiseAssets();
	InitialiseIcons();
}

LevelManager::~LevelManager() {
	for (int i = 0; i < mRoomList.size(); i++) {
		delete(mRoomList[i]);
	}
	mRoomList.clear();
	for (int i = 0; i < mLevelList.size(); i++) {
		delete(mLevelList[i]);
	}
	mLevelList.clear();
	for (int i = 0; i < mLevelLayout.size(); i++) {
		delete(mLevelLayout[i]);
	}
	mLevelLayout.clear();
	for (int i = 0; i < mUpdatableObjects.size(); i++) {
		delete(mUpdatableObjects[i]);
	}
	mUpdatableObjects.clear();

	delete mTempPlayer;
	delete mUi;

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

void LevelManager::ClearLevel() {
	mRenderer->ClearLights();
	mWorld->ClearAndErase();
	mPhysics->Clear();
	mLevelMatrices.clear();
	mUpdatableObjects.clear();
	mLevelLayout.clear();
	mRenderer->SetWallFloorObject(nullptr);
}

LevelManager* LevelManager::GetLevelManager() {
	if (instance == nullptr) {
		instance = new LevelManager();
	}
	return instance;
}

void LevelManager::ResetLevel() {
	if (mActiveLevel > -1) {
		mTempPlayer->GetTransform().SetPosition((*mLevelList[mActiveLevel]).GetPlayerStartTransform(0).GetPosition())
			.SetOrientation((*mLevelList[mActiveLevel]).GetPlayerStartTransform(0).GetOrientation());
		mWorld->GetMainCamera().SetYaw((*mLevelList[mActiveLevel]).GetPlayerStartTransform(0).GetOrientation().ToEuler().y);
	}
}

void LevelManager::LoadLevel(int levelID, int playerID, bool isMultiplayer) {
	if (levelID > mLevelList.size() - 1) return;
	mActiveLevel = levelID;
	mWorld->ClearAndErase();
	mPhysics->Clear();
	ClearLevel();
	std::vector<Vector3> itemPositions;
	LoadMap((*mLevelList[levelID]).GetTileMap(), Vector3(0, 0, 0));
	LoadVents((*mLevelList[levelID]).GetVents(), (*mLevelList[levelID]).GetVentConnections());
	LoadDoors((*mLevelList[levelID]).GetDoors(), Vector3(0, 0, 0));
	LoadLights((*mLevelList[levelID]).GetLights(), Vector3(0, 0, 0));
	mHelipad = AddHelipadToWorld((*mLevelList[levelID]).GetHelipadPosition());
	AddPrisonDoorToWorld((*mLevelList[levelID]).GetPrisonDoor());
	for (Vector3 itemPos : (*mLevelList[levelID]).GetItemPositions()) {
		itemPositions.push_back(itemPos);
	}

	for (auto const& [key, val] : (*mLevelList[levelID]).GetRooms()) {
		switch ((*val).GetType()) {
		case Medium:
			for (Room* room : mRoomList) {
				if (room->GetType() == Medium) {
					LoadMap(room->GetTileMap(), key);
					LoadLights(room->GetLights(), key);
					LoadDoors(room->GetDoors(), key);
					for (int i = 0; i < room->GetItemPositions().size(); i++) {
						itemPositions.push_back(room->GetItemPositions()[i] + key);
					}
					break;
				}
			}
			break;
		}
	}
	float* levelSize = new float[3];
	levelSize = mBuilder->BuildNavMesh(mLevelLayout);
	if(levelSize) mPhysics->SetNewBroadphaseSize(Vector3(levelSize[x], levelSize[y], levelSize[z]));

	if (!isMultiplayer){
		AddPlayerToWorld((*mLevelList[levelID]).GetPlayerStartTransform(playerID), "Player");

		//TODO(erendgrmnc): after implementing ai to multiplayer move out from this if block
		LoadGuards((*mLevelList[levelID]).GetGuardCount());
	}
	SendWallFloorInstancesToGPU();
	LoadItems(itemPositions);	
	delete[] levelSize;

	mTimer = 20.f;
}

void LevelManager::SendWallFloorInstancesToGPU() {
	OGLMesh* instance = (OGLMesh*)mWallFloorCubeMesh;
	instance->SetInstanceMatrices(mLevelMatrices);
	if (!mLevelLayout.empty()) {
		mRenderer->SetWallFloorObject(mLevelLayout[0]);
	}
}

void LevelManager::Update(float dt, bool isPlayingLevel, bool isPaused) {
	if (isPlayingLevel) {
		if ((mUpdatableObjects.size() > 0)) {
			for (GameObject* obj : mUpdatableObjects) {
				obj->UpdateObject(dt);
			}
		}
		Debug::Print("TIME LEFT: " + to_string(int(mTimer)), Vector2(0, 3));
		if (mTempPlayer)
			Debug::Print("POINTS: " + to_string(int(mTempPlayer->GetPoints())), Vector2(0, 6));
		mTimer -= dt;
	}

	if (isPaused)
		mRenderer->Render();
	else {
		mWorld->UpdateWorld(dt);
		mRenderer->Update(dt);
		mPhysics->Update(dt);
		mAnimation->Update(dt);
		mRenderer->Render();
		Debug::UpdateRenderables(dt);
	}
}

void LevelManager::InitialiseAssets() {
	mCubeMesh = mRenderer->LoadMesh("cube.msh");
	mWallFloorCubeMesh = mRenderer->LoadMesh("cube.msh");
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
}

void LevelManager::LoadMap(const std::map<Vector3, TileType>& tileMap, const Vector3& startPosition) {
	for (auto const& [key, val] : tileMap) {
		switch (val) {
		case Wall:
			AddWallToWorld(key + startPosition);
			break;
		case Floor:
			AddFloorToWorld(key + startPosition);
			break;
		}
	}
	
}

void LevelManager::LoadLights(const std::vector<Light*>& lights, const Vector3& centre) {
	for (int i = 0; i < lights.size(); i++) {
		if (lights[i]->GetType() == Light::Point) {
			auto* pl = dynamic_cast<PointLight*>(lights[i]);
			PointLight* newPL = new PointLight(pl->GetPosition() + centre, pl->GetColour(), pl->GetRadius());
			mRenderer->AddLight(newPL);
		}
		else if (lights[i]->GetType() == Light::Spot) {
			auto* sl = dynamic_cast<SpotLight*>(lights[i]);
			SpotLight* newSL = new SpotLight(sl->GetDirection(), sl->GetPosition() + centre, sl->GetColour(), sl->GetRadius(), sl->GetAngle(), 2);
			mRenderer->AddLight(newSL);
		}
		else {
			auto* dl = dynamic_cast<DirectionLight*>(lights[i]);
			DirectionLight* newDL = new DirectionLight(dl->GetDirection(), dl->GetColour(), dl->GetRadius(), dl->GetCentre());
			mRenderer->AddLight(newDL);
		}
	}
}

void LevelManager::LoadGuards(int guardCount) {
	for (int i = 0; i < guardCount; i++) {
		AddGuardToWorld((*mLevelList[mActiveLevel]).GetGuardPaths()[i], (*mLevelList[mActiveLevel]).GetPrisonPosition(), "Guard")->SetIsSensed(true);

	}
}

void LevelManager::LoadItems(const std::vector<Vector3>& itemPositions) {
	for (int i = 0; i < itemPositions.size(); i++) {
		if (i == itemPositions.size() / 2) {
			AddFlagToWorld(itemPositions[i], mInventoryBuffSystemClassPtr);
		}
		else {
			AddPickupToWorld(itemPositions[i], mInventoryBuffSystemClassPtr);
		}
	}
}

void LevelManager::LoadVents(const std::vector<Vent*>& vents, std::vector<int> ventConnections) {
	std::vector<Vent*> addedVents;
	for (int i = 0; i < vents.size(); i++) {
		addedVents.push_back(AddVentToWorld(vents[i]));
	}
	for (int i = 0; i < addedVents.size(); i++) {
		addedVents[i]->ConnectVent(addedVents[ventConnections[i]]);
	}
}

void LevelManager::LoadDoors(const std::vector<Door*>& doors, const Vector3& centre) {
	for (int i = 0; i < doors.size(); i++) {
		AddDoorToWorld(doors[i], centre);
	}
}

void LevelManager::InitialiseIcons() {
	UI::Icon mInventoryIcon1 = mUi->AddIcon(Vector2(45, 90), 4.5, 8, mInventorySlotTex);

	UI::Icon mInventoryIcon2 = mUi->AddIcon(Vector2(50, 90), 4.5, 8, mInventorySlotTex);

	UI::Icon mHighlightAwardIcon = mUi->AddIcon(Vector2(3, 84), 4.5, 7, mHighlightAwardTex, false);
	UI::Icon mLightOffIcon = mUi->AddIcon(Vector2(8, 84), 4.5, 7, mLightOffTex, false);
	UI::Icon mMakingNoiseIcon = mUi->AddIcon(Vector2(13, 84), 4.5, 7, mMakingNoiseTex, false);
	UI::Icon mSilentRunIcon = mUi->AddIcon(Vector2(18, 84), 4.5, 7, mSilentRunTex, false);
	UI::Icon mSlowDownIcon = mUi->AddIcon(Vector2(3, 92), 4.5, 7, mSlowDownTex, false);
	UI::Icon mStunIcon = mUi->AddIcon(Vector2(8, 92), 4.5, 7, mStunTex, false);
	UI::Icon mSwapPositionIcon = mUi->AddIcon(Vector2(13, 92), 4.5, 7, mSwapPositionTex, false);

	UI::Icon mSuspensionBarIcon = mUi->AddIcon(Vector2(90, 16), 12, 75, mSuspensionBarTex);
	UI::Icon mSuspensionIndicatorIcon = mUi->AddIcon(Vector2(93, 86), 5, 5, mSuspensionIndicatorTex);

	mRenderer->SetUIObject(mUi);
}

GameObject* LevelManager::AddWallToWorld(const Vector3& position) {
	GameObject* wall = new GameObject(StaticObj, "Wall");

	Vector3 wallSize = Vector3(5, 5, 5);
	AABBVolume* volume = new AABBVolume(wallSize);
	wall->SetBoundingVolume((CollisionVolume*)volume);
	wall->GetTransform()
		.SetScale(wallSize * 2)
		.SetPosition(position);

	wall->SetRenderObject(new RenderObject(&wall->GetTransform(), mWallFloorCubeMesh, mFloorAlbedo, mFloorNormal, mBasicShader, 
		std::sqrt(std::pow(wallSize.x, 2) + std::powf(wallSize.z, 2))));
	wall->SetPhysicsObject(new PhysicsObject(&wall->GetTransform(), wall->GetBoundingVolume()));

	wall->GetPhysicsObject()->SetInverseMass(0);
	wall->GetPhysicsObject()->InitCubeInertia();

	wall->GetRenderObject()->SetColour(Vector4(0.2f, 0.2f, 0.2f, 1));

	wall->GetRenderObject()->SetIsInstanced(true);

	mWorld->AddGameObject(wall);

	mLevelLayout.push_back(wall);

	mLevelMatrices.push_back(wall->GetTransform().GetMatrix());

	return wall;
}

GameObject* LevelManager::AddFloorToWorld(const Vector3& position) {
	GameObject* floor = new GameObject(StaticObj, "Floor");

	Vector3 wallSize = Vector3(5, 0.5f, 5);
	AABBVolume* volume = new AABBVolume(wallSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(wallSize * 2)
		.SetPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), mWallFloorCubeMesh, mFloorAlbedo, mFloorNormal, mBasicShader, 
		std::sqrt(std::pow(wallSize.x, 2) + std::powf(wallSize.z, 2))));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume(), 0, 2, 2));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	floor->GetRenderObject()->SetColour(Vector4(0.2f, 0.2f, 0.2f, 1));

	floor->GetRenderObject()->SetIsInstanced(true);

	mWorld->AddGameObject(floor);

	if(position.y < 0) mLevelLayout.push_back(floor);

	mLevelMatrices.push_back(floor->GetTransform().GetMatrix());

	return floor;
}

Helipad* LevelManager::AddHelipadToWorld(const Vector3& position) {
	Helipad* helipad = new Helipad();

	Vector3 wallSize = Vector3(15, 0.5f, 15);
	AABBVolume* volume = new AABBVolume(wallSize);
	helipad->SetBoundingVolume((CollisionVolume*)volume);
	helipad->GetTransform()
		.SetScale(wallSize * 2)
		.SetPosition(position);

	helipad->SetRenderObject(new RenderObject(&helipad->GetTransform(), mCubeMesh, mFloorAlbedo, mFloorNormal, mBasicShader, 
		std::sqrt(std::pow(wallSize.x, 2) + std::powf(wallSize.z, 2))));
	helipad->SetPhysicsObject(new PhysicsObject(&helipad->GetTransform(), helipad->GetBoundingVolume()));

	helipad->GetPhysicsObject()->SetInverseMass(0);
	helipad->GetPhysicsObject()->InitCubeInertia();

	helipad->GetRenderObject()->SetColour(Vector4(0.0f, 0.4f, 0.2f, 1));

	mWorld->AddGameObject(helipad);

	mLevelLayout.push_back(helipad);

	return helipad;
}

Vent* LevelManager::AddVentToWorld(Vent* vent) {
	Vent* newVent = new Vent();

	Vector3 size = Vector3(1.25f, 1.25f, 0.05f);
	OBBVolume* volume = new OBBVolume(size);

	newVent->SetBoundingVolume((CollisionVolume*)volume);

	newVent->GetTransform()
		.SetPosition(vent->GetTransform().GetPosition())
		.SetOrientation(vent->GetTransform().GetOrientation())
		.SetScale(size*2);

	newVent->SetRenderObject(new RenderObject(&newVent->GetTransform(), mCubeMesh, mBasicTex, mFloorNormal, mBasicShader,
		std::sqrt(std::pow(size.x, 2) + std::powf(size.y, 2))));
	newVent->SetPhysicsObject(new PhysicsObject(&newVent->GetTransform(), newVent->GetBoundingVolume(), 1, 1, 5));


	newVent->GetPhysicsObject()->SetInverseMass(0);
	newVent->GetPhysicsObject()->InitCubeInertia();

	newVent->SetCollisionLayer(StaticObj);

	mWorld->AddGameObject(newVent);

	return newVent;
}

Door* LevelManager::AddDoorToWorld(Door* door, const Vector3& offset) {
	Door* newDoor = new Door();
	Vector3 size = Vector3(0.5f, 4.5f, 5);
	OBBVolume* volume = new OBBVolume(size);

	newDoor->SetBoundingVolume((CollisionVolume*)volume);

	newDoor->GetTransform()
		.SetPosition(door->GetTransform().GetPosition() + offset)
		.SetOrientation(door->GetTransform().GetOrientation())
		.SetScale(size * 2);

	newDoor->SetRenderObject(new RenderObject(&newDoor->GetTransform(), mCubeMesh, mBasicTex, mFloorNormal, mBasicShader,
		std::sqrt(std::pow(size.y, 2) + std::powf(size.z, 2))));
	newDoor->SetPhysicsObject(new PhysicsObject(&newDoor->GetTransform(), newDoor->GetBoundingVolume(), 1, 1, 5));


	newDoor->GetPhysicsObject()->SetInverseMass(0);
	newDoor->GetPhysicsObject()->InitCubeInertia();

	newDoor->SetCollisionLayer(NoCollide);

	mWorld->AddGameObject(newDoor);

	return newDoor;
}

PrisonDoor* LevelManager::AddPrisonDoorToWorld(PrisonDoor* door) {
	PrisonDoor* newDoor = new PrisonDoor();

	Vector3 size = Vector3(0.5f, 4.5f, 5);
	OBBVolume* volume = new OBBVolume(size);

	newDoor->SetBoundingVolume((CollisionVolume*)volume);

	newDoor->GetTransform()
		.SetPosition(door->GetTransform().GetPosition())
		.SetOrientation(door->GetTransform().GetOrientation())
		.SetScale(size * 2);

	newDoor->SetRenderObject(new RenderObject(&newDoor->GetTransform(), mCubeMesh, mBasicTex, mFloorNormal, mBasicShader,
		std::sqrt(std::pow(size.y, 2) + std::powf(size.z, 2))));
	newDoor->SetPhysicsObject(new PhysicsObject(&newDoor->GetTransform(), newDoor->GetBoundingVolume(), 1, 1, 5));


	newDoor->GetPhysicsObject()->SetInverseMass(0);
	newDoor->GetPhysicsObject()->InitCubeInertia();

	newDoor->GetRenderObject()->SetColour(Vector4(1.0f, 0, 0, 1));

	newDoor->SetCollisionLayer(NoCollide);

	mWorld->AddGameObject(newDoor);

	return newDoor;
}

FlagGameObject* LevelManager::AddFlagToWorld(const Vector3& position, InventoryBuffSystemClass* inventoryBuffSystemClassPtr) {
	FlagGameObject* flag = new FlagGameObject(inventoryBuffSystemClassPtr);
	flag->SetPoints(40);

	Vector3 size = Vector3(0.75f, 0.75f, 0.75f);
	SphereVolume* volume = new SphereVolume(0.75f);
	flag->SetBoundingVolume((CollisionVolume*)volume);
	flag->GetTransform()
		.SetScale(size * 2)
		.SetPosition(position);

	flag->SetRenderObject(new RenderObject(&flag->GetTransform(), mSphereMesh, mBasicTex, mFloorNormal, mBasicShader, 0.75f));
	flag->SetPhysicsObject(new PhysicsObject(&flag->GetTransform(), flag->GetBoundingVolume()));

	flag->SetCollisionLayer(Collectable);

	flag->GetPhysicsObject()->SetInverseMass(0);
	flag->GetPhysicsObject()->InitSphereInertia(false);

	flag->GetRenderObject()->SetColour(Vector4(1.0f, 1.0f, 1.0f, 1));

	mWorld->AddGameObject(flag);

	return flag;

}

PickupGameObject* LevelManager::AddPickupToWorld(const Vector3& position, InventoryBuffSystemClass* inventoryBuffSystemClassPtr)
{
	PickupGameObject* pickup = new PickupGameObject(inventoryBuffSystemClassPtr);

	Vector3 size = Vector3(0.75f, 0.75f, 0.75f);
	SphereVolume* volume = new SphereVolume(0.75f);
	pickup->SetBoundingVolume((CollisionVolume*)volume);
	pickup->GetTransform()
		.SetScale(size * 2)
		.SetPosition(position);

	pickup->SetRenderObject(new RenderObject(&pickup->GetTransform(), mSphereMesh, mFloorAlbedo, mFloorNormal, mBasicShader, 0.75f));
	pickup->SetPhysicsObject(new PhysicsObject(&pickup->GetTransform(), pickup->GetBoundingVolume()));

	pickup->SetCollisionLayer(Collectable);

	pickup->GetPhysicsObject()->SetInverseMass(0);
	pickup->GetPhysicsObject()->InitSphereInertia(false);

	pickup->GetRenderObject()->SetColour(Vector4(0.0f, 0.4f, 0.2f, 1));

	mWorld->AddGameObject(pickup);

	mUpdatableObjects.push_back(pickup);

	return pickup;
}

PlayerObject* LevelManager::AddPlayerToWorld(const Transform& transform, const std::string& playerName) {
	mTempPlayer = new PlayerObject(mWorld, playerName, mInventoryBuffSystemClassPtr);
	CreatePlayerObjectComponents(*mTempPlayer, transform);
	mWorld->GetMainCamera().SetYaw(transform.GetOrientation().ToEuler().y);

	mWorld->AddGameObject(mTempPlayer);
	mUpdatableObjects.push_back(mTempPlayer);
	mTempPlayer->ToggleActive();
	return mTempPlayer;
}

void LevelManager::CreatePlayerObjectComponents(PlayerObject& playerObject, const Vector3& position) const {
	CapsuleVolume* volume = new CapsuleVolume(1.4f, 1.0f);

	playerObject.SetBoundingVolume((CollisionVolume*)volume);

	playerObject.GetTransform()
		.SetScale(Vector3(PLAYER_MESH_SIZE, PLAYER_MESH_SIZE, PLAYER_MESH_SIZE))
		.SetPosition(position);

	playerObject.SetRenderObject(new RenderObject(&playerObject.GetTransform(), mEnemyMesh, mKeeperAlbedo, mKeeperNormal, mBasicShader, PLAYER_MESH_SIZE));
	playerObject.SetPhysicsObject(new PhysicsObject(&playerObject.GetTransform(), playerObject.GetBoundingVolume(), 1, 1, 5));


	playerObject.GetPhysicsObject()->SetInverseMass(PLAYER_INVERSE_MASS);
	playerObject.GetPhysicsObject()->InitSphereInertia(false);

	playerObject.SetCollisionLayer(Player);
}

void LevelManager::CreatePlayerObjectComponents(PlayerObject& playerObject, const Transform& playerTransform) {
	CapsuleVolume* volume = new CapsuleVolume(1.4f, 1.0f);

	playerObject.SetBoundingVolume((CollisionVolume*)volume);

	playerObject.GetTransform()
		.SetScale(Vector3(PLAYER_MESH_SIZE, PLAYER_MESH_SIZE, PLAYER_MESH_SIZE))
		.SetPosition(playerTransform.GetPosition())
		.SetOrientation(playerTransform.GetOrientation());

	playerObject.SetRenderObject(new RenderObject(&playerObject.GetTransform(), mEnemyMesh, mKeeperAlbedo, mKeeperNormal, mBasicShader, PLAYER_MESH_SIZE));
	playerObject.SetPhysicsObject(new PhysicsObject(&playerObject.GetTransform(), playerObject.GetBoundingVolume(), 1, 1, 5));


	playerObject.GetPhysicsObject()->SetInverseMass(PLAYER_INVERSE_MASS);
	playerObject.GetPhysicsObject()->InitSphereInertia(false);

	playerObject.SetCollisionLayer(Player);
}

GameResults LevelManager::CheckGameWon() {
	if (mTempPlayer && mHelipad) {
		if (mHelipad->GetCollidingWithPlayer()) {
			if (mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->ItemInPlayerInventory(PlayerInventory::flag,0))
				return GameResults(true, mTempPlayer->GetPoints());
		}
	}
	if (mTempPlayer)
		return GameResults(false, mTempPlayer->GetPoints());
	return GameResults(false, 0);
}

bool LevelManager::CheckGameLost() {
	if (mTimer <= 0.f)
		return true;
	return false;
}

void LevelManager::AddUpdateableGameObject(GameObject& object){
	mUpdatableObjects.push_back(&object);
}

GuardObject* LevelManager::AddGuardToWorld(const vector<Vector3> nodes, const Vector3 prisonPosition, const std::string& guardName) {
	GuardObject* guard = new GuardObject(guardName);

	float meshSize = PLAYER_MESH_SIZE;
	float inverseMass = PLAYER_INVERSE_MASS;

	CapsuleVolume* volume = new CapsuleVolume(1.3f, 1.0f);
	guard->SetBoundingVolume((CollisionVolume*)volume);

	int currentNode = 1;
	guard->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(nodes[currentNode]);

	guard->SetRenderObject(new RenderObject(&guard->GetTransform(), mEnemyMesh, mKeeperAlbedo, mKeeperNormal, mBasicShader, meshSize));
	guard->SetPhysicsObject(new PhysicsObject(&guard->GetTransform(), guard->GetBoundingVolume(), 1, 0, 5));

	guard->GetPhysicsObject()->SetInverseMass(PLAYER_INVERSE_MASS);
	guard->GetPhysicsObject()->InitSphereInertia(false);

	guard->SetCollisionLayer(Npc);

	guard->SetPlayer(mTempPlayer);
	guard->SetGameWorld(mWorld);
	guard->SetPrisonPosition(prisonPosition);
	guard->SetPatrolNodes(nodes);
	guard->SetCurrentNode(currentNode);

	mWorld->AddGameObject(guard);
	mUpdatableObjects.push_back(guard);

	return guard;
}

void LevelManager::UpdateInventoryObserver(InventoryEvent invEvent, int playerNo) {
	switch (invEvent)
	{
	case soundEmitterUsed:
		AddSoundEmitterToWorld(mTempPlayer->GetTransform().GetPosition(),
			mSuspicionSystemClassPtr->GetLocationBasedSuspicion());
		break;
	default:
		break;
	}
}

SoundEmitter* LevelManager::AddSoundEmitterToWorld(const Vector3& position, LocationBasedSuspicion* locationBasedSuspicionPTR)
{
	SoundEmitter* soundEmitterObjectPtr = new SoundEmitter(5, locationBasedSuspicionPTR);

	Vector3 size = Vector3(0.75f, 0.75f, 0.75f);
	SphereVolume* volume = new SphereVolume(0.75f);
	soundEmitterObjectPtr->SetBoundingVolume((CollisionVolume*)volume);
	soundEmitterObjectPtr->GetTransform()
		.SetScale(size * 2)
		.SetPosition(position);

	soundEmitterObjectPtr->SetRenderObject(new RenderObject(&soundEmitterObjectPtr->GetTransform(), mSphereMesh, mBasicTex, mFloorNormal, mBasicShader, 0.75f));
	soundEmitterObjectPtr->SetPhysicsObject(new PhysicsObject(&soundEmitterObjectPtr->GetTransform(), soundEmitterObjectPtr->GetBoundingVolume()));

	soundEmitterObjectPtr->SetCollisionLayer(Collectable);

	soundEmitterObjectPtr->GetPhysicsObject()->SetInverseMass(0);
	soundEmitterObjectPtr->GetPhysicsObject()->InitSphereInertia(false);

	soundEmitterObjectPtr->GetRenderObject()->SetColour(Vector4(1.0f, 1.0f, 1.0f, 1));

	mWorld->AddGameObject(soundEmitterObjectPtr);

	return soundEmitterObjectPtr;
}
