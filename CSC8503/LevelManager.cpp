#include "LevelManager.h"
#ifdef USEGL
#include "Windows.h"
#include "Psapi.h"
#endif

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
#include "PointLight.h"
#include "SpotLight.h"
#include "DirectionalLight.h"
#include "BaseLight.h"

#include "InventoryBuffSystem/FlagGameObject.h"
#include "InventoryBuffSystem/PickupGameObject.h"
#include "InventoryBuffSystem/InventoryBuffSystem.h"
#include "InventoryBuffSystem/SoundEmitter.h"
#include "PointGameObject.h"
#include "DebugNetworkedGame.h"
#include "SceneManager.h"
#include "NetworkObject.h"
#include "UISystem.h"
#include "Assets.h"
#include "Debug.h"
#ifdef USEGL
#include "MiniMap.h"
#endif
#include <filesystem>
#include <fstream>

namespace {
	constexpr int NETWORK_ID_BUFFER_START = 10;
}

using namespace NCL::CSC8503;

LevelManager* LevelManager::instance = nullptr;

LevelManager::LevelManager() {
	mWorld = new GameWorld();
#ifdef USEGL
	std::thread loadSoundManager([this] {mSoundManager = new SoundManager(mWorld); });

	mRenderer = new GameTechRenderer(*mWorld);
#endif
#ifdef USEPROSPERO
	mRenderer = new GameTechAGCRenderer(*mWorld);
#endif
	mUi = new UISystem();

	mAnimation = new AnimationSystem(*mWorld, mPreAnimationList);
	mBuilder = new RecastBuilder();
	mPhysics = new PhysicsSystem(*mWorld);
	mPhysics->UseGravity(true);

	mPlayerInventoryObservers.clear();
	mPlayerBuffsObservers.clear();
	mGlobalSuspicionObserver.clear();

	mInventoryBuffSystemClassPtr = new InventoryBuffSystemClass();
	mPlayerInventoryObservers.push_back(this);
	mSuspicionSystemClassPtr = new SuspicionSystemClass(mInventoryBuffSystemClassPtr);
	mDtSinceLastFixedUpdate = 0;

	mActiveLevel = -1;

	mGameState = MenuState;

	mNetworkIdBuffer = NETWORK_ID_BUFFER_START;

	mIsLevelInitialised = false;
#ifdef USEGL
	loadSoundManager.join();

    InitialiseMiniMap();
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

	delete mAnimation;
#ifdef USEGL
	delete mSoundManager;
#endif
	delete mInventoryBuffSystemClassPtr;
	delete mSuspicionSystemClassPtr;
#ifdef USEGL
	delete mSoundManager;
	delete mMiniMap;
#endif
}

void LevelManager::ClearLevel() {
	mIsLevelInitialised = false;
	mRenderer->ClearLights();
	mWorld->ClearAndErase();
	mPhysics->Clear();
	mInstanceMatrices.clear();
	mBaseObjects.clear();
	mUpdatableObjects.clear();
	mLevelLayout.clear();
	mRenderer->ClearInstanceObjects();
	mAnimation->Clear();
	mPlayerInventoryObservers.clear();
	mPlayerBuffsObservers.clear();
	mGlobalSuspicionObserver.clear();
	mInventoryBuffSystemClassPtr->Reset();
	mSuspicionSystemClassPtr->Reset(mInventoryBuffSystemClassPtr);
	if(mTempPlayer)mTempPlayer->ResetPlayerPoints();
	mGuardObjects.clear();
	mCCTVTransformList.clear();

	ResetEquippedIconTexture();
}

void LevelManager::InitialiseGameAssets() {
	if (!mAreAssetsInitialised) {
		mRoomList = std::vector<Room*>();
		std::thread loadRooms([this] {
			for (const filesystem::directory_entry& entry : std::filesystem::directory_iterator(Assets::LEVELDIR + "Rooms")) {
				Room* newRoom = new Room(entry.path().string());
				mRoomList.push_back(newRoom);
			}
			});
		mLevelList = std::vector<Level*>();
		std::thread loadLevels([this] {
			for (const filesystem::directory_entry& entry : std::filesystem::directory_iterator(Assets::LEVELDIR + "Levels")) {
				Level* newLevel = new Level(entry.path().string());
				mLevelList.push_back(newLevel);
			}
			});
		InitialiseAssets();
		InitialiseIcons();
		InitialiseDebug();
		loadRooms.join();
		loadLevels.join();
		mAreAssetsInitialised = true;
	}
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

void LevelManager::LoadLevel(int levelID, std::mt19937 seed, int playerID, bool isMultiplayer) {
	if (levelID > mLevelList.size() - 1) return;
	mActiveLevel = levelID;
	mStartTimer = 5;
	ClearLevel();
	std::vector<Vector3> itemPositions;
	std::vector<Vector3> roomItemPositions;
	std::vector<Transform> cctvPositions;
	LoadMap((*mLevelList[levelID]).GetTileMap(), Vector3(0, 0, 0));
	LoadVents((*mLevelList[levelID]).GetVents(), (*mLevelList[levelID]).GetVentConnections(), isMultiplayer);
	LoadDoors((*mLevelList[levelID]).GetDoors(), Vector3(0, 0, 0), isMultiplayer);
	LoadLights((*mLevelList[levelID]).GetLights(), Vector3(0, 0, 0));
	LoadCCTVList((*mLevelList[levelID]).GetCCTVTransforms(), Vector3(0, 0, 0));
	LoadDecorations((*mLevelList[levelID]).GetDecorationMap(), Vector3(0, 0, 0));

	mHelipad = AddHelipadToWorld((*mLevelList[levelID]).GetHelipadPosition());
	mPrisonDoor = AddPrisonDoorToWorld((*mLevelList[levelID]).GetPrisonDoor(), isMultiplayer);
	mUpdatableObjects.push_back(mPrisonDoor);

	for (Vector3 itemPos : (*mLevelList[levelID]).GetItemPositions()) {
		itemPositions.push_back(itemPos);
	}

	for (auto const& [key, val] : (*mLevelList[levelID]).GetRooms()) {
		std::shuffle(mRoomList.begin(), mRoomList.end(), seed);
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
					LoadDecorations(room->GetDecorationMap(), key, (*val).GetPrimaryDoor() * 90);
					for (int i = 0; i < room->GetItemPositions().size(); i++) {
						roomItemPositions.push_back(room->GetItemPositions()[i] + key);
					}
					break;
				}
			}
			break;
		}
	}
	mNavMeshThread = std::thread([this] {
		mBuilder->BuildNavMesh(mLevelLayout);
		LoadDoorsInNavGrid();
		std::cout << "Nav Mesh Set\n";
		});
	if (!isMultiplayer) {
		AddPlayerToWorld((*mLevelList[levelID]).GetPlayerStartTransform(playerID), "Player");
	}
