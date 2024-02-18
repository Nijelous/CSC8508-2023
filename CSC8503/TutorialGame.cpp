#include "TutorialGame.h"
#include "GameWorld.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "AnimationObject.h"
#include "TextureLoader.h"
#include "GuardObject.h"

#include "PositionConstraint.h"
#include "OrientationConstraint.h"
#include "StateGameObject.h"
#include "PlayerObject.h"
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

TutorialGame::TutorialGame(bool isInitingAssets) : controller(*Window::GetWindow()->GetKeyboard(), *Window::GetWindow()->GetMouse()) {
	world		= new GameWorld();
#ifdef USEVULKAN
	renderer	= new GameTechVulkanRenderer(*world);
	renderer->Init();
	renderer->InitStructures();
#else 
	renderer = new GameTechRenderer(*world);
#endif

	physics		= new PhysicsSystem(*world);
	mAnimation   = new AnimationSystem(*world);

	//mLevelManager = new LevelManager();

	forceMagnitude	= 10.0f;
	useGravity		= true;
	physics->UseGravity(useGravity);
	inSelectionMode = false;

	world->GetMainCamera().SetController(controller);

	controller.MapAxis(0, "Sidestep");
	controller.MapAxis(1, "UpDown");
	controller.MapAxis(2, "Forward");

	controller.MapAxis(3, "XLook");
	controller.MapAxis(4, "YLook");

	if (isInitingAssets){
		InitialiseAssets();
	}
}

/*

Each of the little demo scenarios used in the game uses the same 2 meshes, 
and the same texture and shader. There's no need to ever load in anything else
for this module, even in the coursework, but you can add it if you like!

*/
void TutorialGame::InitialiseAssets() {

	LoadAssetFiles();
	InitCamera();
	InitWorld();
}

void TutorialGame::LoadAssetFiles(){
	cubeMesh	= renderer->LoadMesh("cube.msh");
	sphereMesh	= renderer->LoadMesh("sphere.msh");
	capsuleMesh = renderer->LoadMesh("Capsule.msh");
	charMesh	= renderer->LoadMesh("goat.msh");
	enemyMesh	= renderer->LoadMesh("Keeper.msh");
	bonusMesh	= renderer->LoadMesh("apple.msh");
	capsuleMesh = renderer->LoadMesh("capsule.msh");

	basicTex	= renderer->LoadTexture("checkerboard.png");
	mKeeperAlbedo = renderer->LoadTexture("fleshy_albedo.png");
	mKeeperNormal = renderer->LoadTexture("fleshy_normal.png");
	mFloorAlbedo = renderer->LoadTexture("panel_albedo.png");
	mFloorNormal = renderer->LoadTexture("panel_normal.png");
	
	basicShader = renderer->LoadShader("scene.vert", "scene.frag");
	mAnimationShader = renderer->LoadShader("animationScene.vert", "scene.frag");

	mSoldierMesh = renderer->LoadMesh("Role_T.msh");
	mSoldierAnimation = renderer->LoadAnimation("Role_T.anm");
	mSoldierMaterial = renderer->LoadMaterial("Role_T.mat");

	
	mGuardMesh = renderer->LoadMesh("Male_Guard.msh");
	mGuardAnimation = renderer->LoadAnimation("Idle1.anm");
	mGuardMaterial = renderer->LoadMaterial("Male_Guard.mat");

	InitCamera();
	InitWorld();
	mAnimation->PreloadMatTextures(renderer);

}

TutorialGame::~TutorialGame()	{
	delete cubeMesh;
	delete sphereMesh;
	delete capsuleMesh;
	delete charMesh;
	delete enemyMesh;
	delete bonusMesh;

	delete mAnimationShader;
	delete mSoldierAnimation;
	delete mSoldierMaterial;
	delete mSoldierMesh;
	
	delete mGuardAnimation;
	delete mGuardMaterial;
	delete mGuardMesh;

	delete basicTex;
	delete basicShader;
	delete mKeeperAlbedo;
	delete mKeeperNormal;
	delete mFloorAlbedo;
	delete mFloorNormal;

	delete physics;
	delete renderer;
	delete world;
	delete mAnimation;
}

