#include "LevelManager.h"

#include "GameWorld.h"
#include "RecastBuilder.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "AnimationObject.h"
#include "SoundObject.h"

#include "PlayerObject.h"
#include "GuardObject.h"
#include "Helipad.h"
#include "CCTV.h"
#include "Vent.h"
#include "InteractableDoor.h"
#include "PrisonDoor.h"

#include "InventoryBuffSystem/FlagGameObject.h"
#include "InventoryBuffSystem/PickupGameObject.h"
#include "InventoryBuffSystem/InventoryBuffSystem.h"
#include "InventoryBuffSystem/SoundEmitter.h"
#include "PointGameObject.h"
#include "DebugNetworkedGame.h"
#include "SceneManager.h"
#include "UISystem.h"

#include <fmod.hpp>

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
	mUi = new UISystem();
	mInventoryBuffSystemClassPtr = new InventoryBuffSystemClass();
	mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->Attach(this);
	mSuspicionSystemClassPtr = new SuspicionSystemClass(mInventoryBuffSystemClassPtr);
	mDtSinceLastFixedUpdate = 0;

	mSoundManager = new SoundManager(mWorld);

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

	mGameState = MenuState;
	
	
	InitialiseAssets();
	InitialiseIcons();

	mItemTextureMap = {
	{PlayerInventory::item::none, mInventorySlotTex},
	{PlayerInventory::item::disguise, mStunTex},
	{PlayerInventory::item::soundEmitter,  mStunTex},
	{PlayerInventory::item::doorKey,  mKeyIconTex3},
	{PlayerInventory::item::flag , mFlagIconTex},
    {PlayerInventory::item::stunItem, mStunTex},
    {PlayerInventory::item::screwdriver, mStunTex}
	};
}

LevelManager::~LevelManager() {
	mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->Detach(this);

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

	delete mSilentRunTex;
	delete mSlowDownTex;
	delete mStunTex;
	delete mSpeedUpTex;

	delete mLowSuspisionBarTex;
	delete mMidSuspisionBarTex;
	delete mHighSuspisionBarTex;
	delete mSuspisionIndicatorTex;

	delete mAnimationShader;

	delete mGuardMaterial;
	delete mGuardMesh;

	delete mPlayerMaterial;
	delete mPlayerMesh;

	delete mRigMaterial;
	delete mRigMesh;

	delete mCCTVMesh;

	delete mGuardAnimationStand;
	delete mGuardAnimationSprint;
	delete mGuardAnimationWalk;

	
	delete mPlayerAnimationStand;
	delete mPlayerAnimationSprint;
	delete mPlayerAnimationWalk;
	
	delete mRigAnimationStand;
	delete mRigAnimationSprint;
	delete mRigAnimationWalk;

	delete mInventoryBuffSystemClassPtr;
	delete mSuspicionSystemClassPtr;

	delete mSoundManager;
}

