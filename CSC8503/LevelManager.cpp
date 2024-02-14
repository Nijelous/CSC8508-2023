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
#include "InventoryBuffSystem/FlagGameObject.h"
#include "InventoryBuffSystem/PickupGameObject.h"
#include "InventoryBuffSystem/InventoryBuffSystem.h"
#include "UI.h"

#include <filesystem>

using namespace NCL::CSC8503;

LevelManager::LevelManager() {
	mBuilder = new RecastBuilder();
	mWorld = new GameWorld();
	mRenderer = new GameTechRenderer(*mWorld);
	mPhysics = new PhysicsSystem(*mWorld);
	mPhysics->UseGravity(true);
	mAnimation = new AnimationSystem(*mWorld);

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

void LevelManager::LoadLevel(int levelID, int playerID, bool isMultiplayer) {
	if (levelID > mLevelList.size() - 1) return;
	mActiveLevel = levelID;
	mWorld->ClearAndErase();
	mPhysics->Clear();
	for (int i = 0; i < mLevelLayout.size(); i++) {
		delete(mLevelLayout[i]);
	}
	mLevelLayout.clear();
	std::vector<Vector3> itemPositions;
	LoadMap((*mLevelList[levelID]).GetTileMap(), Vector3(0, 0, 0));
	LoadVents((*mLevelList[levelID]).GetVents());
	LoadLights((*mLevelList[levelID]).GetLights(), Vector3(0, 0, 0));
	mHelipad = AddHelipadToWorld((*mLevelList[levelID]).GetHelipadPosition());
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
					for (int i = 0; i < room->GetItemPositions().size(); i++) {
						itemPositions.push_back(room->GetItemPositions()[i] + key);
					}
					break;
				}
			}
			break;
		}
	}

	float* levelSize = mBuilder->BuildNavMesh(mLevelLayout);
	if(levelSize) mPhysics->SetNewBroadphaseSize(Vector3(levelSize[x], levelSize[y], levelSize[z]));

	if (!isMultiplayer){
		AddPlayerToWorld((*mLevelList[levelID]).GetPlayerStartTransform(playerID), "Player");

		//TODO(erendgrmnc): after implementing ai to multiplayer move out from this if block
		LoadGuards((*mLevelList[levelID]).GetGuardCount());
	}
	
	LoadItems(itemPositions);
}

void LevelManager::Update(float dt, bool isUpdatingObjects) {
	if ((mUpdatableObjects.size() > 0) && isUpdatingObjects) {
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

void LevelManager::InitialiseAssets() {
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
		auto* pl = dynamic_cast<PointLight*>(lights[i]);
		if (pl) {
			pl->SetPosition(pl->GetPosition() + centre);
			pl->SetRadius(pl->GetRadius());
			mRenderer->AddLight(pl);
			continue;
		}
		auto* sl = dynamic_cast<SpotLight*>(lights[i]);
		if (sl) {
			sl->SetPosition(pl->GetPosition() + centre);
			sl->SetRadius(sl->GetRadius());
			mRenderer->AddLight(sl);
			continue;
		}
		mRenderer->AddLight(lights[i]);
	}
}

void LevelManager::LoadGuards(int guardCount) {
	for (int i = 0; i < guardCount; i++) {
		AddGuardToWorld((*mLevelList[mActiveLevel]).GetGuardPaths()[i], (*mLevelList[mActiveLevel]).GetPrisonPosition(), "Guard");
		AddGuardToWorld((*mLevelList[mActiveLevel]).GetGuardPaths()[i][i+1], "Guard")->SetIsSensed(true);

	}
}

void LevelManager::LoadItems(const std::vector<Vector3> itemPositions) {
	for (int i = 0; i < itemPositions.size(); i++) {
		AddFlagToWorld(itemPositions[i],mInventoryBuffSystemClassPtr);
		return;
	}
}

void LevelManager::LoadVents(const std::vector<Vent*> vents) {
	for (int i = 0; i < vents.size(); i++) {
		AddVentToWorld(vents[i]);
	}
}

void LevelManager::InitialiseIcons() {
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

GameObject* LevelManager::AddWallToWorld(const Vector3& position) {
	GameObject* wall = new GameObject(StaticObj, "Wall");

	Vector3 wallSize = Vector3(5, 5, 5);
	AABBVolume* volume = new AABBVolume(wallSize);
	wall->SetBoundingVolume((CollisionVolume*)volume);
	wall->GetTransform()
		.SetScale(wallSize * 2)
		.SetPosition(position);

	wall->SetRenderObject(new RenderObject(&wall->GetTransform(), mCubeMesh, mFloorAlbedo, mFloorNormal, mBasicShader, 
		std::sqrt(std::pow(wallSize.x, 2) + std::powf(wallSize.z, 2))));
	wall->SetPhysicsObject(new PhysicsObject(&wall->GetTransform(), wall->GetBoundingVolume()));

	wall->GetPhysicsObject()->SetInverseMass(0);
	wall->GetPhysicsObject()->InitCubeInertia();

	wall->GetRenderObject()->SetColour(Vector4(0.2f, 0.2f, 0.2f, 1));

	mWorld->AddGameObject(wall);

	mLevelLayout.push_back(wall);

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

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), mCubeMesh, mFloorAlbedo, mFloorNormal, mBasicShader, 
		std::sqrt(std::pow(wallSize.x, 2) + std::powf(wallSize.z, 2))));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume(), 0, 2, 2));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	floor->GetRenderObject()->SetColour(Vector4(0.2f, 0.2f, 0.2f, 1));

	mWorld->AddGameObject(floor);

	if(position.y < 0) mLevelLayout.push_back(floor);

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
	Vector3 size = Vector3(1.25f, 1.25f, 0.05f);
	OBBVolume* volume = new OBBVolume(size);

	vent->SetBoundingVolume((CollisionVolume*)volume);

	vent->GetTransform()
		.SetScale(size*2);

	vent->SetRenderObject(new RenderObject(&vent->GetTransform(), mCubeMesh, mBasicTex, mFloorNormal, mBasicShader,
		std::sqrt(std::pow(size.x, 2) + std::powf(size.y, 2))));
	vent->SetPhysicsObject(new PhysicsObject(&vent->GetTransform(), vent->GetBoundingVolume(), 1, 1, 5));


	vent->GetPhysicsObject()->SetInverseMass(0);
	vent->GetPhysicsObject()->InitCubeInertia();

	vent->SetCollisionLayer(StaticObj);

	mWorld->AddGameObject(vent);

	return vent;
}

FlagGameObject* LevelManager::AddFlagToWorld(const Vector3& position, InventoryBuffSystemClass* inventoryBuffSystemClassPtr) {
	FlagGameObject* flag = new FlagGameObject(inventoryBuffSystemClassPtr);

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

	return pickup;
}

PlayerObject* LevelManager::AddPlayerToWorld(const Transform& transform, const std::string& playerName) {
	mTempPlayer = new PlayerObject(mWorld, playerName);
	CreatePlayerObjectComponents(*mTempPlayer, transform);
	mWorld->GetMainCamera().SetYaw(transform.GetOrientation().ToEuler().y);

	mWorld->AddGameObject(mTempPlayer);
	mUpdatableObjects.push_back(mTempPlayer);
	mTempPlayer->SetActive();
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
bool LevelManager::CheckGameWon() {
	if (mTempPlayer && mHelipad) {
		if (mHelipad->GetCollidingWithPlayer()) {
			if (mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->ItemInPlayerInventory(PlayerInventory::flag,0))
				return true;
		}
	}
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