void TutorialGame::UpdateGame(float dt) {
	if (testSphere != nullptr){
		//testSphere->GetPhysicsObject()->AddForce(Vector3(1,0,1));
	}
	if (!inSelectionMode) {
		world->GetMainCamera().UpdateCamera(dt);
	}
	if (lockedObject != nullptr) {
		Vector3 objPos = lockedObject->GetTransform().GetPosition();
		Vector3 camPos = objPos + lockedOffset;

		Matrix4 temp = Matrix4::BuildViewMatrix(camPos, objPos, Vector3(0,1,0));

		Matrix4 modelMat = temp.Inverse();

		Quaternion q(modelMat);
		Vector3 angles = q.ToEuler(); //nearly there now!

		world->GetMainCamera().SetPosition(camPos);
		world->GetMainCamera().SetPitch(angles.x);
		world->GetMainCamera().SetYaw(angles.y);
	}

	tempPlayer->UpdateObject(dt);

	UpdateKeys();
	if (useGravity) {
		Debug::Print("(G)ravity on", Vector2(5, 95), Debug::RED);		
	}
	else {
		Debug::Print("(G)ravity off", Vector2(5, 95), Debug::RED);
	}

	RayCollision closestCollision;
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::K) && selectionObject) {
		Vector3 rayPos;
		Vector3 rayDir;

		rayDir = selectionObject->GetTransform().GetOrientation() * Vector3(0, 0, -1);

		rayPos = selectionObject->GetTransform().GetPosition();

		Ray r = Ray(rayPos, rayDir);

		if (world->Raycast(r, closestCollision, true, selectionObject)) {
			if (objClosest) {
				objClosest->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
			}
			objClosest = (GameObject*)closestCollision.node;

			objClosest->GetRenderObject()->SetColour(Vector4(1, 0, 1, 1));
		}
	}
	// draw Axis
	/*Debug::DrawLine(Vector3(), Vector3(0, 100, 0), Debug::RED);
	Debug::DrawLine(Vector3(), Vector3(100, 0, 0), Debug::BLUE);
	Debug::DrawLine(Vector3(), Vector3(0, 0, 100), Debug::GREEN);*/
	SelectObject();
	MoveSelectedObject();

	if (testStateObject)
		testStateObject->Update(dt);

	if (mGameObjects.size() > 0) {
		for (int i = 0; i < mGameObjects.size(); i++) {
			mGameObjects[i]->UpdateObject(dt);
		}
	}

	world->UpdateWorld(dt);
	mAnimation->Update(dt);
	renderer->Update(dt);
	physics->Update(dt);
	renderer->Render();
	Debug::UpdateRenderables(dt);
}

GameWorld* TutorialGame::GetGameWorld() const{
	return world;
}

void TutorialGame::UpdateKeys() {
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F1)) {
		InitWorld(); //We can reset the simulation at any time with F1
		selectionObject = nullptr;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F2)) {
		InitCamera(); //F2 will reset the camera to a specific default place
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::G)) {
		useGravity = !useGravity; //Toggle gravity!
		physics->UseGravity(useGravity);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::P)) {
		ISoundEngine* engine = createIrrKlangDevice();
		engine->play2D("../Assets/Sound/ophelia.mp3", true);
	}
	//Running certain physics updates in a consistent order might cause some
	//bias in the calculations - the same objects might keep 'winning' the constraint
	//allowing the other one to stretch too much etc. Shuffling the order so that it
	//is random every frame can help reduce such bias.
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F9)) {
		world->ShuffleConstraints(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F10)) {
		world->ShuffleConstraints(false);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F7)) {
		world->ShuffleObjects(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F8)) {
		world->ShuffleObjects(false);
	}

	if (lockedObject) {
		LockedObjectMovement();
	}
	else {
		DebugObjectMovement();
	}
}

void TutorialGame::LockedObjectMovement() {
	Matrix4 view		= world->GetMainCamera().BuildViewMatrix();
	Matrix4 camWorld	= view.Inverse();

	Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); //view is inverse of model!

	//forward is more tricky -  camera forward is 'into' the screen...
	//so we can take a guess, and use the cross of straight up, and
	//the right axis, to hopefully get a vector that's good enough!

	Vector3 fwdAxis = Vector3::Cross(Vector3(0, 1, 0), rightAxis);
	fwdAxis.y = 5.0f;
	fwdAxis.Normalise();


	if (Window::GetKeyboard()->KeyDown(KeyCodes::UP)) {
		selectionObject->GetPhysicsObject()->AddForce(fwdAxis);
	}

	if (Window::GetKeyboard()->KeyDown(KeyCodes::DOWN)) {
		selectionObject->GetPhysicsObject()->AddForce(-fwdAxis);
	}

	if (Window::GetKeyboard()->KeyDown(KeyCodes::NEXT)) {
		selectionObject->GetPhysicsObject()->AddForce(Vector3(0,-10,0));
	}
}