#ifdef USEGL
	else {
		if (!serverPlayersPtr) {
			DebugNetworkedGame* game = reinterpret_cast<DebugNetworkedGame*>(SceneManager::GetSceneManager()->GetCurrentScene());
			serverPlayersPtr = game->GetServerPlayersPtr();
		}
	}
#endif

	LoadGuards((*mLevelList[levelID]).GetGuardCount(), isMultiplayer,seed);
	LoadCCTVs(seed,isMultiplayer);
  
	LoadItems(itemPositions, roomItemPositions, isMultiplayer, seed);
	SendWallFloorInstancesToGPU();

	if (!isMultiplayer) {
		mAnimation->SetGameObjectLists(mUpdatableObjects);
	}

	mRenderer->FillLightUBO();
	mRenderer->FillTextureDataUBO();

	mTimer = INIT_TIMER_VALUE;

	mIsLevelInitialised = true;
	mPlayerInventoryObservers.push_back(this);
	mPlayerBuffsObservers.push_back(mSuspicionSystemClassPtr->GetLocalSuspicionMetre());
	for (const auto invObserver : mPlayerInventoryObservers)
		mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->Attach(invObserver);

	for (const auto buffsObserver : mPlayerBuffsObservers)
		mInventoryBuffSystemClassPtr->GetPlayerBuffsPtr()->Attach(buffsObserver);
	mWorld->SortObjects();
}

void LevelManager::SendWallFloorInstancesToGPU() {
#ifdef USEGL
	for (const auto& [key, val] : mInstanceMatrices) {
		OGLMesh* instance = (OGLMesh*)mMeshes[key];
		instance->SetInstanceMatrices(val);
	}
	GameTechRenderer* renderer = (GameTechRenderer*)mRenderer;
	renderer->SetInstanceObjects(mBaseObjects);
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
#ifdef USEGL
	if (mShowDebug) {
		mTakeNextTime -= dt;
		if (mTakeNextTime < 0) {
			DebugUpdate(dt, isPlayingLevel, isPaused);
			mTakeNextTime = 0.1f;
			return;
		}
	}
#endif
	if (isPlayingLevel) {
		mGameState = LevelState;
		if (mStartTimer > 0) {
			if (mStartTimer == 5) {
				mTempPlayer->UpdateObject(dt);
			}
			mStartTimer -= dt;
			Debug::Print(to_string((int)mStartTimer + 1), Vector2(50, 50), Vector4(1, 1, 1, 1), 40.0f);
			if (mStartTimer <= 0) {
				mNavMeshThread.join();
			}
			if (SceneManager::GetSceneManager()->IsInSingleplayer()) {
				mRenderer->Render();
				Debug::UpdateRenderables(dt);
				return;
			}
		}
		else {
			if ((mUpdatableObjects.size() > 0)) {
				for (GameObject* obj : mUpdatableObjects) {
					obj->UpdateObject(dt);
				}
			}

			Debug::Print("TIME LEFT: " + to_string(int(mTimer)), Vector2(0, 3));
			mTimer -= dt;
		}
	}
	else
		mGameState = MenuState;

#ifdef USEGL
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F3)) {
		mShowDebug = !mShowDebug;
	}
	if (mShowDebug) {
		PrintDebug(dt);
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::F4)) {
			mShowVolumes = !mShowVolumes;
		}
	}
#endif

	if (isPaused) {
		mRenderer->Render();
		mGameState = PauseState;
	}
	else {
		mWorld->UpdateWorld(dt);
		mRenderer->Update(dt);
		if (mIsLevelInitialised) {
			mPhysics->Update(dt);
			mAnimation->Update(dt, mUpdatableObjects);
		}

		if (mUpdatableObjects.size() > 0) {
#ifdef USEGL
			mSoundManager->UpdateSounds(mUpdatableObjects);
#endif
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

void NCL::CSC8503::LevelManager::DebugUpdate(float dt, bool isPlayingLevel, bool isPaused) {
#ifdef USEGL
	std::chrono::steady_clock::time_point start;
	std::chrono::steady_clock::time_point end;
	std::chrono::duration<double, std::milli> timeTaken;
	if (isPlayingLevel) {
		mGameState = LevelState;
		if (mStartTimer > 0) {
			if (mStartTimer == 5) {
				mTempPlayer->UpdateObject(dt);
			}
			mStartTimer -= dt;
			Debug::Print(to_string((int)mStartTimer + 1), Vector2(50, 50), Vector4(1, 1, 1, 1), 40.0f);
			if (mStartTimer <= 0) {
				mNavMeshThread.join();
			}
		}
		else {
			if ((mUpdatableObjects.size() > 0)) {
				start = std::chrono::high_resolution_clock::now();
				for (GameObject* obj : mUpdatableObjects) {
					obj->UpdateObject(dt);
				}
				end = std::chrono::high_resolution_clock::now();
				timeTaken = end - start;
				mUpdateObjectsTime = timeTaken.count();
			}
			if (mTempPlayer)
				Debug::Print("POINTS: " + to_string(int(mTempPlayer->GetPoints())), Vector2(0, 6));

			Debug::Print("TIME LEFT: " + to_string(int(mTimer)), Vector2(0, 3));
			mTimer -= dt;
		}
	}
	else
		mGameState = MenuState;

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F3)) {
		mShowDebug = !mShowDebug;
	}
	if (mShowDebug) {
		PrintDebug(dt);
	}
	if (isPaused) {
		start = std::chrono::high_resolution_clock::now();
		mRenderer->Render();
		end = std::chrono::high_resolution_clock::now();
		timeTaken = end - start;
		mRenderTime = timeTaken.count();
		mGameState = PauseState;
		mPhysicsTime = 0;
		mAnimationTime = 0;
		mWorldTime = 0;
	}
	else {
		start = std::chrono::high_resolution_clock::now();
		mWorld->UpdateWorld(dt);
		end = std::chrono::high_resolution_clock::now();
		timeTaken = end - start;
		mWorldTime = timeTaken.count();
		mRenderer->Update(dt);
		if (mIsLevelInitialised) {
			start = std::chrono::high_resolution_clock::now();
			mPhysics->Update(dt);
			end = std::chrono::high_resolution_clock::now();
			timeTaken = end - start;
			mPhysicsTime = timeTaken.count();

			start = std::chrono::high_resolution_clock::now();
			mAnimation->Update(dt, mUpdatableObjects);
			end = std::chrono::high_resolution_clock::now();
			timeTaken = end - start;
			mAnimationTime = timeTaken.count();
		}
		else {
			mPhysicsTime = 0;
			mAnimationTime = 0;
		}
		if (mUpdatableObjects.size() > 0) {
			mSoundManager->UpdateSounds(mUpdatableObjects);
		}
		start = std::chrono::high_resolution_clock::now();
		mRenderer->Render();
		end = std::chrono::high_resolution_clock::now();
		timeTaken = end - start;
		mRenderTime = timeTaken.count();

		Debug::UpdateRenderables(dt);
		mDtSinceLastFixedUpdate += dt;
		if (mDtSinceLastFixedUpdate >= TIME_UNTIL_FIXED_UPDATE) {
			FixedUpdate(mDtSinceLastFixedUpdate);
			mDtSinceLastFixedUpdate = 0;
		}
	}
#endif
}

