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
#include "NetworkObject.h"
#include "UISystem.h"
#include <filesystem>
#include <fstream>

namespace {
	constexpr int NETWORK_ID_BUFFER_START = 10;
}

using namespace NCL::CSC8503;

LevelManager* LevelManager::instance = nullptr;

LevelManager::LevelManager() {
	mRoomList = std::vector<Room*>();
	std::thread loadRooms([this] {
		for (const auto& entry : std::filesystem::directory_iterator("../Assets/Levels/Rooms")) {
			Room* newRoom = new Room(entry.path().string());
			mRoomList.push_back(newRoom);
		}
		});
	mLevelList = std::vector<Level*>();
	std::thread loadLevels([this] {
		for (const auto& entry : std::filesystem::directory_iterator("../Assets/Levels/Levels")) {
			Level* newLevel = new Level(entry.path().string());
			mLevelList.push_back(newLevel);
		}
		});
	mWorld = new GameWorld();
#ifdef USEGL
	std::thread loadSoundManager([this] {mSoundManager = new SoundManager(mWorld); });

	mRenderer = new GameTechRenderer(*mWorld);
#endif

#ifdef USEPROSPERO
	mRenderer = new GameTechAGCRenderer(*mWorld);
#endif
	mUi = new UISystem();
	InitialiseAssets();
#ifdef USEGL // remove after implemented
	mAnimation = new AnimationSystem(*mWorld, mPreAnimationList);

	//preLoadtexID   I used Guard mesh to player and used rigMesh to guard   @(0v0)@  Chris 12/02/1998
	mAnimation->PreloadMatTextures(*mRenderer, *mMeshes["Rig"], *mMaterials["Rig"], mGuardTextures);
	mAnimation->PreloadMatTextures(*mRenderer, *mMeshes["Guard"], *mMaterials["Guard"], mPlayerTextures);
#endif
	mBuilder = new RecastBuilder();
	mPhysics = new PhysicsSystem(*mWorld);
	mPhysics->UseGravity(true);
	mInventoryBuffSystemClassPtr = new InventoryBuffSystemClass();
	mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->Attach(this);
	mSuspicionSystemClassPtr = new SuspicionSystemClass(mInventoryBuffSystemClassPtr);
	mDtSinceLastFixedUpdate = 0;

	mActiveLevel = -1;

	mGameState = MenuState;

	mNetworkIdBuffer = NETWORK_ID_BUFFER_START;
  
	mIsLevelInitialised = false;

	InitialiseIcons();
	mItemTextureMap = {
	{PlayerInventory::item::none, mTextures["InventorySlot"]},
	{PlayerInventory::item::disguise, mTextures["Stun"]},
	{PlayerInventory::item::soundEmitter,  mTextures["Stun"]},
	{PlayerInventory::item::doorKey,  mTextures["KeyIcon3"]},
	{PlayerInventory::item::flag , mTextures["FlagIcon"]},
    {PlayerInventory::item::stunItem, mTextures["Stun"]},
    {PlayerInventory::item::screwdriver, mTextures["Stun"]}
	};
	loadRooms.join();
	loadLevels.join();
#ifdef USEGL
	loadSoundManager.join();
#endif
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

	for (auto& mesh : mMeshes) {
		delete mesh.second;
	}
	mMeshes.clear();

	for (auto& texture : mTextures) {
		delete texture.second;
	}
	mTextures.clear();

	for (auto& shader : mShaders) {
		delete shader.second;
	}
	mShaders.clear();

	for (auto& material : mMaterials) {
		delete material.second;
	}
	mMaterials.clear();

	for (auto& animation : mAnimations) {
		delete animation.second;
	}
	mAnimations.clear();

	delete mTempPlayer;
	delete mUi;

	delete mPhysics;
	delete mRenderer;
	delete mWorld;
#ifdef USEGL
	delete mAnimation;
	delete mSoundManager;
#endif
	delete mInventoryBuffSystemClassPtr;
	delete mSuspicionSystemClassPtr;



}