void TutorialGame::CreatePlayerObjectComponents(PlayerObject& playerObject,  const Vector3& position) const{
	CapsuleVolume* volume  = new CapsuleVolume(1.4f, 1.0f);

	playerObject.SetBoundingVolume((CollisionVolume*)volume);

	playerObject.GetTransform()
		.SetScale(Vector3(PLAYER_MESH_SIZE, PLAYER_MESH_SIZE, PLAYER_MESH_SIZE))
		.SetPosition(position);

	playerObject.SetRenderObject(new RenderObject(&playerObject.GetTransform(), enemyMesh, mKeeperAlbedo, mKeeperNormal, basicShader, PLAYER_MESH_SIZE));
	playerObject.SetPhysicsObject(new PhysicsObject(&playerObject.GetTransform(), playerObject.GetBoundingVolume(), 1, 1, 5));


	playerObject.GetPhysicsObject()->SetInverseMass(PLAYER_INVERSE_MASS);
	playerObject.GetPhysicsObject()->InitSphereInertia(false);
}

void TutorialGame::DebugObjectMovement() {
//If we've selected an object, we can manipulate it with some key presses
	if (inSelectionMode && selectionObject) {
		//Twist the selected object!
		if (Window::GetKeyboard()->KeyDown(KeyCodes::LEFT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(-10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::NUM7)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, 10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::NUM8)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, -10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::UP)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, -10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::DOWN)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, 10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::NUM5)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, -10, 0));
		}
	}
}

void TutorialGame::InitCamera() {
	world->GetMainCamera().SetNearPlane(0.1f);
	world->GetMainCamera().SetFarPlane(500.0f);
	world->GetMainCamera().SetPitch(-15.0f);
	world->GetMainCamera().SetYaw(315.0f);
	world->GetMainCamera().SetPosition(Vector3(-60, 40, 60));
	lockedObject = nullptr;
}

void TutorialGame::InitWorld() {
	world->ClearAndErase();
	physics->Clear();

	//mLevelManager->LoadLevel(0, world, cubeMesh, mFloorAlbedo, mFloorNormal, basicShader);

	//AddPlayerToWorld(mLevelManager->GetPlayerStartPosition(0), "Player");

	AddPlayerToWorld(Vector3(100,-17,100), "Player");
  
	AddGuardToWorld(Vector3(90, -17, 90), "Enemy");

	testSphere = AddSphereToWorld(Vector3(40,-17,40), 1.0f, true);

	AddAABBCubeToWorld(Vector3(0,0,0), Vector3(10,20,10), 0.0f, "Wall");

	AddGuardToWorld(Vector3(30, -17, 5), "Guard Object");


	InitDefaultFloor();
	AddAnimationTest(Vector3(50, 0, 50), "test");
}

/*

A single function to add a large immoveable cube to the bottom of our world

*/
GameObject* TutorialGame::AddFloorToWorld(const Vector3& position, const std::string& objectName) {
	GameObject* floor = new GameObject(StaticObj, objectName);

	Vector3 floorSize = Vector3(120, 2, 120);
	AABBVolume* volume = new AABBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(floorSize * 2)
		.SetPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, mFloorAlbedo, mFloorNormal, basicShader, 120));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume(), 1, 2, 2));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	floor->GetRenderObject()->SetColour(Vector4(0.2f, 0.2f, 0.2f, 1));

	world->AddGameObject(floor);

	return floor;
}

/*

Builds a game object that uses a sphere mesh for its graphics, and a bounding sphere for its
rigid body representation. This and the cube function will let you build a lot of 'simple' 
physics worlds. You'll probably need another function for the creation of OBB cubes too.

*/
GameObject* TutorialGame::AddSphereToWorld(const Vector3& position, float radius, bool applyPhysics, float inverseMass, const std::string& objectName) {
	GameObject* sphere = new GameObject();

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius, applyPhysics);
	sphere->SetBoundingVolume((CollisionVolume*)volume);

	sphere->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, mFloorAlbedo, mFloorNormal, basicShader,radius));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

	sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
	sphere->GetPhysicsObject()->InitSphereInertia(false);

	world->AddGameObject(sphere);

	return sphere;
}