void LevelManager::FixedUpdate(float dt) {
	mInventoryBuffSystemClassPtr->Update(dt);
	mSuspicionSystemClassPtr->Update(dt);
}

void LevelManager::InitialiseAssets() {
	std::ifstream assetsFile(Assets::ASSETROOT + "UsedAssets.csv");
	std::string line;
	std::string* assetDetails = new std::string[4];
	vector<std::string> groupDetails;
	std::string groupType = "";
	std::thread animLoadThread;
	std::thread matLoadThread;
	int fileSize = 0;
	while (getline(assetsFile, line)) {
		fileSize++;
		if (line.substr(0, line.find(",")) == "mat") fileSize++;
	}
	assetsFile.clear();
	assetsFile.seekg(std::ifstream::beg);
	bool updateScreen = true;
	bool meshesLoaded = false;
	int meshCount = 0;
	bool loaded = false;
	int lines = 0;
	int animLines = 0;
	int matLines = 0;
	std::thread renderFlip([&updateScreen, &loaded, &meshesLoaded, &meshCount, &lines] {
		int added = 0;
		while (!loaded) {
			updateScreen = true;
			std::this_thread::sleep_for(16.7ms);
			if (meshesLoaded && added != meshCount) {
				added++;
				lines++;
			}
		}
		});
	while (getline(assetsFile, line)) {
		CheckRenderLoadScreen(updateScreen, lines + animLines + matLines, fileSize);
		for (int i = 0; i < 3; i++) {
			assetDetails[i] = line.substr(0, line.find(","));
			line.erase(0, assetDetails[i].length() + 1);
		}
		assetDetails[3] = line;
		if (groupType == "") groupType = assetDetails[0];
		if (groupType != assetDetails[0]) {
			if (groupType == "anim") {
				animLoadThread = std::thread([this, groupDetails, &animLines] {
					for (int i = 0; i < groupDetails.size(); i += 3) {
						mAnimations[groupDetails[i]] = mRenderer->LoadAnimation(groupDetails[i + 1]);
						animLines++;
					}
					});
			}
			else if (groupType == "mat") {
				matLoadThread = std::thread([this, groupDetails, &matLines] {
					for (int i = 0; i < groupDetails.size(); i += 3) {
						mMaterials[groupDetails[i]] = mRenderer->LoadMaterial(groupDetails[i + 1]);
						matLines++;
					}
					});
			}
			else if (groupType == "msh") {
#ifdef USEGL
				mRenderer->LoadMeshes(mMeshes, groupDetails);
				meshCount = groupDetails.size() / 3;
				meshesLoaded = true;
#endif
#ifdef USEPROSPERO
				for (int i = 0; i < groupDetails.size(); i += 3) {
					mMeshes[groupDetails[i]] = mRenderer->LoadMesh(groupDetails[i + 1]);
				}
#endif
			}
			else if (groupType == "tex") {
				for (int i = 0; i < groupDetails.size(); i += 3) {
					CheckRenderLoadScreen(updateScreen, lines + animLines + matLines, fileSize);
					mTextures[groupDetails[i]] = mRenderer->LoadTexture(groupDetails[i + 1]);
					lines++;
				}
			}
			else if (groupType == "sdr") {
				for (int i = 0; i < groupDetails.size(); i += 3) {
					CheckRenderLoadScreen(updateScreen, lines + animLines + matLines, fileSize);
					mShaders[groupDetails[i]] = mRenderer->LoadShader(groupDetails[i + 1], groupDetails[i + 2]);
					lines++;
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
	mPreAnimationList.insert(std::make_pair("GuardStand", mAnimations["GuardStand"]));
	mPreAnimationList.insert(std::make_pair("GuardWalk", mAnimations["GuardWalk"]));
	mPreAnimationList.insert(std::make_pair("GuardSprint", mAnimations["GuardSprint"]));
	mPreAnimationList.insert(std::make_pair("GuardPoint", mAnimations["GuardPoint"]));


	mPreAnimationList.insert(std::make_pair("PlayerStand", mAnimations["PlayerStand"]));
	mPreAnimationList.insert(std::make_pair("PlayerWalk", mAnimations["PlayerWalk"]));
	mPreAnimationList.insert(std::make_pair("PlayerSprint", mAnimations["PlayerSprint"]));

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
	for (auto const& [key, val] : mMaterials) {
		CheckRenderLoadScreen(updateScreen, lines + animLines + matLines, fileSize);
		if (key.substr(0, 6) == "Player") {
			mMeshMaterials[key] = mRenderer->LoadMeshMaterial(*mMeshes["Player"], *val);
		}
		else {
			mMeshMaterials[key] = mRenderer->LoadMeshMaterial(*mMeshes[key], *val);
		}
		lines++;
	}
	loaded = true;
	renderFlip.join();
	CheckRenderLoadScreen(updateScreen, 100, 100);
}

void LevelManager::CheckRenderLoadScreen(bool& updateScreen, int linesDone, int totalLines) {
	if (updateScreen) {
		updateScreen = false;
		float percent = linesDone / (float)totalLines;
#ifdef USEGL
		Debug::Print(std::format("Loading: {:.0f}%", percent * 100), Vector2(25, 50), Vector4(1 - percent, percent, 0, 1), 40.0f);
#endif
		mRenderer->Render();
		Debug::UpdateRenderables(0);
	}
}

void LevelManager::InitialiseDebug() {
#ifdef USEGL
	SYSTEM_INFO sysInfo;
	FILETIME ftime, fsys, fuser;

	GetSystemInfo(&sysInfo);
	mNumProcessors = sysInfo.dwNumberOfProcessors;

	GetSystemTimeAsFileTime(&ftime);
	memcpy(&mLastCPU, &ftime, sizeof(FILETIME));

	mSelf = GetCurrentProcess();
	GetProcessTimes(mSelf, &ftime, &ftime, &fsys, &fuser);
	memcpy(&mLastSysCPU, &fsys, sizeof(FILETIME));
	memcpy(&mLastUserCPU, &fuser, sizeof(FILETIME));

	mUpdateObjectsTime = 0;
	mRenderTime = 0;
	mWorldTime = 0;
	mPhysicsTime = 0;
	mAnimationTime = 0;
#endif
}

void NCL::CSC8503::LevelManager::PrintDebug(float dt) {
#ifdef USEGL
	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof(statex);
	GlobalMemoryStatusEx(&statex);

	PROCESS_MEMORY_COUNTERS_EX pmc;
	GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));

	FILETIME ftime, fsys, fuser;
	ULARGE_INTEGER now, sys, user;
	double percent;
	GetSystemTimeAsFileTime(&ftime);
	memcpy(&now, &ftime, sizeof(FILETIME));

	mSelf = GetCurrentProcess();
	GetProcessTimes(mSelf, &ftime, &ftime, &fsys, &fuser);
	memcpy(&sys, &fsys, sizeof(FILETIME));
	memcpy(&user, &fuser, sizeof(FILETIME));
	percent = (sys.QuadPart - mLastSysCPU.QuadPart) + (user.QuadPart - mLastUserCPU.QuadPart);
	percent /= (now.QuadPart - mLastCPU.QuadPart);
	percent /= mNumProcessors;
	mLastCPU = now;
	mLastUserCPU = user;
	mLastSysCPU = sys;
	percent *= 100;

	Debug::Print(std::format("FPS: {:.0f}", 1000.0f / dt), Vector2(1, 6), Vector4(1, 1, 1, 1), 15.0f);
	Debug::Print(std::format("Physical Memory Used: {} MB", pmc.WorkingSetSize / 1048576), Vector2(1, 9), Vector4(1, 0, 0, 1), 15.0f);
	Debug::Print(std::format("Total Physical Memory: {} MB", statex.ullTotalPhys / 1048576), Vector2(1, 12), Vector4(1, 0, 0, 1), 15.0f);
	Debug::Print(std::format("Virtual Memory Used: {} MB", pmc.PrivateUsage / 1048576), Vector2(1, 15), Vector4(1, 0, 0, 1), 15.0f);
	Debug::Print(std::format("Total Virtual Memory: {} MB", statex.ullTotalVirtual / 1048576), Vector2(1, 18), Vector4(1, 0, 0, 1), 15.0f);
	Debug::Print(std::format("Percentage Memory Used: {:.5f}%", (float)pmc.WorkingSetSize / statex.ullTotalPhys), Vector2(1, 21), Vector4(0, 1, 0, 1), 15.0f);
	Debug::Print(std::format("Percentage CPU Used: {:.2f}%", percent), Vector2(1, 24), Vector4(0, 0, 1, 1), 15.0f);
	Debug::Print("Key Function Time:", Vector2(1, 40), Vector4(1, 1, 1, 1), 12.5f);
	Debug::Print(std::format("UpdateObjects: {:.2f}ms", mUpdateObjectsTime), Vector2(1, 43), Vector4(1, 1, 1, 1), 12.5f);
	Debug::Print(std::format("Render: {:.2f}ms", mRenderTime), Vector2(1, 46), Vector4(1, 1, 1, 1), 12.5f);
	Debug::Print(std::format("Update World: {:.2f}ms", mWorldTime), Vector2(1, 49), Vector4(1, 1, 1, 1), 12.5f);
	Debug::Print(std::format("Physics Update: {:.2f}ms", mPhysicsTime), Vector2(1, 52), Vector4(1, 1, 1, 1), 12.5f);
	Debug::Print(std::format("Animation Update: {:.2f}ms", mAnimationTime), Vector2(1, 55), Vector4(1, 1, 1, 1), 12.5f);
	Debug::Print(std::format("Position: {:.1f}, {:.1f}, {:.1f}", mTempPlayer->GetTransform().GetPosition().x, mTempPlayer->GetTransform().GetPosition().y,
		mTempPlayer->GetTransform().GetPosition().z), Vector2(30, 3), Vector4(1, 1, 1, 1), 15.0f);

	if (mShowVolumes) {
		std::vector<GameObject*>::const_iterator first;
		std::vector<GameObject*>::const_iterator last;
		mWorld->GetObjectIterators(first, last);
		for (auto i = first; i != last; i++) {
			if ((*i)->GetName() == "Floor" || (*i)->GetName() == "Wall") continue;
			(*i)->DrawCollisionVolume();
		}
	}
#endif
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
			AddFloorToWorld(offsetKey, false);
			break;
		case CornerWall:
			AddCornerWallToWorld(offsetKey);
			break;
		case OutsideFloor:
			AddFloorToWorld(offsetKey, true);
			break;
		}
	}
}