void LevelManager::ClearLevel() {
	mIsLevelInitialised = false;
	mRenderer->ClearLights();
	mWorld->ClearAndErase();
	mPhysics->Clear();
	mLevelFloorMatrices.clear();
	mLevelWallMatrices.clear();
	mLevelCornerWallMatrices.clear();
	mUpdatableObjects.clear();
	mLevelLayout.clear();
	mRenderer->ClearInstanceObjects();
#ifdef USEGL
	mAnimation->Clear();
#endif
	mInventoryBuffSystemClassPtr->Reset();
	mSuspicionSystemClassPtr->Reset(mInventoryBuffSystemClassPtr);
	if(mTempPlayer)mTempPlayer->ResetPlayerPoints();
	mBaseFloor = nullptr;
	mBaseWall = nullptr;
	mBaseCornerWall = nullptr;
	mGuardObjects.clear();
	mCCTVTransformList.clear();


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
	std::vector<Transform> cctvPositions;
	LoadMap((*mLevelList[levelID]).GetTileMap(), Vector3(0, 0, 0));
	LoadVents((*mLevelList[levelID]).GetVents(), (*mLevelList[levelID]).GetVentConnections(), isMultiplayer);
	LoadDoors((*mLevelList[levelID]).GetDoors(), Vector3(0, 0, 0), isMultiplayer);
	LoadLights((*mLevelList[levelID]).GetLights(), Vector3(0, 0, 0));
	LoadCCTVList((*mLevelList[levelID]).GetCCTVTransforms(), Vector3(0, 0, 0));
	mHelipad = AddHelipadToWorld((*mLevelList[levelID]).GetHelipadPosition());
	mPrisonDoor = AddPrisonDoorToWorld((*mLevelList[levelID]).GetPrisonDoor());
	mUpdatableObjects.push_back(mPrisonDoor);

	for (Vector3 itemPos : (*mLevelList[levelID]).GetItemPositions()) {
		itemPositions.push_back(itemPos);
	}

	for (auto const& [key, val] : (*mLevelList[levelID]).GetRooms()) {
		std::random_device rd;
		std::mt19937 g(rd());
		std::shuffle(mRoomList.begin(), mRoomList.end(), g);
		switch ((*val).GetType()) {
		case Small:
		case Medium:
		case Large:
			for (Room* room : mRoomList) {
				if (room->GetType() == (*val).GetType() && room->GetDoorConfig() == (*val).GetDoorConfig()) {
					LoadMap(room->GetTileMap(), key, (*val).GetPrimaryDoor() * 90);
					LoadLights(room->GetLights(), key, (*val).GetPrimaryDoor() * 90);
					LoadDoors(room->GetDoors(), key, isMultiplayer, (*val).GetPrimaryDoor() * 90);
					LoadCCTVList(room->GetCCTVTransforms(), key, (*val).GetPrimaryDoor() * 90);
					for (int i = 0; i < room->GetItemPositions().size(); i++) {
						roomItemPositions.push_back(room->GetItemPositions()[i] + key);
					}
					break;
				}
			}
			break;
		}
	}
	if(mHasStartedGame) mNavMeshThread.join();
	mHasSetNavMesh = false;
	mNavMeshThread = std::thread([this] {
		mBuilder->BuildNavMesh(mLevelLayout);
		LoadDoorsInNavGrid();
		mHasSetNavMesh = true;
		std::cout << "Nav Mesh Set\n";
		});

	if (!isMultiplayer) {
		AddPlayerToWorld((*mLevelList[levelID]).GetPlayerStartTransform(playerID), "Player", mPrisonDoor);
		mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->Attach(mTempPlayer);
		mInventoryBuffSystemClassPtr->GetPlayerBuffsPtr()->Attach(mTempPlayer);
	}
#ifdef USEGL
	else {
		if (!serverPlayersPtr) {
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

	LoadGuards((*mLevelList[levelID]).GetGuardCount(), isMultiplayer);
	LoadCCTVs();


#endif
  
	LoadItems(itemPositions, roomItemPositions, isMultiplayer);
	SendWallFloorInstancesToGPU();
	
#ifdef USEGL
	mAnimation->SetGameObjectLists(mUpdatableObjects,mPlayerTextures,mGuardTextures);
#endif
	mRenderer->FillLightUBO();
	mRenderer->FillTextureDataUBO();

	mTimer = INIT_TIMER_VALUE;

	//Temp fix for crash problem
	mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->Attach(this);
	mInventoryBuffSystemClassPtr->GetPlayerBuffsPtr()->Attach(mMainFlag);
	mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->Attach(mMainFlag);
	mInventoryBuffSystemClassPtr->GetPlayerBuffsPtr()->Attach(mSuspicionSystemClassPtr->GetLocalSuspicionMetre());
  
	while (!mBuilder->HasSetSize()) {

	}
	mHasStartedGame = true;
	mIsLevelInitialised = true;
}

void LevelManager::SendWallFloorInstancesToGPU() {
#ifdef USEGL
	OGLMesh* floorInstance = (OGLMesh*)mMeshes["FloorCube"];
	floorInstance->SetInstanceMatrices(mLevelFloorMatrices);
	OGLMesh* wallInstance = (OGLMesh*)mMeshes["StraightWall"];
	wallInstance->SetInstanceMatrices(mLevelWallMatrices);
	OGLMesh* cornerWallInstance = (OGLMesh*)mMeshes["CornerWall"];
	cornerWallInstance->SetInstanceMatrices(mLevelCornerWallMatrices);
	GameTechRenderer* renderer = (GameTechRenderer*)mRenderer;
	renderer->SetInstanceObjects(mBaseFloor, mBaseWall, mBaseCornerWall);
#endif

}

void LevelManager::AddNetworkObject(GameObject& objToAdd) {
#ifdef USEGL
	auto* networkObj = new NetworkObject(objToAdd, mNetworkIdBuffer);
	mNetworkIdBuffer++;
	objToAdd.SetNetworkObject(networkObj);

	auto* sceneManager = SceneManager::GetSceneManager();
	Scene* currentScene = sceneManager->GetCurrentScene();

	DebugNetworkedGame* networkScene = static_cast<DebugNetworkedGame*>(currentScene);
	networkScene->AddNetworkObjectToNetworkObjects(networkObj);
#endif
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
		if (mIsLevelInitialised) {
			mPhysics->Update(dt);
#ifdef USEGL
			mAnimation->Update(dt, mUpdatableObjects);
#endif
		}
#ifdef USEGL
		if (mUpdatableObjects.size()>0) {
			mSoundManager->UpdateSounds(mUpdatableObjects);
		}
#endif
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
	std::ifstream assetsFile("../Assets/UsedAssets.csv");
	std::string line;
	std::string* assetDetails = new std::string[4];
	vector<std::string> groupDetails;
	std::string groupType = "";
	std::thread animLoadThread;
	std::thread matLoadThread;
	while (getline(assetsFile, line)) {
		for (int i = 0; i < 3; i++) {
			assetDetails[i] = line.substr(0, line.find(","));
			line.erase(0, assetDetails[i].length() + 1);
		}
		assetDetails[3] = line;
		if (groupType == "") groupType = assetDetails[0];
		if (groupType != assetDetails[0]) {
			if (groupType == "anim") {
        #ifdef USEGL // remove after implemented
				animLoadThread = std::thread([this, groupDetails] {
					for (int i = 0; i < groupDetails.size(); i += 3) {
						mAnimations[groupDetails[i]] = mRenderer->LoadAnimation(groupDetails[i + 1]);
					}
					});
        #endif
			}
			else if (groupType == "mat") {
				matLoadThread = std::thread([this, groupDetails] {
					for (int i = 0; i < groupDetails.size(); i += 3) {
						mMaterials[groupDetails[i]] = mRenderer->LoadMaterial(groupDetails[i + 1]);
					}
					});
			}
			else if (groupType == "msh") {
				for (int i = 0; i < groupDetails.size(); i += 3) {
					mMeshes[groupDetails[i]] = mRenderer->LoadMesh(groupDetails[i + 1]);
				}
			}
			else if (groupType == "tex") {
				for (int i = 0; i < groupDetails.size(); i += 3) {
					mTextures[groupDetails[i]] = mRenderer->LoadTexture(groupDetails[i + 1]);
				}
			}
			else if (groupType == "sdr") {
				for (int i = 0; i < groupDetails.size(); i += 3) {
					mShaders[groupDetails[i]] = mRenderer->LoadShader(groupDetails[i + 1], groupDetails[i + 2]);
				}
			}
			groupType = assetDetails[0];
			groupDetails.clear();
		}
		for (int i = 1; i < 4; i++) {
			groupDetails.push_back(assetDetails[i]);
		}
	}
	delete[] assetDetails;

	animLoadThread.join();
	//preLoadList
	mPreAnimationList.insert(std::make_pair("GuardStand", mAnimations["RigStand"]));
	mPreAnimationList.insert(std::make_pair("GuardWalk", mAnimations["RigWalk"]));
	mPreAnimationList.insert(std::make_pair("GuardSprint", mAnimations["RigSprint"]));
	

	mPreAnimationList.insert(std::make_pair("PlayerStand", mAnimations["GuardStand"]));
	mPreAnimationList.insert(std::make_pair("PlayerWalk", mAnimations["GuardWalk"]));
	mPreAnimationList.insert(std::make_pair("PlayerSprint", mAnimations["GuardSprint"]));

	//icons
	vector<Texture*> keyTexVec = { 
		{mTextures["KeyIcon1"]},
		{mTextures["KeyIcon2"]},
		{mTextures["KeyIcon3"]}
	};

	vector<Texture*> susTexVec = {
		{mTextures["LowSusBar"]},
		{mTextures["MidSusBar"]},
		{mTextures["HighSusBar"]}
	};
	mUi->SetTextureVector("key", keyTexVec);
	mUi->SetTextureVector("bar", susTexVec);
	matLoadThread.join();
}

void LevelManager::LoadMap(const std::unordered_map<Transform, TileType>& tileMap, const Vector3& startPosition, int rotation) {
	for (auto const& [key, val] : tileMap) {
		Transform offsetKey = Transform();
		offsetKey.SetPosition(key.GetPosition()).SetOrientation(key.GetOrientation());
		offsetKey.SetMatrix(Matrix4::Rotation(rotation, Vector3(0, -1, 0)) * offsetKey.GetMatrix());
		offsetKey.SetPosition(offsetKey.GetPosition() + startPosition);
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

void LevelManager::LoadLights(const std::vector<Light*>& lights, const Vector3& centre, int rotation) {
#ifdef USEGL
	for (int i = 0; i < lights.size(); i++) {
		if (lights[i]->GetType() == Light::Point) {
			PointLight* pl = static_cast<PointLight*>(lights[i]);
			PointLight* newPL = new PointLight((Matrix4::Rotation(rotation, Vector3(0, -1, 0)) * (pl->GetPosition())) + centre,
				pl->GetColour(), pl->GetRadius());
			mRenderer->AddLight(newPL);
		}
		else if (lights[i]->GetType() == Light::Spot) {
			SpotLight* sl = static_cast<SpotLight*>(lights[i]);
			SpotLight* newSL = new SpotLight(Matrix4::Rotation(rotation, Vector3(0, -1, 0)) * sl->GetDirection(), 
				(Matrix4::Rotation(rotation, Vector3(0, 1, 0)) * (sl->GetPosition())) + centre,
				sl->GetColour(), sl->GetRadius(), sl->GetAngle(), 2);
			mRenderer->AddLight(newSL);
		}
		else {
			DirectionLight* dl = static_cast<DirectionLight*>(lights[i]);
			DirectionLight* newDL = new DirectionLight(dl->GetDirection(), dl->GetColour(), dl->GetRadius(), dl->GetCentre());
			mRenderer->AddLight(newDL);
		}
	}
#endif
}

void LevelManager::LoadGuards(int guardCount, bool isInMultiplayer) {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(0, (*mLevelList[mActiveLevel]).GetGuardPaths().size() - 1);
	std::set<int> pickedNumbers;
	for (int i = 0; i < guardCount; i++) {
		int pathIndex;
		do {
			pathIndex = dis(gen);
		} while (pickedNumbers.count(pathIndex) > 0);
		pickedNumbers.insert(pathIndex);
		GuardObject* addedGuard = AddGuardToWorld((*mLevelList[mActiveLevel]).GetGuardPaths()[pathIndex], (*mLevelList[mActiveLevel]).GetPrisonPosition(), "Guard", isInMultiplayer);
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

void LevelManager::LoadVents(const std::vector<Vent*>& vents, std::vector<int> ventConnections, bool isMultiplayerLevel) {
	std::vector<Vent*> addedVents;
	for (int i = 0; i < vents.size(); i++) {
		addedVents.push_back(AddVentToWorld(vents[i], isMultiplayerLevel));
	}
	for (int i = 0; i < addedVents.size(); i++) {
		addedVents[i]->ConnectVent(addedVents[ventConnections[i]]);
	}
}

void LevelManager::LoadDoors(const std::vector<Door*>& doors, const Vector3& centre, bool isMultiplayerLevel, int rotation) {
	for (int i = 0; i < doors.size(); i++) {
		Transform doorTransform = Transform();
		doorTransform.SetPosition(Matrix4::Rotation(rotation, Vector3(0, -1, 0)) * doors[i]->GetTransform().GetPosition())
			.SetOrientation(Quaternion::EulerAnglesToQuaternion(0, rotation, 0) * doors[i]->GetTransform().GetOrientation());
		doorTransform.SetScale((doorTransform.GetOrientation() * Vector3(1, 9, 9)).Abs());
		InteractableDoor* interactableDoorPtr = AddDoorToWorld(doorTransform, centre, isMultiplayerLevel);
		mUpdatableObjects.push_back(interactableDoorPtr);
	}
}

void LevelManager::LoadCCTVList(const std::vector<Transform>& transforms, const Vector3& startPosition, int rotation) {
	for (int i = 0; i < transforms.size(); i++) {
		Transform offsetTransform = Transform();
		offsetTransform.SetPosition(transforms[i].GetPosition()).SetOrientation(transforms[i].GetOrientation());
		offsetTransform.SetMatrix(Matrix4::Rotation(rotation, Vector3(0, -1, 0)) * offsetTransform.GetMatrix());
		offsetTransform.SetPosition(offsetTransform.GetPosition() + startPosition);
		mCCTVTransformList.push_back(offsetTransform);
	}
}

void LevelManager::LoadCCTVs() {
	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(mCCTVTransformList.begin(), mCCTVTransformList.end(), g);
	for (int i = 0; i < (*mLevelList[mActiveLevel]).GetCCTVCount(); i++) {
		AddCCTVToWorld(mCCTVTransformList[i]);
	}
}

void LevelManager::LoadDoorsInNavGrid() {
	for (int i = 0; i < mUpdatableObjects.size(); i++) {
		if (mUpdatableObjects[i]->GetName() == "InteractableDoor" || mUpdatableObjects[i]->GetName() == "Prison Door") {
			float* startPos = new float[3] {mUpdatableObjects[i]->GetTransform().GetPosition().x, 
				mUpdatableObjects[i]->GetTransform().GetPosition().y, 
				mUpdatableObjects[i]->GetTransform().GetPosition().z};
			AABBVolume* volume = (AABBVolume*)mUpdatableObjects[i]->GetBoundingVolume();
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

void LevelManager::SetGameState(GameStates state) {
	mGameState = state;
}

void LevelManager::SetPlayersForGuards() const {
	//TODO(erendgrmnc): Refactor it
#ifdef USEGL
	for (GuardObject* guard : mGuardObjects) {
		for (int i = 0; i < serverPlayersPtr->size(); i++) {
			if (serverPlayersPtr->at(i) != nullptr){
				guard->AddPlayer(serverPlayersPtr->at(i));
			}
		}
	}
#endif
}

PlayerObject* LevelManager::GetNearestPlayer(const Vector3& startPos) const {
#ifdef USEGL
	NetworkPlayer& firstPlayer = *serverPlayersPtr->at(0);
	PlayerObject* returnObj = &firstPlayer;
	const Vector3& firstPos = firstPlayer.GetTransform().GetPosition();
	float minDistance = sqrt((startPos.x - firstPos.x) * (startPos.x - firstPos.x) +
		(startPos.z - firstPos.z) * (startPos.z - firstPos.z));

	for (int i = 1; i < serverPlayersPtr->size(); i++) {
		NetworkPlayer* serverPlayer = serverPlayersPtr->at(i);
		if (serverPlayer != nullptr) {
			const Vector3& playerPos = serverPlayer->GetTransform().GetPosition();

			float distance = sqrt((startPos.x - playerPos.x) * (startPos.x - playerPos.x) +
				(startPos.z - playerPos.z) * (startPos.z - playerPos.z));
			if (distance < minDistance) {
				minDistance = distance;
				returnObj = serverPlayer;
			}
		}
		
	}

	return returnObj;
#endif
}

PrisonDoor* LevelManager::GetPrisonDoor() const {
	return mPrisonDoor;
}

void LevelManager::InitialiseIcons() {
	//Inventory
	UISystem::Icon* mInventoryIcon1 = mUi->AddIcon(Vector2(45, 90), 4.5, 8, mTextures["InventorySlot"]);
	mUi->SetEquippedItemIcon(FIRST_ITEM_SLOT, *mInventoryIcon1);

	UISystem::Icon* mInventoryIcon2 = mUi->AddIcon(Vector2(50, 90), 4.5, 8, mTextures["InventorySlot"], 0.5);
	mUi->SetEquippedItemIcon(SECOND_ITEM_SLOT, *mInventoryIcon2);

	//Buff
	UISystem::Icon* mSilentSprintIcon = mUi->AddIcon(Vector2(65, 3), 4.5, 7, mTextures["SilentRun"], 0.3);
	mUi->SetEquippedItemIcon(SILENT_BUFF_SLOT, *mSilentSprintIcon);

	UISystem::Icon* mSlowIcon = mUi->AddIcon(Vector2(70, 3), 4.5, 7, mTextures["SlowDown"], 0.3);
	mUi->SetEquippedItemIcon(SLOW_BUFF_SLOT, *mSlowIcon);

	UISystem::Icon* mStunIcon = mUi->AddIcon(Vector2(75, 3), 4.5, 7, mTextures["Stun"], 0.3);
	mUi->SetEquippedItemIcon(STUN_BUFF_SLOT, *mStunIcon);

	UISystem::Icon* mSpeedIcon = mUi->AddIcon(Vector2(80, 3), 4.5, 7, mTextures["SpeedUp"], 0.3);
	mUi->SetEquippedItemIcon(SPEED_BUFF_SLOT, *mSpeedIcon);

	//suspicion
	UISystem::Icon* mSuspisionBarIcon = mUi->AddIcon(Vector2(90, 15), 3, 75, mTextures["LowSusBar"], 0.7);
	mUi->SetEquippedItemIcon(SUSPISION_BAR_SLOT, *mSuspisionBarIcon);

	UISystem::Icon* mSuspisionIndicatorIcon = mUi->AddIcon(Vector2(90, 86), 3, 3, mTextures["SusIndicator"], 0.7);
	mUi->SetEquippedItemIcon(SUSPISION_INDICATOR_SLOT, *mSuspisionIndicatorIcon);

	UISystem::Icon* mCross = mUi->AddIcon(Vector2(50, 50), 3, 5, mTextures["Cross"],0.0);
	mUi->SetEquippedItemIcon(CROSS, *mCross);

	UISystem::Icon* mAlarm = mUi->AddIcon(Vector2(0, 0), 100, 100, mTextures["Alarm"], 0.0);
	mUi->SetEquippedItemIcon(ALARM, *mAlarm);

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

	wall->SetRenderObject(new RenderObject(&wall->GetTransform(), mMeshes["StraightWall"], mTextures["WallTex"], mTextures["WallNormal"], mShaders["Instance"],
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

	wall->SetRenderObject(new RenderObject(&wall->GetTransform(), mMeshes["CornerWall"], mTextures["WallTex"], mTextures["WallNormal"], mShaders["Instance"],
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

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), mMeshes["FloorCube"], mTextures["FloorAlbedo"], mTextures["FloorNormal"], mShaders["Instance"],
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

	camera->SetRenderObject(new RenderObject(&camera->GetTransform(), mMeshes["CCTV"], mTextures["FloorAlbedo"], mTextures["FloorNormal"], mShaders["Basic"],
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

	helipad->SetRenderObject(new RenderObject(&helipad->GetTransform(), mMeshes["Cube"], mTextures["Basic"], mTextures["FloorNormal"], mShaders["Basic"],
		std::sqrt(std::pow(wallSize.x, 2) + std::powf(wallSize.z, 2))));
	helipad->SetPhysicsObject(new PhysicsObject(&helipad->GetTransform(), helipad->GetBoundingVolume()));

	helipad->GetPhysicsObject()->SetInverseMass(0);
	helipad->GetPhysicsObject()->InitCubeInertia();

	helipad->GetRenderObject()->SetColour(Vector4(0.0f, 0.4f, 0.2f, 1));

	mWorld->AddGameObject(helipad);

	mLevelLayout.push_back(helipad);

	return helipad;
}

Vent* LevelManager::AddVentToWorld(Vent* vent, bool isMultiplayerLevel) {
	Vent* newVent = new Vent();
	newVent->SetName("Vent");

	Vector3 size = Vector3(1.25f, 1.25f, 0.05f);
	OBBVolume* volume = new OBBVolume(size);

	newVent->SetBoundingVolume((CollisionVolume*)volume);

	newVent->GetTransform()
		.SetPosition(vent->GetTransform().GetPosition())
		.SetOrientation(vent->GetTransform().GetOrientation())
		.SetScale(size*2);

	newVent->SetRenderObject(new RenderObject(&newVent->GetTransform(), mMeshes["Cube"], mTextures["Basic"], mTextures["FloorNormal"], mShaders["Basic"],
		std::sqrt(std::pow(size.x, 2) + std::powf(size.y, 2))));
	newVent->SetPhysicsObject(new PhysicsObject(&newVent->GetTransform(), newVent->GetBoundingVolume(), 1, 1, 5));


	newVent->GetPhysicsObject()->SetInverseMass(0);
	newVent->GetPhysicsObject()->InitCubeInertia();

	newVent->SetCollisionLayer(StaticObj);

	if (isMultiplayerLevel) {
		AddNetworkObject(*newVent);
	}

	mWorld->AddGameObject(newVent);

	return newVent;
}

InteractableDoor* LevelManager::AddDoorToWorld(const Transform& transform, const Vector3& offset, bool isMultiplayerLevel) {
	InteractableDoor* newDoor = new InteractableDoor();

	AABBVolume* volume = new AABBVolume(transform.GetScale()/2);
	newDoor->SetBoundingVolume((CollisionVolume*)volume);

	newDoor->GetTransform()
		.SetPosition(transform.GetPosition() + offset)
		.SetOrientation(transform.GetOrientation())
		.SetScale(Vector3(1, 9, 9));

	newDoor->SetRenderObject(new RenderObject(&newDoor->GetTransform(), mMeshes["Cube"], mTextures["Basic"], mTextures["FloorNormal"], mShaders["Basic"],
		std::sqrt(std::pow(transform.GetScale().y/2, 2) + std::powf(transform.GetScale().z / 2, 2))));
	newDoor->SetPhysicsObject(new PhysicsObject(&newDoor->GetTransform(), newDoor->GetBoundingVolume(), 1, 1, 5));
#ifdef USEGL
	newDoor->SetSoundObject(new SoundObject());
#endif
	newDoor->GetPhysicsObject()->SetInverseMass(0);
	newDoor->GetPhysicsObject()->InitCubeInertia();

	newDoor->SetCollisionLayer(NoSpecialFeatures);

	if (isMultiplayerLevel) {
		AddNetworkObject(*newDoor);
	}

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

	newDoor->SetRenderObject(new RenderObject(&newDoor->GetTransform(), mMeshes["Cube"], mTextures["Basic"], mTextures["FloorNormal"], mShaders["Basic"],
		std::sqrt(std::pow(size.y, 2) + std::powf(size.z, 2))));
	newDoor->SetPhysicsObject(new PhysicsObject(&newDoor->GetTransform(), newDoor->GetBoundingVolume(), 1, 1, 5));
#ifdef USEGL
	newDoor->SetSoundObject(new SoundObject());
#endif
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

	flag->SetRenderObject(new RenderObject(&flag->GetTransform(), mMeshes["Sphere"], mTextures["Basic"], mTextures["FloorNormal"], mShaders["Basic"], 0.75f));
	flag->SetPhysicsObject(new PhysicsObject(&flag->GetTransform(), flag->GetBoundingVolume()));
#ifdef USEGL
	flag->SetSoundObject(new SoundObject());
#endif
	flag->SetCollisionLayer(Collectable);

	flag->GetPhysicsObject()->SetInverseMass(0);
	flag->GetPhysicsObject()->InitSphereInertia(false);

	flag->GetRenderObject()->SetColour(Vector4(1.0f, 1.0f, 1.0f, 1));

	mWorld->AddGameObject(flag);

	mUpdatableObjects.push_back(flag);

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

	pickup->SetRenderObject(new RenderObject(&pickup->GetTransform(), mMeshes["Sphere"], mTextures["FloorAlbedo"], mTextures["FloorNormal"], mShaders["Basic"], 0.75f));
	pickup->SetPhysicsObject(new PhysicsObject(&pickup->GetTransform(), pickup->GetBoundingVolume()));
#ifdef USEGL
	pickup->SetSoundObject(new SoundObject());
#endif
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

	pointObject->SetRenderObject(new RenderObject(&pointObject->GetTransform(), mMeshes["Sphere"], mTextures["FloorAlbedo"], mTextures["FloorNormal"], mShaders["Basic"], 0.75f));
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

void LevelManager::CreatePlayerObjectComponents(PlayerObject& playerObject, const Vector3& position) {
	CapsuleVolume* volume = new CapsuleVolume(1.4f, 1.0f);

	playerObject.SetBoundingVolume((CollisionVolume*)volume);

	playerObject.GetTransform()
		.SetScale(Vector3(PLAYER_MESH_SIZE, PLAYER_MESH_SIZE, PLAYER_MESH_SIZE))
		.SetPosition(position);

	playerObject.SetRenderObject(new RenderObject(&playerObject.GetTransform(), mMeshes["Farmer"], mTextures["FleshyAlbedo"], mTextures["FleshyNormal"], mShaders["Basic"],
		PLAYER_MESH_SIZE));
	playerObject.SetPhysicsObject(new PhysicsObject(&playerObject.GetTransform(), playerObject.GetBoundingVolume(), 1, 1, 5));
#ifdef USEGL
	playerObject.SetSoundObject(new SoundObject(mSoundManager->AddWalkSound()));
#endif
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

	playerObject.SetRenderObject(new RenderObject(&playerObject.GetTransform(), mMeshes["Guard"], mTextures["FleshyAlbedo"], mTextures["FleshyNormal"], mShaders["Animation"],
		PLAYER_MESH_SIZE));
	playerObject.SetPhysicsObject(new PhysicsObject(&playerObject.GetTransform(), playerObject.GetBoundingVolume(), 1, 1, 5));
#ifdef USEGL
	playerObject.SetSoundObject(new SoundObject(mSoundManager->AddWalkSound()));
#endif
	playerObject.GetRenderObject()->SetAnimationObject(new AnimationObject(AnimationObject::AnimationType::playerAnimation, mAnimations["GuardStand"], mMaterials["Guard"]));


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

	Texture& itemTex = *mTextures["InventorySlot"];

	mUi->ChangeEquipmentSlotTexture(itemSlot, itemTex);
}

void LevelManager::ResetEquippedIconTexture() {

	Texture& itemTex = *mTextures["InventorySlot"];

	mUi->ChangeEquipmentSlotTexture(FIRST_ITEM_SLOT, itemTex);
	mUi->ChangeEquipmentSlotTexture(SECOND_ITEM_SLOT, itemTex);

	mUi->ChangeBuffSlotTransparency(SILENT_BUFF_SLOT, 0.3);
	mUi->ChangeBuffSlotTransparency(SLOW_BUFF_SLOT, 0.3);
	mUi->ChangeBuffSlotTransparency(STUN_BUFF_SLOT, 0.3);
	mUi->ChangeBuffSlotTransparency(SPEED_BUFF_SLOT, 0.3);
	mUi->ChangeBuffSlotTransparency(CROSS, 0.8);
	


}


GameResults LevelManager::CheckGameWon() {
	if (mTempPlayer && mHelipad) {
		std::tuple<bool, int> colResult = mHelipad->GetCollidingWithPlayer();
		bool isPlayerOnHelipad = std::get<0>(colResult);
		if (isPlayerOnHelipad) {
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

GuardObject* LevelManager::AddGuardToWorld(const vector<Vector3> nodes, const Vector3 prisonPosition, const std::string& guardName, bool isInMultiplayer) {
	GuardObject* guard = new GuardObject(guardName);
	
	float meshSize = PLAYER_MESH_SIZE;
	float inverseMass = PLAYER_INVERSE_MASS;

	CapsuleVolume* volume = new CapsuleVolume(1.3f, 1.0f, Vector3(0, 2.0f, 0));
	guard->SetBoundingVolume((CollisionVolume*)volume);

	int currentNode = 1;
	guard->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(nodes[currentNode] + Vector3(20,-1.5f,20));


	guard->SetPhysicsObject(new PhysicsObject(&guard->GetTransform(), guard->GetBoundingVolume(), 1, 0, 10));

	guard->GetPhysicsObject()->SetInverseMass(PLAYER_INVERSE_MASS);
	guard->GetPhysicsObject()->InitSphereInertia(false);
	guard->SetCollisionLayer(Npc);

	RenderObject* renderObject = new RenderObject(&guard->GetTransform(), mMeshes["Rig"], mTextures["FleshyAlbedo"], mTextures["FleshyNormal"], mShaders["Animation"], meshSize);
	AnimationObject* animObject = new AnimationObject(AnimationObject::AnimationType::guardAnimation, mAnimations["RigStand"], mMaterials["Rig"]);

	renderObject->SetAnimationObject(animObject);
	guard->SetRenderObject(renderObject);
#ifdef USEGL
	guard->SetSoundObject(new SoundObject(mSoundManager->AddWalkSound()));
#endif
	guard->SetPatrolNodes(nodes);
	guard->SetCurrentNode(currentNode);
	guard->SetObjectState(GameObject::Idle);
	if (isInMultiplayer) {
		AddNetworkObject(*guard);
	}
	else {
		guard->SetPlayer(mTempPlayer);
	}

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

	soundEmitterObjectPtr->SetRenderObject(new RenderObject(&soundEmitterObjectPtr->GetTransform(), mMeshes["Sphere"], mTextures["Basic"], mTextures["FloorNormal"], mShaders["Basic"], 
		0.75f));
	soundEmitterObjectPtr->SetPhysicsObject(new PhysicsObject(&soundEmitterObjectPtr->GetTransform(), soundEmitterObjectPtr->GetBoundingVolume()));

	soundEmitterObjectPtr->SetCollisionLayer(Collectable);
#ifdef USEGL
	soundEmitterObjectPtr->SetSoundObject(new SoundObject(mSoundManager->AddSoundEmitterSound(position)));
#endif
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

Helipad* LevelManager::GetHelipad() {
	return mHelipad;
}