GameObject* TutorialGame::AddCapsuleToWorld(const Vector3& position, float halfHeight, float radius, float inverseMass, const std::string& objectName) {
	GameObject* capsule = new GameObject();

	Vector3 capsuleSize = Vector3(radius * 2, (halfHeight * 2), radius * 2);
	CapsuleVolume* volume = new CapsuleVolume(halfHeight, radius);
	capsule->SetBoundingVolume((CollisionVolume*)volume);

	capsule->GetTransform()
		.SetScale(capsuleSize)
		.SetPosition(position);

	capsule->SetRenderObject(new RenderObject(&capsule->GetTransform(), capsuleMesh, basicTex, nullptr, basicShader, halfHeight));
	capsule->SetPhysicsObject(new PhysicsObject(&capsule->GetTransform(), capsule->GetBoundingVolume()));

	capsule->GetPhysicsObject()->SetInverseMass(inverseMass);
	capsule->GetPhysicsObject()->InitSphereInertia(false);

	world->AddGameObject(capsule);

	return capsule;
}

GameObject* TutorialGame::AddOBBCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass, const std::string& objectName) {
	GameObject* cube = new GameObject();

	OBBVolume* volume = new OBBVolume(dimensions);
	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);
	float largestDim = std::max(dimensions.x, std::max(dimensions.y, dimensions.z));
	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, nullptr, basicShader, largestDim));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));
	
	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();
	
	world->AddGameObject(cube);

	return cube;
}

GameObject* TutorialGame::AddAABBCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass, const std::string& objectName) {
	GameObject* cube = new GameObject();

	AABBVolume* volume = new AABBVolume(dimensions);
	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);
	float largestDim = std::max(dimensions.x, std::max(dimensions.y, dimensions.z));
	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, mFloorAlbedo, mFloorNormal, basicShader, largestDim));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);

	return cube;
}

GameObject* TutorialGame::AddPlayerToWorld(const Vector3& position, const std::string& objectName) {

	tempPlayer = new PlayerObject(world, objectName);
	CreatePlayerObjectComponents(*tempPlayer, position);

	world->AddGameObject(tempPlayer);
	tempPlayer->SetActive();
	return tempPlayer;
}

GameObject* TutorialGame::AddEnemyToWorld(const Vector3& position, const std::string& objectName) {
	float meshSize		= 3.0f;
	float inverseMass	= 0.5f;

	GameObject* character = new GameObject();

	CapsuleVolume* volume = new CapsuleVolume(1.3f, 1.0f);
	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), enemyMesh, mKeeperAlbedo, mKeeperNormal, basicShader, meshSize));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia(false);

	world->AddGameObject(character);

	return character;
}

GameObject* TutorialGame::AddBonusToWorld(const Vector3& position, const std::string& objectName) {
	GameObject* apple = new GameObject();

	SphereVolume* volume = new SphereVolume(0.5f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform()
		.SetScale(Vector3(2, 2, 2))
		.SetPosition(position);

	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), bonusMesh, basicTex, nullptr, mAnimationShader, 0.5f));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetPhysicsObject()->SetInverseMass(1.0f);
	apple->GetPhysicsObject()->InitSphereInertia(false);

	world->AddGameObject(apple);

	return apple;
}

GameObject* TutorialGame::AddAnimationTest(const Vector3& position, const std::string& objectName)
{
	float meshSize = 1.0f;
	float inverseMass = 0.5f;

	GameObject* animTest = new GameObject();
	CapsuleVolume* volume = new CapsuleVolume(1.3f, 1.0f);
	animTest->SetBoundingVolume((CollisionVolume*)volume);

	animTest->GetTransform()
		.SetScale(Vector3(meshSize*2, meshSize * 2, meshSize * 2))
		.SetPosition(position);

	animTest->SetRenderObject(new RenderObject(&animTest->GetTransform(), mGuardMesh, nullptr, nullptr, mAnimationShader, meshSize));
	animTest->SetPhysicsObject(new PhysicsObject(&animTest->GetTransform(), animTest->GetBoundingVolume()));
	
	animTest->SetAnimationObject(new AnimationObject(mGuardAnimation, mGuardMaterial));
	

	animTest->GetPhysicsObject()->SetInverseMass(inverseMass);
	animTest->GetPhysicsObject()->InitSphereInertia(false);

	world->AddGameObject(animTest);

	return animTest;

}