void LevelManager::LoadLights(const std::vector<Light*>& lights, const Vector3& centre, int rotation) {
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
}

void LevelManager::LoadGuards(int guardCount, bool isInMultiplayer, std::mt19937 seed) {
	std::uniform_int_distribution<> dis(0, (*mLevelList[mActiveLevel]).GetGuardPaths().size() - 1);
	std::set<int> pickedNumbers;
	for (int i = 0; i < guardCount; i++) {
		int pathIndex;
		do {
			pathIndex = dis(seed);
		} while (pickedNumbers.count(pathIndex) > 0);
		pickedNumbers.insert(pathIndex);
		GuardObject* addedGuard = AddGuardToWorld((*mLevelList[mActiveLevel]).GetGuardPaths()[pathIndex], (*mLevelList[mActiveLevel]).GetPrisonPosition(), "Guard", isInMultiplayer);
		mGuardObjects.push_back(addedGuard);
	}
}

void LevelManager::LoadItems(const std::vector<Vector3>& itemPositions, const std::vector<Vector3>& roomItemPositions, const bool& isMultiplayer, std::mt19937 seed) {
	for (int i = 0; i < itemPositions.size(); i++) {
		AddPickupToWorld(itemPositions[i], mInventoryBuffSystemClassPtr, isMultiplayer);
	}

	std::uniform_int_distribution<> dis(0, roomItemPositions.size() - 1);
	std::uniform_int_distribution<> dis2(0, 1);
	int flagItem = dis(seed);
	for (int i = 0; i < roomItemPositions.size(); i++) {
		if (i == flagItem) {
			mMainFlag = AddFlagToWorld(roomItemPositions[i], mInventoryBuffSystemClassPtr,mSuspicionSystemClassPtr,seed,isMultiplayer);
			continue;
		}
		if (!isMultiplayer) {
			int itemRand = dis2(seed);
			switch (itemRand) {
			case 0:
				AddPickupToWorld(roomItemPositions[i], mInventoryBuffSystemClassPtr, isMultiplayer);
				break;
			case 1:
				AddPointObjectToWorld(roomItemPositions[i]);
				break;
			}
		}
		else {
			AddPickupToWorld(roomItemPositions[i], mInventoryBuffSystemClassPtr, isMultiplayer);
		}
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

void LevelManager::LoadDecorations(const std::unordered_map<DecorationType, std::vector<Transform>>& decorationMap, const Vector3& startPosition, int rotation) {
	for (auto const& [key, val] : decorationMap) {
		for (int i = 0; i < val.size(); i++) {
			Transform offsetKey = Transform();
			offsetKey.SetPosition(val[i].GetPosition()).SetOrientation(val[i].GetOrientation());
			offsetKey.SetMatrix(Matrix4::Rotation(rotation, Vector3(0, -1, 0)) * offsetKey.GetMatrix());
			offsetKey.SetPosition(offsetKey.GetPosition() + startPosition);
			switch (key) {
			case Desk:
				offsetKey.SetScale((offsetKey.GetOrientation() * Vector3(14, 4, 7.5f)).Abs());
				mLevelLayout.push_back(AddDecorationToWorld(offsetKey, "HotelDesk"));
				break;
			case Painting:
				offsetKey.SetScale((offsetKey.GetOrientation() * Vector3(2.85f, 4.2f, 0.1f)).Abs());
				AddDecorationToWorld(offsetKey, "Painting");
				break;
			case PlantTall:
				offsetKey.SetScale(Vector3(2, 6, 2));
				mLevelLayout.push_back(AddDecorationToWorld(offsetKey, "TallPlant"));
				break;
			case PlantPot:
				offsetKey.SetScale(Vector3(1.5f, 1.5f, 1.5f));
				mLevelLayout.push_back(AddDecorationToWorld(offsetKey, "SmallPlant"));
				break;
			case Bookshelf:
				offsetKey.SetScale((offsetKey.GetOrientation() * Vector3(6.5f, 8.6f, 2)).Abs());
				mLevelLayout.push_back(AddDecorationToWorld(offsetKey, "Bookshelf"));
				break;
			case Bed:
				offsetKey.SetScale((offsetKey.GetOrientation() * Vector3(7, 4, 8.6f)).Abs());
				mLevelLayout.push_back(AddDecorationToWorld(offsetKey, "Bed"));
				break;
			case Chair:
				offsetKey.SetScale(Vector3(2, 3, 2));
				mLevelLayout.push_back(AddDecorationToWorld(offsetKey, "Chair"));
				break;
			case CeilingLight:
				offsetKey.SetScale(Vector3(1.5f, 0.45f, 1.5f));
				AddDecorationToWorld(offsetKey, "CeilingLight");
				break;
			case TV:
				offsetKey.SetScale((offsetKey.GetOrientation() * Vector3(6, 4, 2.8f)).Abs());
				mLevelLayout.push_back(AddDecorationToWorld(offsetKey, "TV"));
				break;
			case Table: // To-Do (Alex): Material
				offsetKey.SetScale((offsetKey.GetOrientation() * Vector3(6.5f, 2, 3.5f)).Abs());
				mLevelLayout.push_back(AddDecorationToWorld(offsetKey, "Table"));
				break;
			case TableSmall: // To-Do (Alex): Material
				offsetKey.SetScale((offsetKey.GetOrientation() * Vector3(3.35f, 2, 3.35f)).Abs());
				mLevelLayout.push_back(AddDecorationToWorld(offsetKey, "SmallTable"));
				break;
			case Shelf:
				offsetKey.SetScale((offsetKey.GetOrientation() * Vector3(6.6f, 1.5f, 1.6f)).Abs());
				mLevelLayout.push_back(AddDecorationToWorld(offsetKey, "Shelf"));
				break;
			case Sofa:
				offsetKey.SetScale((offsetKey.GetOrientation() * Vector3(7, 3, 3)).Abs());
				mLevelLayout.push_back(AddDecorationToWorld(offsetKey, "Sofa"));
				break;
			case PoolTable: 
				offsetKey.SetScale((offsetKey.GetOrientation() * Vector3(7, 4, 12)).Abs());
				mLevelLayout.push_back(AddDecorationToWorld(offsetKey, "PoolTable"));
				break;
			}
		}
	}
}

void LevelManager::LoadCCTVs(std::mt19937 seed, const bool isMultiplayerLevel) {
	std::shuffle(mCCTVTransformList.begin(), mCCTVTransformList.end(), seed);
	for (int i = 0; i < (*mLevelList[mActiveLevel]).GetCCTVCount(); i++) {
		AddCCTVToWorld(mCCTVTransformList[i], isMultiplayerLevel);
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
			if (serverPlayersPtr->at(i) != nullptr) {
				guard->AddPlayer(serverPlayersPtr->at(i));
			}
		}
	}
#endif
}

void LevelManager::InitAnimationSystemObjects() const {
	mAnimation->SetGameObjectLists(mUpdatableObjects);
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

float NCL::CSC8503::LevelManager::GetNearestGuardDistance(const Vector3& startPos) const {
	GuardObject& firstGuard = *mGuardObjects[0];
	GuardObject* returnObj = &firstGuard;
	const Vector3& firstPos = firstGuard.GetTransform().GetPosition();
	float minDistance = sqrt((startPos.x - firstPos.x) * (startPos.x - firstPos.x) +
		(startPos.z - firstPos.z) * (startPos.z - firstPos.z));

	for (int i = 1; i < mGuardObjects.size(); i++) {
		GuardObject* serverGuard = mGuardObjects[i];
		if (serverGuard != nullptr) {
			const Vector3& guardPos = serverGuard->GetTransform().GetPosition();

			float distance = sqrt((startPos.x - guardPos.x) * (startPos.x - guardPos.x) +
				(startPos.z - guardPos.z) * (startPos.z - guardPos.z));
			if (distance < minDistance) {
				minDistance = distance;
				returnObj = serverGuard;
			}
		}

	}

	return minDistance;
}

float LevelManager::GetNearestGuardToPlayerDistance(const int playerNo) const {
	Vector3 playerPos;
#ifdef USEGL
	if (serverPlayersPtr) {
		if ((*serverPlayersPtr)[playerNo] == nullptr) return -1;
		playerPos = (*serverPlayersPtr)[playerNo]->GetTransform().GetPosition();
	}
	else
#endif
		playerPos = mTempPlayer->GetTransform().GetPosition();

	return GetNearestGuardDistance(playerPos);
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

	UISystem::Icon* mCross = mUi->AddIcon(Vector2(48, 50), 3, 5, mTextures["Cross"], 0.0);
	mUi->SetEquippedItemIcon(CROSS, *mCross);

	UISystem::Icon* mAlarm = mUi->AddIcon(Vector2(0, 0), 100, 100, mTextures["Alarm"], 0.0);
	mUi->SetEquippedItemIcon(ALARM, *mAlarm);

	UISystem::Icon* mNoticeRight = mUi->AddIcon(Vector2(52, 50), 8, 6, mTextures["OpenDoor"], 0.0);
	mUi->SetEquippedItemIcon(NOTICERIGHT, *mNoticeRight);

	UISystem::Icon* mNoticeLeft = mUi->AddIcon(Vector2(39, 50), 8, 6, mTextures["CloseDoor"], 0.0);
	mUi->SetEquippedItemIcon(NOTICELEFT, *mNoticeLeft);

	UISystem::Icon* mNoticeTop = mUi->AddIcon(Vector2(45, 43), 8, 6, mTextures["LockDoor"], 0.0);
	mUi->SetEquippedItemIcon(NOTICETOP, *mNoticeTop);

	UISystem::Icon* mNoticeBot = mUi->AddIcon(Vector2(45, 58), 8, 6, mTextures["UnLockDoor"], 0.0);
	mUi->SetEquippedItemIcon(NOTICEBOT, *mNoticeBot);
  
	UISystem::Icon* mNoticeBotLeft = mUi->AddIcon(Vector2(39, 58), 8, 6, mTextures["UseScrewDriver"], 0.0);
	mUi->SetEquippedItemIcon(NOTICEBOTLEFT, *mNoticeBotLeft);

	UISystem::Icon* mNoticeBotRight = mUi->AddIcon(Vector2(52, 58), 8, 6, mTextures["UnLockDoor"], 0.0);
	mUi->SetEquippedItemIcon(NOTICEBOTRIGHT, *mNoticeBotRight);

	UISystem::Icon* mNoticeTopRight = mUi->AddIcon(Vector2(52, 43), 8, 6, mTextures["HoldE"], 0.0);
	mUi->SetEquippedItemIcon(NOTICETOPRIGHT, *mNoticeTopRight);
  
	mRenderer->SetUIObject(mUi);

	mItemTextureMap = {
		{PlayerInventory::item::none, mTextures["InventorySlot"]},
		{PlayerInventory::item::disguise, mTextures["Disguise"]},
		{PlayerInventory::item::soundEmitter,  mTextures["BoomBox"]},
		{PlayerInventory::item::doorKey,  mTextures["KeyIcon3"]},
		{PlayerInventory::item::flag , mTextures["FlagIcon"]},
		{PlayerInventory::item::stunItem, mTextures["Stun"]},
		{PlayerInventory::item::screwdriver, mTextures["ScrewDriver"]}
	};
}

void LevelManager::InitialiseMiniMap() {
#ifdef USEGL
    mMiniMap = new MiniMap(mRenderer);
#endif
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

	mInstanceMatrices["StraightWall"].push_back(wall->GetTransform().GetMatrix());

	if (mBaseObjects.find("StraightWall") == mBaseObjects.end()) mBaseObjects["StraightWall"] = wall;

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

	mInstanceMatrices["CornerWall"].push_back(wall->GetTransform().GetMatrix());

	if (mBaseObjects.find("CornerWall") == mBaseObjects.end()) mBaseObjects["CornerWall"] = wall;

	return wall;
}

GameObject* LevelManager::AddFloorToWorld(const Transform& transform, bool isOutside) {
	GameObject* floor = new GameObject(StaticObj, "Floor");

	Vector3 floorSize = Vector3(4.5f, 0.5f, 4.5f);
	AABBVolume* volume = new AABBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(floorSize * 2)
		.SetPosition(transform.GetPosition())
		.SetOrientation(transform.GetOrientation());

	if (!isOutside) {
		floor->SetRenderObject(new RenderObject(&floor->GetTransform(), mMeshes["Floor"], mTextures["CarpetAlbedo"], mTextures["CarpetNormal"], mShaders["Instance"],
			std::sqrt(std::pow(floorSize.x, 2) + std::powf(floorSize.z, 2))));
		mInstanceMatrices["Floor"].push_back(floor->GetTransform().GetMatrix());
		if (mBaseObjects.find("Floor") == mBaseObjects.end()) mBaseObjects["Floor"] = floor;
	}
	else if(transform.GetPosition().y < 0) {
		floor->SetRenderObject(new RenderObject(&floor->GetTransform(), mMeshes["OutsideFloor"], mTextures["PavementAlbedo"], mTextures["PavementNormal"], mShaders["Instance"],
			std::sqrt(std::pow(floorSize.x, 2) + std::powf(floorSize.z, 2))));
		mInstanceMatrices["OutsideFloor"].push_back(floor->GetTransform().GetMatrix());
		if (mBaseObjects.find("OutsideFloor") == mBaseObjects.end()) mBaseObjects["OutsideFloor"] = floor;
	}
	else {
		floor->SetRenderObject(new RenderObject(&floor->GetTransform(), mMeshes["Ceiling"], mTextures["FloorAlbedo"], mTextures["FloorNormal"], mShaders["Instance"],
			std::sqrt(std::pow(floorSize.x, 2) + std::powf(floorSize.z, 2))));
		mInstanceMatrices["Ceiling"].push_back(floor->GetTransform().GetMatrix());
		if (mBaseObjects.find("Ceiling") == mBaseObjects.end()) mBaseObjects["Ceiling"] = floor;
	}
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume(), 0, 2, 2));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	floor->GetRenderObject()->SetColour(Vector4(0.2f, 0.2f, 0.2f, 1));

	floor->GetRenderObject()->SetIsInstanced(true);

	mWorld->AddGameObject(floor);

	if (transform.GetPosition().y < 0) mLevelLayout.push_back(floor);

	return floor;
}