void LevelManager::ClearLevel() {
	mRenderer->ClearLights();
	mWorld->ClearAndErase();
	mPhysics->Clear();
	mLevelFloorMatrices.clear();
	mLevelWallMatrices.clear();
	mLevelCornerWallMatrices.clear();
	mUpdatableObjects.clear();
	mLevelLayout.clear();
	mRenderer->ClearInstanceObjects();
	mAnimation->Clear();
	mInventoryBuffSystemClassPtr->Reset();
	mSuspicionSystemClassPtr->Reset(mInventoryBuffSystemClassPtr);
	if(mTempPlayer)mTempPlayer->ResetPlayerPoints();
	mBaseFloor = nullptr;
	mBaseWall = nullptr;
	mBaseCornerWall = nullptr;
	ResetEquippedIconTexture();
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
	ClearLevel();
	std::vector<Vector3> itemPositions;
	std::vector<Vector3> roomItemPositions;
	LoadMap((*mLevelList[levelID]).GetTileMap(), Vector3(0, 0, 0));
	LoadVents((*mLevelList[levelID]).GetVents(), (*mLevelList[levelID]).GetVentConnections());
	LoadDoors((*mLevelList[levelID]).GetDoors(), Vector3(0, 0, 0));
	LoadLights((*mLevelList[levelID]).GetLights(), Vector3(0, 0, 0));
	LoadCCTVs((*mLevelList[levelID]).GetCCTVTransforms(), Vector3(0, 0, 0));
	mHelipad = AddHelipadToWorld((*mLevelList[levelID]).GetHelipadPosition());
	PrisonDoor* prisonDoorPtr = AddPrisonDoorToWorld((*mLevelList[levelID]).GetPrisonDoor());
	mUpdatableObjects.push_back(prisonDoorPtr);

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
					LoadCCTVs(room->GetCCTVTransforms(), key);
					for (int i = 0; i < room->GetItemPositions().size(); i++) {
						roomItemPositions.push_back(room->GetItemPositions()[i] + key);
					}
					break;
				}
			}
			break;
		}
	}
	float* levelSize = new float[3];
	levelSize = mBuilder->BuildNavMesh(mLevelLayout);
	if (levelSize) mPhysics->SetNewBroadphaseSize(Vector3(levelSize[x], levelSize[y], levelSize[z]));
	LoadDoorsInNavGrid();

	if (!isMultiplayer) {
		AddPlayerToWorld((*mLevelList[levelID]).GetPlayerStartTransform(playerID), "Player", prisonDoorPtr);
		mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->Attach(mTempPlayer);
		mInventoryBuffSystemClassPtr->GetPlayerBuffsPtr()->Attach(mTempPlayer);

		//TODO(erendgrmnc): after implementing ai to multiplayer move out from this if block
		LoadGuards((*mLevelList[levelID]).GetGuardCount());
	}
	else {
		if (!serverPlayersPtr){
			DebugNetworkedGame* game = reinterpret_cast<DebugNetworkedGame*>(SceneManager::GetSceneManager()->GetCurrentScene());
			serverPlayersPtr = game->GetServerPlayersPtr();
		}

		for (const auto& pair : *serverPlayersPtr)
		{
			PlayerInventoryObserver* invObserver = reinterpret_cast<PlayerInventoryObserver*>(pair.second);
			mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->Attach(invObserver);
			PlayerBuffsObserver* buffsObserver = reinterpret_cast<PlayerBuffsObserver*>(pair.second);
			mInventoryBuffSystemClassPtr->GetPlayerBuffsPtr()->Attach(buffsObserver);
		}
	}
  
	LoadItems(itemPositions, roomItemPositions, isMultiplayer);
	SendWallFloorInstancesToGPU();


	mAnimation->SetGameObjectLists(mUpdatableObjects,mPlayerTextures,mGuardTextures);

	delete[] levelSize;

	mTimer = INIT_TIMER_VALUE;

	//Temp fix for crash problem
	mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->Attach(this);
	mInventoryBuffSystemClassPtr->GetPlayerBuffsPtr()->Attach(mMainFlag);
	mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->Attach(mMainFlag);
	mInventoryBuffSystemClassPtr->GetPlayerBuffsPtr()->Attach(mSuspicionSystemClassPtr->GetLocalSuspicionMetre());
}

void LevelManager::SendWallFloorInstancesToGPU() {
	OGLMesh* floorInstance = (OGLMesh*)mFloorCubeMesh;
	floorInstance->SetInstanceMatrices(mLevelFloorMatrices);
	OGLMesh* wallInstance = (OGLMesh*)mStraightWallMesh;
	wallInstance->SetInstanceMatrices(mLevelWallMatrices);
	OGLMesh* cornerWallInstance = (OGLMesh*)mCornerWallMesh;
	cornerWallInstance->SetInstanceMatrices(mLevelCornerWallMatrices);
	mRenderer->SetInstanceObjects(mBaseFloor, mBaseWall, mBaseCornerWall);
}

void LevelManager::Update(float dt, bool isPlayingLevel, bool isPaused) {
	if (isPlayingLevel) {
		mGameState = LevelState;
		if ((mUpdatableObjects.size() > 0)) {
			for (GameObject* obj : mUpdatableObjects) {
				obj->UpdateObject(dt);
			}
		}
		if (mTempPlayer)
			Debug::Print("POINTS: " + to_string(int(mTempPlayer->GetPoints())), Vector2(0, 6));

		Debug::Print("TIME LEFT: " + to_string(int(mTimer)), Vector2(0, 3));
		mTimer -= dt;
	}
	else
		mGameState = MenuState;


	if (isPaused) {
		mRenderer->Render();
		mGameState = PauseState;
	}
	else {
		mWorld->UpdateWorld(dt);
		mRenderer->Update(dt);
		mPhysics->Update(dt);
		mAnimation->Update(dt, mUpdatableObjects, mPreAnimationList);
		if (mUpdatableObjects.size()>0) {
			mSoundManager->UpdateSounds(mUpdatableObjects);
		}
		mRenderer->Render();
		Debug::UpdateRenderables(dt);
		mDtSinceLastFixedUpdate += dt;
		if (mDtSinceLastFixedUpdate >= TIME_UNTIL_FIXED_UPDATE) {
			FixedUpdate(mDtSinceLastFixedUpdate);
			mDtSinceLastFixedUpdate = 0;
		}
	}
}

void LevelManager::FixedUpdate(float dt){
	mInventoryBuffSystemClassPtr->Update(dt);
	mSuspicionSystemClassPtr->Update(dt);
}