StateGameObject* TutorialGame::AddStateObjectToWorld(const Vector3& position, const std::string& objectName) {
	StateGameObject* apple = new StateGameObject(objectName);

	AABBVolume* volume = new AABBVolume(Vector3(1,1,1));
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform()
		.SetScale(Vector3(2, 2, 2))
		.SetPosition(position);

	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), cubeMesh, basicTex, nullptr, basicShader, 1));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetPhysicsObject()->SetInverseMass(1.0f);
	apple->GetPhysicsObject()->InitSphereInertia(false);

	world->AddGameObject(apple);

	return apple;
}

GuardObject* TutorialGame::AddGuardToWorld(const Vector3& position, const std::string& objectName) {
	//unique_ptr<GuardObject> guard(new GuardObject(objectName));
	GuardObject* guard = new GuardObject(objectName);

	float meshSize = 3.0f;
	float inverseMass = 0.5f;

	CapsuleVolume* volume = new CapsuleVolume(1.3f, 1.0f);
	guard->SetBoundingVolume((CollisionVolume*)volume);

	guard->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	guard->SetRenderObject(new RenderObject(&guard->GetTransform(), enemyMesh, mKeeperAlbedo, mKeeperNormal, basicShader, meshSize));
	guard->SetPhysicsObject(new PhysicsObject(&guard->GetTransform(), guard->GetBoundingVolume(), 1, 0, 5));

	guard->GetPhysicsObject()->SetInverseMass(inverseMass);
	guard->GetPhysicsObject()->InitSphereInertia(false);
	

	guard->SetPlayer(tempPlayer);
	guard->SetGameWorld(world);

	world->AddGameObject(guard);
	mGameObjects.push_back(guard);

	return guard;
}

void TutorialGame::InitDefaultFloor() {
	AddFloorToWorld(Vector3(0, -20, 0), "Floor Object");
}

void TutorialGame::InitGameExamples() {
	AddPlayerToWorld(Vector3(0, 5, 0), "Player Object");
	AddEnemyToWorld(Vector3(5, 5, 0), "Enemy Object");
	AddBonusToWorld(Vector3(10, 5, 0), "Bonus Object");
	AddGuardToWorld(Vector3(10, 5, 5), "Guard Object");
	AddAnimationTest(Vector3(15,15, 0), "Animation Object");
}

void TutorialGame::InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius) {
	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddSphereToWorld(position, radius, true, 1.0f, "Sphere Object " + (x + z));
		}
	}
	AddFloorToWorld(Vector3(0, -2, 0), "Floor Object");
}

void TutorialGame::InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing) {
	float sphereRadius = 1.0f;
	float halfHeight = 0.5f;
	Vector3 cubeDims = Vector3(1, 1, 1);

	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);

			if (rand() % 2) {
				AddAABBCubeToWorld(position, cubeDims, 1.0f, "Cube Object");
				//AddCapsuleToWorld(position, halfHeight, sphereRadius, 2.0f, "Capsule Object");
			}
			else {
				AddSphereToWorld(position, sphereRadius,true, 1.0f, "Sphere Object");
				//AddCapsuleToWorld(position, halfHeight, sphereRadius, 2.0f, "Capsule Object");
			}
		}
	}
}

void TutorialGame::InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims) {
	for (int x = 1; x < numCols+1; ++x) {
		for (int z = 1; z < numRows+1; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddOBBCubeToWorld(position, cubeDims, 1.0f, "Cube Object " + (x + z));
		}
	}
}

void TutorialGame::InitOBBAABB() {
	float sphereRadius = 1.0f;
	float halfHeight = 0.5f;
	Vector3 cubeDims = Vector3(1,1,1);

	AddOBBCubeToWorld(Vector3(5,10,0), cubeDims, 1.0f, "Cube Object");
	AddAABBCubeToWorld(Vector3(10,10,0), cubeDims * 2, 1.0f, "Cube Object");
}