CCTV* LevelManager::AddCCTVToWorld(const Transform& transform, const bool isMultiplayerLevel) {
	CCTV* camera = new CCTV(25, mWorld);

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

	camera->GenerateViewPyramid();
	camera->SetInitAngle(transform.GetOrientation().ToEuler().y);
#ifdef USEGL
	camera->SetSoundObject(new SoundObject(mSoundManager->AddCCTVSpotSound()));
#endif

	if (!isMultiplayerLevel){
		camera->SetPlayerObjectPtr(mTempPlayer);
	}
	mUpdatableObjects.push_back(camera);
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

	helipad->SetRenderObject(new RenderObject(&helipad->GetTransform(), mMeshes["Helipad"], mTextures["HelipadAlbedo"], mTextures["FloorNormal"], mShaders["Basic"],
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

	Vector3 size = (vent->GetTransform().GetOrientation() * Vector3(1, 1, 0.05f)).Abs();
	AABBVolume* volume = new AABBVolume(size);

	newVent->SetBoundingVolume((CollisionVolume*)volume);

	newVent->GetTransform()
		.SetPosition(vent->GetTransform().GetPosition() + Vector3(0, 0.5f, 0))
		.SetOrientation(vent->GetTransform().GetOrientation())
		.SetScale(Vector3(1, 1, 1));

	newVent->SetRenderObject(new RenderObject(&newVent->GetTransform(), mMeshes["Vent"], mTextures["VentAlbedo"], mTextures["VentNormal"], mShaders["Basic"],
		std::sqrt(std::pow(size.x, 2) + std::powf(size.y, 2))));
	newVent->SetPhysicsObject(new PhysicsObject(&newVent->GetTransform(), newVent->GetBoundingVolume(), 1, 1, 5));
#ifdef USEGL
	newVent->SetSoundObject(new SoundObject());
#endif

	newVent->GetPhysicsObject()->SetInverseMass(0);
	newVent->GetPhysicsObject()->InitCubeInertia();

	newVent->SetCollisionLayer(StaticObj);

	if (isMultiplayerLevel) {
		AddNetworkObject(*newVent);
	}

	mUpdatableObjects.push_back(newVent);
	mWorld->AddGameObject(newVent);

	return newVent;
}

InteractableDoor* LevelManager::AddDoorToWorld(const Transform& transform, const Vector3& offset, bool isMultiplayerLevel) {
	InteractableDoor* newDoor = new InteractableDoor();

	AABBVolume* volume = new AABBVolume(transform.GetScale() / 2);
	newDoor->SetBoundingVolume((CollisionVolume*)volume);

	newDoor->GetTransform()
		.SetPosition(transform.GetPosition() + offset)
		.SetOrientation(transform.GetOrientation())
		.SetScale(Vector3(1, 1, 1));

	newDoor->SetRenderObject(new RenderObject(&newDoor->GetTransform(), mMeshes["Door"], mTextures["DoorAlbedo"], mTextures["DoorNormal"], mShaders["Basic"],
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
	mGlobalSuspicionObserver.push_back(newDoor);
	return newDoor;
}

PrisonDoor* LevelManager::AddPrisonDoorToWorld(PrisonDoor* door, bool isMultiplayerLevel) {
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
		.SetScale(Vector3(1, 1, 1));

	newDoor->SetRenderObject(new RenderObject(&newDoor->GetTransform(), mMeshes["Door"], mTextures["DoorAlbedo"], mTextures["DoorNormal"], mShaders["Basic"],
		std::sqrt(std::pow(size.y, 2) + std::powf(size.z, 2))));
	newDoor->SetPhysicsObject(new PhysicsObject(&newDoor->GetTransform(), newDoor->GetBoundingVolume(), 1, 1, 5));
#ifdef USEGL
	newDoor->SetSoundObject(new SoundObject());
#endif
	newDoor->GetPhysicsObject()->SetInverseMass(0);
	newDoor->GetPhysicsObject()->InitCubeInertia();

	newDoor->GetRenderObject()->SetColour(Vector4(1.0f, 0, 0, 1));

	newDoor->SetCollisionLayer(NoSpecialFeatures);

	if (isMultiplayerLevel) {
		AddNetworkObject(*newDoor);
	}

	mWorld->AddGameObject(newDoor);
	mGlobalSuspicionObserver.push_back(newDoor);

	return newDoor;
}

FlagGameObject* LevelManager::AddFlagToWorld(const Vector3& position, InventoryBuffSystemClass* inventoryBuffSystemClassPtr, SuspicionSystemClass* suspicionSystemClassPtr, 
	std::mt19937 seed,bool isMultiplayerLevel) {
	FlagGameObject* flag = new FlagGameObject(inventoryBuffSystemClassPtr, suspicionSystemClassPtr);

	flag->SetPoints(40);

	Vector3 size = Vector3(0.5f, 0.5f, 0.5f);
	SphereVolume* volume = new SphereVolume(1);
	flag->SetBoundingVolume((CollisionVolume*)volume);
	flag->GetTransform()
		.SetScale(size * 2)
		.SetPosition(position);

	std::uniform_int_distribution<> dis(0, 2);
	int rand = dis(seed);
	switch (rand) {
	case 0:
		flag->SetRenderObject(new RenderObject(&flag->GetTransform(), mMeshes["Chest"], mTextures["ChestAlbedo"], mTextures["ChestNormal"], mShaders["Basic"], 1));
		break;
	case 1:
		flag->SetRenderObject(new RenderObject(&flag->GetTransform(), mMeshes["DragonStatue"], mTextures["DragonStatueAlbedo"], mTextures["DragonStatueNormal"], mShaders["Basic"], 1));
		break;
	case 2:
		flag->SetRenderObject(new RenderObject(&flag->GetTransform(), mMeshes["Diamond"], mTextures["DiamondAlbedo"], mTextures["FloorNormal"], mShaders["Basic"], 1));
		break;
	}
	
	flag->SetPhysicsObject(new PhysicsObject(&flag->GetTransform(), flag->GetBoundingVolume()));
#ifdef USEGL
	flag->SetSoundObject(new SoundObject());
#endif
	flag->SetCollisionLayer(Collectable);

	flag->GetPhysicsObject()->SetInverseMass(0);
	flag->GetPhysicsObject()->InitSphereInertia(false);

	flag->GetRenderObject()->SetColour(Vector4(1.0f, 1.0f, 1.0f, 1));

	mPlayerInventoryObservers.push_back(flag);
	mPlayerBuffsObservers.push_back(flag);
	mWorld->AddGameObject(flag);
	if (isMultiplayerLevel) {
		AddNetworkObject(*flag);
	}
	mUpdatableObjects.push_back(flag);

	return flag;
}

PickupGameObject* LevelManager::AddPickupToWorld(const Vector3& position, InventoryBuffSystemClass* inventoryBuffSystemClassPtr, const bool& isMultiplayer)
{
	PickupGameObject* pickup = new PickupGameObject(inventoryBuffSystemClassPtr, isMultiplayer);

	Vector3 size = Vector3(0.5f, 0.5f, 0.5f);
	SphereVolume* volume = new SphereVolume(1);
	pickup->SetBoundingVolume((CollisionVolume*)volume);
	pickup->GetTransform()
		.SetScale(size * 2)
		.SetPosition(position);
  
	pickup->SetRenderObject(new RenderObject(&pickup->GetTransform(), mMeshes["Toolbox"], mTextures["ToolboxAlbedo"], mTextures["ToolboxNormal"], mShaders["Basic"], 1));
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

	Vector3 size = Vector3(0.5f, 0.5f, 0.5f);
	SphereVolume* volume = new SphereVolume(1);
	pointObject->SetBoundingVolume((CollisionVolume*)volume);
	pointObject->GetTransform()
		.SetScale(size * 2)
		.SetPosition(position);

	pointObject->SetRenderObject(new RenderObject(&pointObject->GetTransform(), mMeshes["Coins"], mTextures["CoinsAlbedo"], mTextures["CoinsNormal"], mShaders["Basic"], 1));
	pointObject->SetPhysicsObject(new PhysicsObject(&pointObject->GetTransform(), pointObject->GetBoundingVolume()));
#ifdef USEGL
	pointObject->SetSoundObject(new SoundObject());
#endif

	pointObject->SetCollisionLayer(Collectable);

	pointObject->GetPhysicsObject()->SetInverseMass(0);
	pointObject->GetPhysicsObject()->InitSphereInertia(false);

	pointObject->GetRenderObject()->SetColour(Vector4(0.0f, 0.4f, 0.2f, 1));

	mWorld->AddGameObject(pointObject);

	mUpdatableObjects.push_back(pointObject);

	return pointObject;
}

PlayerObject* LevelManager::AddPlayerToWorld(const Transform& transform, const std::string& playerName) {
#ifdef USEGL
	mTempPlayer = new PlayerObject(mWorld, mInventoryBuffSystemClassPtr, mSuspicionSystemClassPtr, mUi, new SoundObject(mSoundManager->AddWalkSound()), playerName);
#endif
#ifdef USEPROSPERO#
	mTempPlayer = new PlayerObject(mWorld, mInventoryBuffSystemClassPtr, mSuspicionSystemClassPtr, mUi, playerName);
#endif
	CreatePlayerObjectComponents(*mTempPlayer, transform);
	mWorld->GetMainCamera().SetYaw(transform.GetOrientation().ToEuler().y);

#ifdef USEGL
    mInventoryBuffSystemClassPtr->GetPlayerBuffsPtr()->Attach(mMiniMap);
#endif

	mWorld->AddGameObject(mTempPlayer);
	mUpdatableObjects.push_back(mTempPlayer);
	mPlayerInventoryObservers.push_back(mTempPlayer);
	mPlayerBuffsObservers.push_back(mTempPlayer);

	mTempPlayer->SetIsRendered(false);
	return mTempPlayer;
}

void LevelManager::CreatePlayerObjectComponents(PlayerObject& playerObject, const Vector3& position) {
	CapsuleVolume* volume = new CapsuleVolume(1.4f, 1.0f, Vector3(0,2.f,0));

	playerObject.SetBoundingVolume((CollisionVolume*)volume);
	playerObject.GetTransform()
		.SetScale(Vector3(PLAYER_MESH_SIZE, PLAYER_MESH_SIZE, PLAYER_MESH_SIZE))
		.SetPosition(position);

	playerObject.SetRenderObject(new RenderObject(&playerObject.GetTransform(), mMeshes["Player"], mTextures["FleshyAlbedo"], mTextures["Normal"], mShaders["Animation"],
		PLAYER_MESH_SIZE));
	playerObject.GetRenderObject()->SetAnimationObject(new AnimationObject(AnimationObject::AnimationType::playerAnimation, mAnimations["PlayerStand"]));

	playerObject.SetPhysicsObject(new PhysicsObject(&playerObject.GetTransform(), playerObject.GetBoundingVolume(), 1, 1, 5));
#ifdef USEGL
	playerObject.SetSoundObject(new SoundObject(mSoundManager->AddWalkSound()));
#endif
	playerObject.GetPhysicsObject()->SetInverseMass(PLAYER_INVERSE_MASS);
	playerObject.GetPhysicsObject()->InitSphereInertia(false);

	playerObject.SetCollisionLayer(Player);
}

void LevelManager::CreatePlayerObjectComponents(PlayerObject& playerObject, const Transform& playerTransform) {
	CapsuleVolume* volume = new CapsuleVolume(1.4f, 1.0f, Vector3(0, 2.f, 0));

	playerObject.SetBoundingVolume((CollisionVolume*)volume);

	playerObject.GetTransform()
		.SetScale(Vector3(PLAYER_MESH_SIZE, PLAYER_MESH_SIZE, PLAYER_MESH_SIZE))
		.SetPosition(playerTransform.GetPosition())
		.SetOrientation(playerTransform.GetOrientation());

	playerObject.SetRenderObject(new RenderObject(&playerObject.GetTransform(), mMeshes["Player"], mTextures["FleshyAlbedo"], mTextures["Normal"], mShaders["Animation"],
		PLAYER_MESH_SIZE));
	playerObject.SetPhysicsObject(new PhysicsObject(&playerObject.GetTransform(), playerObject.GetBoundingVolume(), 1, 1, 5));
#ifdef USEGL
	playerObject.SetSoundObject(new SoundObject(mSoundManager->AddWalkSound()));
#endif
	playerObject.GetRenderObject()->SetAnimationObject(new AnimationObject(AnimationObject::AnimationType::playerAnimation, mAnimations["PlayerStand"]));
	playerObject.GetRenderObject()->SetMatTextures(mMeshMaterials["Player_Red"]);


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
			if (mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->ItemInPlayerInventory(PlayerInventory::flag, 0))
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
	for (const auto& guard : mGuardObjects) {
		guard->ApplyBuffToGuard(buffToApply);
	}
}

void LevelManager::AddUpdateableGameObject(GameObject& object) {
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
		.SetPosition(nodes[currentNode] + Vector3(20, -1.5f, 20));


	guard->SetPhysicsObject(new PhysicsObject(&guard->GetTransform(), guard->GetBoundingVolume(), 1, 0, 10));

	guard->GetPhysicsObject()->SetInverseMass(PLAYER_INVERSE_MASS);
	guard->GetPhysicsObject()->InitSphereInertia(false);
	guard->SetCollisionLayer(Npc);

	guard->SetRenderObject(new RenderObject(&guard->GetTransform(), mMeshes["Guard"], mTextures["Basic"], mTextures["Normal"], mShaders["Animation"], meshSize));
	guard->GetRenderObject()->SetAnimationObject(new AnimationObject(AnimationObject::AnimationType::guardAnimation, mAnimations["GuardStand"]));
	guard->GetRenderObject()->SetMatTextures(mMeshMaterials["Guard"]);
#ifdef USEPROSPERO
	guard->GetRenderObject()->SetIgnoredSubmeshID(1);
#endif
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
		guard->AddPlayer(mTempPlayer);
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

	soundEmitterObjectPtr->SetRenderObject(new RenderObject(&soundEmitterObjectPtr->GetTransform(), mMeshes["Toolbox"], mTextures["ToolboxAlbedo"], mTextures["ToolboxNormal"], mShaders["Basic"], 1));

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

GameObject* LevelManager::AddDecorationToWorld(const Transform& transform, const std::string& meshName) {
	GameObject* decoration = new GameObject(StaticObj, meshName);

	Vector3 size = transform.GetScale() / 2;
	AABBVolume* volume = new AABBVolume(size);
	decoration->SetBoundingVolume((CollisionVolume*)volume);
	decoration->GetTransform()
		.SetScale(Vector3(1, 1, 1))
		.SetPosition(transform.GetPosition())
		.SetOrientation(transform.GetOrientation());
	if (mTextures.find(meshName + "Albedo") != mTextures.end()) {
		decoration->SetRenderObject(new RenderObject(&decoration->GetTransform(), mMeshes[meshName], mTextures[meshName + "Albedo"], mTextures[meshName + "Normal"], mShaders["Instance"],
			std::sqrt(std::pow(size.x, 2) + std::powf(size.z, 2))));
	}
	else {
		decoration->SetRenderObject(new RenderObject(&decoration->GetTransform(), mMeshes[meshName], mTextures["FloorAlbedo"], mTextures["Normal"], mShaders["Instance"],
			std::sqrt(std::pow(size.x, 2) + std::powf(size.z, 2))));
	}
	if (mMeshMaterials.find(meshName) != mMeshMaterials.end()) {
		decoration->GetRenderObject()->SetMatTextures(mMeshMaterials[meshName]);
	}
	decoration->SetPhysicsObject(new PhysicsObject(&decoration->GetTransform(), decoration->GetBoundingVolume()));

	decoration->GetPhysicsObject()->SetInverseMass(0);
	decoration->GetPhysicsObject()->InitCubeInertia();

	decoration->GetRenderObject()->SetColour(Vector4(0.2f, 0.2f, 0.2f, 1));

	decoration->GetRenderObject()->SetIsInstanced(true);

	mWorld->AddGameObject(decoration);

	mInstanceMatrices[meshName].push_back(decoration->GetTransform().GetMatrix());

	if (mBaseObjects.find(meshName) == mBaseObjects.end()) mBaseObjects[meshName] = decoration;

	return decoration;
}

FlagGameObject* LevelManager::GetMainFlag() {
	return mMainFlag;
}

Helipad* LevelManager::GetHelipad() {
	return mHelipad;
}