void LevelManager::InitialiseAssets() {
	mCubeMesh = mRenderer->LoadMesh("cube.msh");
	mFloorCubeMesh = mRenderer->LoadMesh("cube.msh");
	mSphereMesh = mRenderer->LoadMesh("sphere.msh");
	mCapsuleMesh = mRenderer->LoadMesh("Capsule.msh");
	mCharMesh = mRenderer->LoadMesh("goat.msh");
	mEnemyMesh = mRenderer->LoadMesh("Keeper.msh");
	mBonusMesh = mRenderer->LoadMesh("apple.msh");
	mCapsuleMesh = mRenderer->LoadMesh("capsule.msh");
	mStraightWallMesh = mRenderer->LoadMesh("Walls/StraightWallCoridoor.msh");
	mCornerWallMesh = mRenderer->LoadMesh("Walls/CornerWallCoridoor.msh");
	mCCTVMesh = mRenderer->LoadMesh("Security_Camera.msh");

	mBasicTex = mRenderer->LoadTexture("checkerboard.png");
	mKeeperAlbedo = mRenderer->LoadTexture("fleshy_albedo.png");
	mKeeperNormal = mRenderer->LoadTexture("fleshy_normal.png");
	mFloorAlbedo = mRenderer->LoadTexture("panel_albedo.png");
	mFloorNormal = mRenderer->LoadTexture("panel_normal.png");
	mWallTex = mRenderer->LoadTexture("corridor_wall_c.tga");
	mWallNormal = mRenderer->LoadTexture("corridor_wall_n.tga");

	mBasicShader = mRenderer->LoadShader("scene.vert", "scene.frag");
	mAnimationShader = mRenderer->LoadShader("animationScene.vert", "scene.frag");



	mGuardMesh = mRenderer->LoadMesh("MaleGuard/Male_Guard.msh");	
	mGuardMaterial = mRenderer->LoadMaterial("MaleGuard/Male_Guard.mat");

	mPlayerMesh = mRenderer->LoadMesh("FemaleGuard/Female_Guard.msh");
	mPlayerMaterial = mRenderer->LoadMaterial("FemaleGuard/Female_Guard.mat");
	
	mRigMesh = mRenderer->LoadMesh("Max/Rig_Maximilian.msh");
	mRigMaterial = mRenderer->LoadMaterial("Max/Rig_Maximilian.mat");
	//Animations
	mGuardAnimationStand = mRenderer->LoadAnimation("MaleGuard/Idle1.anm");
	mGuardAnimationWalk = mRenderer->LoadAnimation("MaleGuard/StepForwardOneHand.anm");
	mGuardAnimationSprint = mRenderer->LoadAnimation("MaleGuard/StepForward.anm");

	mPlayerAnimationStand = mRenderer->LoadAnimation("FemaleGuard/Idle1.anm");
	mPlayerAnimationWalk = mRenderer->LoadAnimation("FemaleGuard/StepForwardOneHand.anm");
	mPlayerAnimationSprint = mRenderer->LoadAnimation("FemaleGuard/StepForward.anm");

	mRigAnimationStand = mRenderer->LoadAnimation("Max/Idle.anm");
	mRigAnimationWalk = mRenderer->LoadAnimation("Max/Walk2.anm");
	mRigAnimationSprint = mRenderer->LoadAnimation("Max/Incentivise.anm");
	//preLoadtexID   I used Guard mesh to player and used rigMesh to guard   @(0v0)@  Chris 12/02/1998

	mAnimation->PreloadMatTextures(*mRenderer, *mGuardMesh,*mGuardMaterial, mPlayerTextures);
	mAnimation->PreloadMatTextures(*mRenderer, *mRigMesh, *mRigMaterial, mGuardTextures);

	//preLoadList
	mPreAnimationList.insert(std::make_pair("GuardStand", mRigAnimationStand));
	mPreAnimationList.insert(std::make_pair("GuardWalk", mRigAnimationWalk));
	mPreAnimationList.insert(std::make_pair("GuardSprint", mRigAnimationSprint));
	

	mPreAnimationList.insert(std::make_pair("PlayerStand", mGuardAnimationStand));
	mPreAnimationList.insert(std::make_pair("PlayerWalk", mGuardAnimationWalk));
	mPreAnimationList.insert(std::make_pair("PlayerSprint", mGuardAnimationSprint));

	//icons
	mInventorySlotTex = mRenderer->LoadTexture("InventorySlot.png");
	
	mSpeedUpTex = mRenderer->LoadTexture("SpeedUp.png");
	mSilentRunTex = mRenderer->LoadTexture("Silence.png");
	mSlowDownTex = mRenderer->LoadTexture("SpeedDown.png");
	mStunTex = mRenderer->LoadTexture("Stun.png");

	mCrossTex = mRenderer->LoadTexture("Cross.png");

	mKeyIconTex3 = mRenderer->LoadTexture("key3.png");
	mKeyIconTex2 = mRenderer->LoadTexture("key2.png");
	mKeyIconTex1 = mRenderer->LoadTexture("key1.png");

	mFlagIconTex = mRenderer->LoadTexture("flag.png");

	mLowSuspisionBarTex = mRenderer->LoadTexture("lowSus.png");
	mMidSuspisionBarTex = mRenderer->LoadTexture("midSus.png");
	mHighSuspisionBarTex = mRenderer->LoadTexture("highSus.png");
	mSuspisionIndicatorTex = mRenderer->LoadTexture("SuspensionIndicator.png");
}