void TutorialGame::BridgeConstraintTest(Vector3 startPosition) {
	Vector3 cubeSize = Vector3(8,8,8);

	float invCubeMass = 5;
	int numLinks = 10;
	float maxDistance = 30;
	float maxAngleDiff = 5;
	float cubeDistance = 20;

	Vector3 startPos = startPosition;

	GameObject* start = AddOBBCubeToWorld(startPos + Vector3(0,0,0), cubeSize, 0);
	GameObject* end = AddOBBCubeToWorld(startPos + Vector3((numLinks + 2) * cubeDistance,0,0), cubeSize, 0);

	GameObject* previous = start;

	for (int i = 0; i < numLinks; i++) {
		GameObject* block = AddOBBCubeToWorld(startPos + Vector3((i+1) * cubeDistance, 0, 0), cubeSize, invCubeMass);
		PositionConstraint* constraint = new PositionConstraint(previous, block, maxDistance);
		OrientationConstraint* otherConstraint = new OrientationConstraint(previous, block, maxAngleDiff);
		world->AddConstraint(constraint);
		world->AddConstraint(otherConstraint);
		previous = block;
	}
	PositionConstraint* constraint = new PositionConstraint(previous, end, maxDistance);
	OrientationConstraint* otherConstraint = new OrientationConstraint(previous, end, maxAngleDiff);
	world->AddConstraint(constraint);
	world->AddConstraint(otherConstraint);
}

/*
Every frame, this code will let you perform a raycast, to see if there's an object
underneath the cursor, and if so 'select it' into a pointer, so that it can be 
manipulated later. Pressing Q will let you toggle between this behaviour and instead
letting you move the camera around. 

*/
bool TutorialGame::SelectObject() {
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::Q)) {
		inSelectionMode = !inSelectionMode;
		if (inSelectionMode) {
			Window::GetWindow()->ShowOSPointer(true);
			Window::GetWindow()->LockMouseToWindow(false);
		}
		else {
			Window::GetWindow()->ShowOSPointer(false);
			Window::GetWindow()->LockMouseToWindow(true);
		}
	}
	if (inSelectionMode) {
		Debug::Print("Press Q to change to camera mode!", Vector2(5, 85));

		if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::Left)) {
			if (selectionObject) {	//set colour to deselected;
				selectionObject->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
				selectionObject = nullptr;
			}

			Ray ray = CollisionDetection::BuildRayFromMouse(world->GetMainCamera());

			RayCollision closestCollision;
			if (world->Raycast(ray, closestCollision, true)) {
				selectionObject = (GameObject*)closestCollision.node;

				selectionObject->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
				return true;
			}
			else {
				return false;
			}
		}
		if (Window::GetKeyboard()->KeyPressed(NCL::KeyCodes::L)) {
			if (selectionObject) {
				if (lockedObject == selectionObject) {
					lockedObject = nullptr;
				}
				else {
					lockedObject = selectionObject;
				}
			}
		}
	}
	else {
		Debug::Print("Press Q to change to select mode!", Vector2(5, 85));
	}
	return false;
}

/*
If an object has been clicked, it can be pushed with the right mouse button, by an amount
determined by the scroll wheel. In the first tutorial this won't do anything, as we haven't
added linear motion into our physics system. After the second tutorial, objects will move in a straight
line - after the third, they'll be able to twist under torque aswell.
*/

void TutorialGame::MoveSelectedObject() {
	Debug::Print("Click Force:" + std::to_string(forceMagnitude), Vector2(5, 90));
	forceMagnitude += Window::GetMouse()->GetWheelMovement() * 100.0f;

	if (!selectionObject) {
		return;//we haven't selected anything!
	}
	//Push the selected object!
	if (Window::GetMouse()->ButtonPressed(NCL::MouseButtons::Right)) {
		Ray ray = CollisionDetection::BuildRayFromMouse(world->GetMainCamera());

		RayCollision closestCollision;
		if (world->Raycast(ray, closestCollision, true)) {
			if (closestCollision.node == selectionObject) {
				selectionObject->GetPhysicsObject()->AddForceAtPosition(ray.GetDirection() * forceMagnitude, closestCollision.collidedAt);
			}
		}
	}
}