void LevelManager::LoadMap(const std::unordered_map<Transform, TileType>& tileMap, const Vector3& startPosition) {
	for (auto const& [key, val] : tileMap) {
		Transform offsetKey = Transform();
		offsetKey.SetPosition(key.GetPosition() + startPosition).SetOrientation(key.GetOrientation());
		switch (val) {
		case Wall:
			AddWallToWorld(offsetKey);
			break;
		case Floor:
			AddFloorToWorld(offsetKey);
			break;
		case CornerWall:
			AddCornerWallToWorld(offsetKey);
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
		auto* addedGuard = AddGuardToWorld((*mLevelList[mActiveLevel]).GetGuardPaths()[i], (*mLevelList[mActiveLevel]).GetPrisonPosition(), "Guard");
		addedGuard->SetIsSensed(false);
		mGuardObjects.push_back(addedGuard);
	}
}

void LevelManager::LoadItems(const std::vector<Vector3>& itemPositions, const std::vector<Vector3>& roomItemPositions, const bool& isMultiplayer) {
	for (int i = 0; i < itemPositions.size(); i++) {
		AddPickupToWorld(itemPositions[i], mInventoryBuffSystemClassPtr, isMultiplayer);
	}
	std::random_device rd;
	std::mt19937 gen(rd());

	std::uniform_int_distribution<> dis(0, roomItemPositions.size()-1);
	int flagItem = dis(gen);
	for (int i = 0; i < roomItemPositions.size(); i++) {
		if (i == flagItem) {
			mMainFlag = AddFlagToWorld(roomItemPositions[i], mInventoryBuffSystemClassPtr,mSuspicionSystemClassPtr);
			continue;
		}
		AddPickupToWorld(roomItemPositions[i], mInventoryBuffSystemClassPtr, isMultiplayer);
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
		InteractableDoor* interactableDoorPtr =AddDoorToWorld(doors[i], centre);
		mUpdatableObjects.push_back(interactableDoorPtr);
	}
}

void LevelManager::LoadCCTVs(const std::vector<Transform>& transforms, const Vector3& startPosition) {
	for (int i = 0; i < transforms.size(); i++) {
		Transform offsetTransform = Transform();
		offsetTransform.SetPosition(transforms[i].GetPosition() + startPosition).SetOrientation(transforms[i].GetOrientation());
		AddCCTVToWorld(offsetTransform);
	}
}

void LevelManager::LoadDoorsInNavGrid() {
	for (int i = 0; i < mUpdatableObjects.size(); i++) {
		auto* door = dynamic_cast<InteractableDoor*>(mUpdatableObjects[i]);
		if (door) {
			float* startPos = new float[3] {door->GetTransform().GetPosition().x, door->GetTransform().GetPosition().y, door->GetTransform().GetPosition().z};
			AABBVolume* volume = (AABBVolume*)door->GetBoundingVolume();
			float* halfExt = new float[3] {volume->GetHalfDimensions().x, volume->GetHalfDimensions().y, volume->GetHalfDimensions().z};
			LoadDoorInNavGrid(startPos, halfExt, ClosedDoorFlag);
			delete[] startPos;
			delete[] halfExt;
			continue;
		}
		auto* prisonDoor = dynamic_cast<PrisonDoor*>(mUpdatableObjects[i]);
		if (prisonDoor) {
			float* startPos = new float[3] {prisonDoor->GetTransform().GetPosition().x, prisonDoor->GetTransform().GetPosition().y, prisonDoor->GetTransform().GetPosition().z};
			AABBVolume* volume = (AABBVolume*)prisonDoor->GetBoundingVolume();
			float* halfExt = new float[3] {volume->GetHalfDimensions().x, volume->GetHalfDimensions().y, volume->GetHalfDimensions().z};
			LoadDoorInNavGrid(startPos, halfExt, ClosedDoorFlag);
			delete[] startPos;
			delete[] halfExt;
		}
	}
}

void LevelManager::LoadDoorInNavGrid(float* position, float* halfSize, PolyFlags flag) {
	dtQueryFilter* filter = new dtQueryFilter();
	dtPolyRef* startRef = new dtPolyRef();
	float* nearestPoint = new float[3];
	LevelManager::GetLevelManager()->GetBuilder()->GetNavMeshQuery()->findNearestPoly(position, halfSize, filter, startRef, nearestPoint);
	dtPolyRef* resultRef = new dtPolyRef[20];
	dtPolyRef* resultParent = new dtPolyRef[20];
	int* resultCount = new int();
	mBuilder->GetNavMeshQuery()->findLocalNeighbourhood(*startRef, position, halfSize[y], filter, resultRef, resultParent, resultCount, 20);
	mBuilder->GetNavMesh()->setPolyFlags(*startRef, flag);
	for (int j = 0; j < *resultCount; j++) {
		mBuilder->GetNavMesh()->setPolyFlags(resultRef[j], flag);
	}
	delete[] nearestPoint;
	delete resultCount;
	delete filter;
}

void LevelManager::InitialiseIcons() {
	//Inventory
	UISystem::Icon* mInventoryIcon1 = mUi->AddIcon(Vector2(45, 90), 4.5, 8, mInventorySlotTex);
	mUi->SetEquippedItemIcon(0, *mInventoryIcon1);

	UISystem::Icon* mInventoryIcon2 = mUi->AddIcon(Vector2(50, 90), 4.5, 8, mInventorySlotTex);
	mUi->SetEquippedItemIcon(1, *mInventoryIcon2);

	//Buff
	UISystem::Icon* mSilentSprintIcon = mUi->AddIcon(Vector2(65, 3), 4.5, 7, mSilentRunTex, false);
	mUi->SetEquippedItemIcon(2, *mSilentSprintIcon);

	UISystem::Icon* mSlowIcon = mUi->AddIcon(Vector2(70, 3), 4.5, 7, mSlowDownTex, false);
	mUi->SetEquippedItemIcon(3, *mSlowIcon);

	UISystem::Icon* mStunIcon = mUi->AddIcon(Vector2(75, 3), 4.5, 7, mStunTex, false);
	mUi->SetEquippedItemIcon(4, *mStunIcon);

	UISystem::Icon* mSpeedIcon = mUi->AddIcon(Vector2(80, 3), 4.5, 7, mSpeedUpTex, false);
	mUi->SetEquippedItemIcon(5, *mSpeedIcon);

	//suspicion
	UISystem::Icon* mSuspisionBarIcon = mUi->AddIcon(Vector2(90, 15), 3, 75, mLowSuspisionBarTex);
	mUi->SetEquippedItemIcon(6, *mSuspisionBarIcon);

	UISystem::Icon* mSuspisionIndicatorIcon = mUi->AddIcon(Vector2(90, 86), 3, 3, mSuspisionIndicatorTex);
	mUi->SetEquippedItemIcon(7, *mSuspisionIndicatorIcon);

	UISystem::Icon* mCross = mUi->AddIcon(Vector2(50, 50), 3, 5, mCrossTex);

	mRenderer->SetUIObject(mUi);

	
	
}

GameObject* LevelManager::AddWallToWorld(const Transform& transform) {
	GameObject* wall = new GameObject(StaticObj, "Wall");

	Vector3 wallSize = Vector3(1.5f, 1.5f, 1.5f);
	AABBVolume* volume = new AABBVolume(wallSize + Vector3(0, 3, 0), Vector3(0, 4.5f, 0));
	wall->SetBoundingVolume((CollisionVolume*)volume);
	wall->GetTransform()
		.SetScale(wallSize * 2)
		.SetPosition(transform.GetPosition())
		.SetOrientation(transform.GetOrientation());

	wall->SetRenderObject(new RenderObject(&wall->GetTransform(), mStraightWallMesh, mWallTex, mWallNormal, mBasicShader, 
		std::sqrt(std::pow(wallSize.x, 2) + std::powf(wallSize.z, 2))));
	wall->SetPhysicsObject(new PhysicsObject(&wall->GetTransform(), wall->GetBoundingVolume()));

	wall->GetPhysicsObject()->SetInverseMass(0);
	wall->GetPhysicsObject()->InitCubeInertia();

	wall->GetRenderObject()->SetColour(Vector4(0.2f, 0.2f, 0.2f, 1));

	wall->GetRenderObject()->SetIsInstanced(true);

	mWorld->AddGameObject(wall);

	mLevelLayout.push_back(wall);

	mLevelWallMatrices.push_back(wall->GetTransform().GetMatrix());

	if (!mBaseWall) mBaseWall = wall;

	return wall;
}

GameObject* LevelManager::AddCornerWallToWorld(const Transform& transform) {
	GameObject* wall = new GameObject(StaticObj, "Wall");

	Vector3 wallSize = Vector3(1.5f, 1.5f, 1.5f);
	Vector3 offset = Matrix4::Rotation(transform.GetOrientation().ToEuler().y, Vector3(0, 1, 0)) * Vector3(1.5f, 4.5f, 1.5f);
	AABBVolume* volume = new AABBVolume(wallSize + Vector3(1.5f, 3, 1.5f), offset);
	wall->SetBoundingVolume((CollisionVolume*)volume);
	wall->GetTransform()
		.SetScale(wallSize * 2)
		.SetPosition(transform.GetPosition())
		.SetOrientation(transform.GetOrientation());

	wall->SetRenderObject(new RenderObject(&wall->GetTransform(), mCornerWallMesh, mWallTex, mWallNormal, mBasicShader,
		std::sqrt(std::pow(wallSize.x, 2) + std::powf(wallSize.z, 2))));
	wall->SetPhysicsObject(new PhysicsObject(&wall->GetTransform(), wall->GetBoundingVolume()));

	wall->GetPhysicsObject()->SetInverseMass(0);
	wall->GetPhysicsObject()->InitCubeInertia();

	wall->GetRenderObject()->SetColour(Vector4(0.2f, 0.2f, 0.2f, 1));

	wall->GetRenderObject()->SetIsInstanced(true);

	mWorld->AddGameObject(wall);

	mLevelLayout.push_back(wall);

	mLevelCornerWallMatrices.push_back(wall->GetTransform().GetMatrix());

	if (!mBaseCornerWall) mBaseCornerWall = wall;

	return wall;
}

GameObject* LevelManager::AddFloorToWorld(const Transform& transform) {
	GameObject* floor = new GameObject(StaticObj, "Floor");

	Vector3 floorSize = Vector3(4.5f, 0.5f, 4.5f);
	AABBVolume* volume = new AABBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(floorSize * 2)
		.SetPosition(transform.GetPosition())
		.SetOrientation(transform.GetOrientation());

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), mFloorCubeMesh, mFloorAlbedo, mFloorNormal, mBasicShader, 
		std::sqrt(std::pow(floorSize.x, 2) + std::powf(floorSize.z, 2))));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume(), 0, 2, 2));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	floor->GetRenderObject()->SetColour(Vector4(0.2f, 0.2f, 0.2f, 1));

	floor->GetRenderObject()->SetIsInstanced(true);

	mWorld->AddGameObject(floor);

	if(transform.GetPosition().y < 0) mLevelLayout.push_back(floor);

	mLevelFloorMatrices.push_back(floor->GetTransform().GetMatrix());

	if (!mBaseFloor) mBaseFloor = floor;

	return floor;
}

CCTV* LevelManager::AddCCTVToWorld(const Transform& transform) {
	CCTV* camera = new CCTV();

	Vector3 wallSize = Vector3(1, 1, 1);
	camera->GetTransform()
		.SetScale(wallSize * 2)
		.SetPosition(transform.GetPosition())
		.SetOrientation(transform.GetOrientation());

	camera->SetRenderObject(new RenderObject(&camera->GetTransform(), mCCTVMesh, mFloorAlbedo, mFloorNormal, mBasicShader,
		std::sqrt(std::pow(wallSize.x, 2) + std::powf(wallSize.z, 2))));
	camera->SetPhysicsObject(new PhysicsObject(&camera->GetTransform(), camera->GetBoundingVolume()));

	camera->GetPhysicsObject()->SetInverseMass(0);
	camera->GetPhysicsObject()->InitCubeInertia();

	camera->GetRenderObject()->SetColour(Vector4(0.2f, 0.2f, 0.2f, 1));

	mWorld->AddGameObject(camera);

	return camera;
}

Helipad* LevelManager::AddHelipadToWorld(const Vector3& position) {
	Helipad* helipad = new Helipad();

	Vector3 wallSize = Vector3(13.5f, 0.5f, 13.5f);
	AABBVolume* volume = new AABBVolume(wallSize);
	helipad->SetBoundingVolume((CollisionVolume*)volume);
	helipad->GetTransform()
		.SetScale(wallSize * 2)
		.SetPosition(position);

	helipad->SetRenderObject(new RenderObject(&helipad->GetTransform(), mCubeMesh, mBasicTex, mFloorNormal, mBasicShader, 
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

InteractableDoor* LevelManager::AddDoorToWorld(Door* door, const Vector3& offset) {
	InteractableDoor* newDoor = new InteractableDoor();
	Vector3 size = Vector3(0.5f, 4.5f, 4.5f);
	if (abs(door->GetTransform().GetOrientation().y) == 1 || abs(door->GetTransform().GetOrientation().w) == 1) {
		AABBVolume* volume = new AABBVolume(size);
		newDoor->SetBoundingVolume((CollisionVolume*)volume);
	}
	else {
		AABBVolume* volume = new AABBVolume(Vector3(4.5f, 4.5f, 0.5f));
		newDoor->SetBoundingVolume((CollisionVolume*)volume);
	}

	newDoor->GetTransform()
		.SetPosition(door->GetTransform().GetPosition() + offset)
		.SetOrientation(door->GetTransform().GetOrientation())
		.SetScale(size * 2);

	newDoor->SetRenderObject(new RenderObject(&newDoor->GetTransform(), mCubeMesh, mBasicTex, mFloorNormal, mBasicShader,
		std::sqrt(std::pow(size.y, 2) + std::powf(size.z, 2))));
	newDoor->SetPhysicsObject(new PhysicsObject(&newDoor->GetTransform(), newDoor->GetBoundingVolume(), 1, 1, 5));
	newDoor->SetSoundObject(new SoundObject(mSoundManager->AddDoorOpenSound()));

	newDoor->GetPhysicsObject()->SetInverseMass(0);
	newDoor->GetPhysicsObject()->InitCubeInertia();

	newDoor->SetCollisionLayer(NoSpecialFeatures);

	mWorld->AddGameObject(newDoor);

	return newDoor;
}

PrisonDoor* LevelManager::AddPrisonDoorToWorld(PrisonDoor* door) {
	PrisonDoor* newDoor = new PrisonDoor();

	Vector3 size = Vector3(0.5f, 4.5f, 4.5f);
	if (abs(door->GetTransform().GetOrientation().y) == 1 || abs(door->GetTransform().GetOrientation().w) == 1) {
		AABBVolume* volume = new AABBVolume(size);
		newDoor->SetBoundingVolume((CollisionVolume*)volume);
	}
	else {
		AABBVolume* volume = new AABBVolume(Vector3(4.5f, 4.5f, 0.5f));
		newDoor->SetBoundingVolume((CollisionVolume*)volume);
	}

	newDoor->GetTransform()
		.SetPosition(door->GetTransform().GetPosition())
		.SetOrientation(door->GetTransform().GetOrientation())
		.SetScale(size * 2);

	newDoor->SetRenderObject(new RenderObject(&newDoor->GetTransform(), mCubeMesh, mBasicTex, mFloorNormal, mBasicShader,
		std::sqrt(std::pow(size.y, 2) + std::powf(size.z, 2))));
	newDoor->SetPhysicsObject(new PhysicsObject(&newDoor->GetTransform(), newDoor->GetBoundingVolume(), 1, 1, 5));
	newDoor->SetSoundObject(new SoundObject(mSoundManager->AddDoorOpenSound()));

	newDoor->GetPhysicsObject()->SetInverseMass(0);
	newDoor->GetPhysicsObject()->InitCubeInertia();

	newDoor->GetRenderObject()->SetColour(Vector4(1.0f, 0, 0, 1));

	newDoor->SetCollisionLayer(NoSpecialFeatures);

	mWorld->AddGameObject(newDoor);

	return newDoor;
}

FlagGameObject* LevelManager::AddFlagToWorld(const Vector3& position, InventoryBuffSystemClass* inventoryBuffSystemClassPtr, SuspicionSystemClass* suspicionSystemClassPtr) {
	FlagGameObject* flag = new FlagGameObject(inventoryBuffSystemClassPtr, suspicionSystemClassPtr);
	
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

PickupGameObject* LevelManager::AddPickupToWorld(const Vector3& position, InventoryBuffSystemClass* inventoryBuffSystemClassPtr,const bool& isMultiplayer)
{
	PickupGameObject* pickup = new PickupGameObject(inventoryBuffSystemClassPtr,isMultiplayer);

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

PointGameObject* LevelManager::AddPointObjectToWorld(const Vector3& position, int pointsWorth, float initCooldown)
{
	PointGameObject* pointObject = new PointGameObject(pointsWorth, initCooldown);

	Vector3 size = Vector3(0.75f, 0.75f, 0.75f);
	SphereVolume* volume = new SphereVolume(0.75f);
	pointObject->SetBoundingVolume((CollisionVolume*)volume);
	pointObject->GetTransform()
		.SetScale(size * 2)
		.SetPosition(position);

	pointObject->SetRenderObject(new RenderObject(&pointObject->GetTransform(), mSphereMesh, mFloorAlbedo, mFloorNormal, mBasicShader, 0.75f));
	pointObject->SetPhysicsObject(new PhysicsObject(&pointObject->GetTransform(), pointObject->GetBoundingVolume()));

	pointObject->SetCollisionLayer(Collectable);

	pointObject->GetPhysicsObject()->SetInverseMass(0);
	pointObject->GetPhysicsObject()->InitSphereInertia(false);

	pointObject->GetRenderObject()->SetColour(Vector4(0.0f, 0.4f, 0.2f, 1));

	mWorld->AddGameObject(pointObject);

	mUpdatableObjects.push_back(pointObject);

	return pointObject;
}

PlayerObject* LevelManager::AddPlayerToWorld(const Transform& transform, const std::string& playerName, PrisonDoor* prisonDoor) {
	mTempPlayer = new PlayerObject(mWorld, mInventoryBuffSystemClassPtr, mSuspicionSystemClassPtr, mUi, playerName, prisonDoor);
	CreatePlayerObjectComponents(*mTempPlayer, transform);
	mWorld->GetMainCamera().SetYaw(transform.GetOrientation().ToEuler().y);

	mWorld->AddGameObject(mTempPlayer);
	mUpdatableObjects.push_back(mTempPlayer);
	mTempPlayer->SetIsRendered(false);
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

	playerObject.SetRenderObject(new RenderObject(&playerObject.GetTransform(), mGuardMesh, mKeeperAlbedo, mKeeperNormal, mAnimationShader, PLAYER_MESH_SIZE));
	playerObject.SetPhysicsObject(new PhysicsObject(&playerObject.GetTransform(), playerObject.GetBoundingVolume(), 1, 1, 5));
	playerObject.SetSoundObject(new SoundObject(mSoundManager->AddWalkSound()));
	playerObject.GetRenderObject()->SetAnimationObject(new AnimationObject(AnimationObject::AnimationType::playerAnimation, mGuardAnimationStand, mGuardMaterial));


	playerObject.GetPhysicsObject()->SetInverseMass(PLAYER_INVERSE_MASS);
	playerObject.GetPhysicsObject()->InitSphereInertia(false);

	playerObject.SetCollisionLayer(Player);
}

void LevelManager::ChangeEquippedIconTexture(int itemSlot, PlayerInventory::item equippedItem) {
	if (mItemTextureMap[equippedItem] == nullptr) {
		std::cout << "Can not find Icon texture" << std::endl;
		return;
	}
	Texture& itemTex = *mItemTextureMap[equippedItem];
	
	mUi->ChangeEquipmentSlotTexture(itemSlot, itemTex);
}

void LevelManager::DropEquippedIconTexture(int itemSlot) {

	Texture& itemTex = *mInventorySlotTex;

	mUi->ChangeEquipmentSlotTexture(itemSlot, itemTex);
}

void LevelManager::ResetEquippedIconTexture() {

	Texture& itemTex = *mInventorySlotTex;

	mUi->ChangeEquipmentSlotTexture(0, itemTex);
	mUi->ChangeEquipmentSlotTexture(1, itemTex);

	mUi->ChangeBuffSlotTransparency(2, false);
	mUi->ChangeBuffSlotTransparency(3, false);
	mUi->ChangeBuffSlotTransparency(4, false);
	mUi->ChangeBuffSlotTransparency(5, false);


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

std::vector<GuardObject*>& LevelManager::GetGuardObjects() {
	return mGuardObjects;
}

void LevelManager::AddBuffToGuards(PlayerBuffs::buff buffToApply) {
	for(const auto& guard : mGuardObjects) {
		guard->ApplyBuffToGuard(buffToApply);
	}
}

void LevelManager::AddUpdateableGameObject(GameObject& object){
	mUpdatableObjects.push_back(&object);
}

GuardObject* LevelManager::AddGuardToWorld(const vector<Vector3> nodes, const Vector3 prisonPosition, const std::string& guardName) {
	GuardObject* guard = new GuardObject(guardName);

	float meshSize = PLAYER_MESH_SIZE;
	float inverseMass = PLAYER_INVERSE_MASS;

	CapsuleVolume* volume = new CapsuleVolume(1.3f, 1.0f, Vector3(0, 2.0f, 0));
	guard->SetBoundingVolume((CollisionVolume*)volume);

	int currentNode = 1;
	guard->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(nodes[currentNode] + Vector3(20,-1.5f,20));

	guard->SetRenderObject(new RenderObject(&guard->GetTransform(), mRigMesh, mKeeperAlbedo, mKeeperNormal, mAnimationShader, meshSize));
	guard->SetPhysicsObject(new PhysicsObject(&guard->GetTransform(), guard->GetBoundingVolume(), 1, 0, 5));
	guard->SetSoundObject(new SoundObject(mSoundManager->AddWalkSound()));
	guard->GetRenderObject()->SetAnimationObject(new AnimationObject(AnimationObject::AnimationType::guardAnimation, mRigAnimationStand, mRigMaterial));


	guard->GetPhysicsObject()->SetInverseMass(PLAYER_INVERSE_MASS);
	guard->GetPhysicsObject()->InitSphereInertia(false);



	guard->SetCollisionLayer(Npc);

	guard->SetPlayer(mTempPlayer);
	guard->SetPatrolNodes(nodes);
	guard->SetCurrentNode(currentNode);

	mWorld->AddGameObject(guard);
	mUpdatableObjects.push_back(guard);

	return guard;
}

InventoryBuffSystemClass* NCL::CSC8503::LevelManager::GetInventoryBuffSystem() {
	return mInventoryBuffSystemClassPtr;
}

SuspicionSystemClass* NCL::CSC8503::LevelManager::GetSuspicionSystem() {
	return mSuspicionSystemClassPtr;
}

void LevelManager::UpdateInventoryObserver(InventoryEvent invEvent, int playerNo, int invSlot, bool isItemRemoved) {
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
	SoundEmitter* soundEmitterObjectPtr = new SoundEmitter(5, locationBasedSuspicionPTR, position);

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

	mUpdatableObjects.push_back(soundEmitterObjectPtr);

	return soundEmitterObjectPtr;
}

FlagGameObject* LevelManager::GetMainFlag() {
	return mMainFlag;
}
