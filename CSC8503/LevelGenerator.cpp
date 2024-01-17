#include "LevelGenerator.h"
#include "Assets.h"
#include "GameWorld.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "TextureLoader.h"

#include "PositionConstraint.h"
#include "OrientationConstraint.h"
#include "StateGameObject.h"

#include <fstream>
#include <cmath>

using namespace NCL;
using namespace CSC8503;

const char WALL_NODE = 'x';
const char FLOOR_NODE = '.';

LevelGenerator::LevelGenerator() {
	InitialiseAssets();
	currentPoints = 0;
	timer = 300.0f;

	playerSpeed = 20.0f;
	enemySpeed = 15.0f;
}

LevelGenerator::~LevelGenerator() {
	delete cubeMesh;
	delete sphereMesh;
	delete capsuleMesh;
	delete charMesh;
	delete enemyMesh;
	delete bonusMesh;

	delete basicTex;
	delete basicShader;

	delete physics;
	delete renderer;
	delete world;

	delete playerObject;
	delete enemyObject;
}

void LevelGenerator::UpdateGame(float dt) {
	NetworkedGame::UpdateGame(dt);
	switch (currentLevel) {
	case(MenuScreen):
		DisplayMainMenu();
		break;

	// physics
	case(PhysicsMenu):
		DisplayPhysicsMenu();
		break;
	case(Physics):
		UpdatePhysicsGame(dt);
		break;

	// pathfinding
	case(PathfindingMenu):
		DisplayPathfindingMenu();
		break;
	case(Pathfinding):
		UpdatePathfindingGame(dt);
		break;

	// networking
	case(NetworkingMenu):
		DisplayNetworkingMenu();
		break;
	case(Networking):
		UpdateNetworkingGame(dt);
		break;

	// win or lose screen
	case(LoseScreen):
		DisplayLoseScreen();
		break;
	case(WinScreen):
		DisplayWinScreen();
		break;
	}
}

void LevelGenerator::UpdatePhysicsGame(float dt) {
	timer = timer - dt;
	DisplayPointsAndItemsLeft();
	CharacterController(dt, playerObject, playerSpeed);
	CheckPlayerHitGoal();

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::LSHIFT)) {
		InitCamera();
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::RSHIFT)) {
		LockCameraToObject(playerObject);
		world->GetMainCamera().SetPosition(playerObject->GetTransform().GetPosition() + lockedOffset);
	}
}

void LevelGenerator::UpdatePathfindingGame(float dt) {
	timer = timer - dt;
	DisplayPointsAndItemsLeft();
	CheckPlayerHitEnemy();
	CheckPlayerHitGoal();
	CharacterController(dt, playerObject, playerSpeed);
	stateMachine->Update(dt);
}

void LevelGenerator::UpdateNetworkingGame(float dt) {
	if (networkRole == Server) {
		CharacterController(dt, player1Object, playerSpeed);
		UpdatePlayer2();
		DisplayPlayer1Data();
	}
	if (networkRole == Client) {
		DisplayPlayer2Data();
	}
	CheckPlayerHitGoal();
}

void LevelGenerator::InitWorld() {
	//GeneratePathfindingLevel();
}

void LevelGenerator::CharacterController(float dt, GameObject* characterControlled, float speed) {
	if (lockedObject) {
		Vector3 cameraPos = world->GetMainCamera().GetPosition();
		Vector3 characterPos = characterControlled->GetTransform().GetPosition();
		Vector3 temp = Vector3(cameraPos.x, characterPos.y, cameraPos.z);

		Vector3 fwdAxis = cameraPos - characterPos;

		Vector3 rightAxis = Vector3::Cross(fwdAxis, Vector3(0,1,0));

		fwdAxis = MakeForceUnfiform(fwdAxis, speed);
		rightAxis = MakeForceUnfiform(rightAxis, speed);

		if (Window::GetKeyboard()->KeyDown(KeyCodes::W)) {
			characterControlled->GetPhysicsObject()->AddForce(-fwdAxis);
		}
		if (Window::GetKeyboard()->KeyDown(KeyCodes::S)) {
			characterControlled->GetPhysicsObject()->AddForce( fwdAxis);
		}
		if (Window::GetKeyboard()->KeyDown(KeyCodes::A)) {
			characterControlled->GetPhysicsObject()->AddForce( rightAxis);
		}
		if (Window::GetKeyboard()->KeyDown(KeyCodes::D)) {
			characterControlled->GetPhysicsObject()->AddForce(-rightAxis);
		}

		RayCollision closestCollision;
		Vector3 rayPos = characterControlled->GetTransform().GetPosition();
		Vector3 rayDirection = Vector3(0,-1,0);
		Ray r = Ray(rayPos, rayDirection);

		if (Window::GetKeyboard()->KeyDown(KeyCodes::SPACE)) {
			if (world->Raycast(r, closestCollision, true, characterControlled)) {
				if ((characterControlled->GetTransform().GetPosition().y - closestCollision.collidedAt.y) <= 1.5f)
					characterControlled->GetPhysicsObject()->AddForce(Vector3(0, 1000, 0));
			}
		}

		if (!inSelectionMode) {
			if (controller.GetAxis(3) > 0.0f)
				RotateCameraAroundPLayer(true, controller.GetAxis(3) * 1, dt);
			if (controller.GetAxis(3) < 0.0f)
				RotateCameraAroundPLayer(false, controller.GetAxis(3) * -1, dt);
		}
	}
	Quaternion rotation = GetObjectRotation(characterControlled->GetTransform().GetPosition(), world->GetMainCamera().GetPosition());
	Quaternion flip = Quaternion(0, 0, 1.0f, 0);
	characterControlled->GetTransform().SetOrientation(rotation * flip);
}

// made using these sources
// https://stackoverflow.com/questions/18558910/direction-vector-to-rotation-matrix
// https://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/
Quaternion LevelGenerator::GetObjectRotation(Vector3 objectPos, Vector3 camPos) {
	Vector3 directionVector = camPos - objectPos;
	directionVector.Normalise();
	directionVector.y = 0;
	Vector3 xAxis = Vector3::Cross(directionVector, Vector3(0, 1, 0));
	xAxis.Normalise();
	Vector3 yAxis = Vector3::Cross(directionVector, xAxis);
	yAxis.Normalise();

	Matrix3 m = Matrix3();
	Vector3 column1 = Vector3(xAxis.x, yAxis.x, directionVector.x);
	Vector3 column2 = Vector3(xAxis.y, yAxis.y, directionVector.y);
	Vector3 column3 = Vector3(xAxis.z, yAxis.z, directionVector.z);
	m.SetColumn(0, column1);
	m.SetColumn(1, column2);
	m.SetColumn(2, column3);

	Quaternion rotation = Quaternion();

	float trace = m.GetColumn(0)[0] + m.GetColumn(1)[1] + m.GetColumn(2)[2];
	if (trace > 0) {
		float s = 0.5f / sqrtf(trace + 1.0f);
		rotation.w = 0.25f / s;
		rotation.x = (m.GetColumn(2)[1] - m.GetColumn(1)[2]) * s;
		rotation.y = (m.GetColumn(0)[2] - m.GetColumn(2)[0]) * s;
		rotation.z = (m.GetColumn(1)[0] - m.GetColumn(0)[1]) * s;
	}
	else {
		if (m.GetColumn(0)[0] > m.GetColumn(1)[1] && m.GetColumn(0)[0] > m.GetColumn(2)[2]) {
			float s = 2.0f * sqrtf(1.0f + m.GetColumn(0)[0] - m.GetColumn(1)[1] - m.GetColumn(2)[2]);
			rotation.w = (m.GetColumn(2)[1] - m.GetColumn(1)[2]) / s;
			rotation.x = 0.25f * s;
			rotation.y = (m.GetColumn(0)[1] + m.GetColumn(1)[0]) / s;
			rotation.z = (m.GetColumn(0)[2] + m.GetColumn(2)[0]) / s;
		}
		else if (m.GetColumn(1)[1] > m.GetColumn(2)[2]) {
			float s = 2.0f * sqrtf(1.0f + m.GetColumn(1)[1] - m.GetColumn(0)[0] - m.GetColumn(2)[2]);
			rotation.w = (m.GetColumn(0)[2] - m.GetColumn(2)[0]) / s;
			rotation.x = (m.GetColumn(0)[1] + m.GetColumn(1)[0]) / s;
			rotation.y = 0.25f * s;
			rotation.z = (m.GetColumn(1)[2] + m.GetColumn(2)[1]) / s;
		}
		else {
			float s = 2.0f * sqrtf(1.0f + m.GetColumn(2)[2] - m.GetColumn(0)[0] - m.GetColumn(1)[1]);
			rotation.w = (m.GetColumn(1)[0] - m.GetColumn(0)[1]) / s;
			rotation.x = (m.GetColumn(0)[2] + m.GetColumn(2)[0]) / s;
			rotation.y = (m.GetColumn(1)[2] + m.GetColumn(2)[1]) / s;
			rotation.z = 0.25f * s;
		}
	}
	return rotation;
}

void LevelGenerator::RotateCameraAroundPLayer(bool clockWise, float speed, float dt) {
	float newX;
	float newZ;
	if (clockWise) {
		newX = (lockedOffset.x * cos(speed * (1 * dt))) - (lockedOffset.z * sin(speed * (1 * dt)));
		newZ = (lockedOffset.z * cos(speed * (1 * dt))) + (lockedOffset.x * sin(speed * (1 * dt)));
	}
	else {
		newX = (lockedOffset.x * cos(speed * (-1 * dt))) - (lockedOffset.z * sin(speed * (-1 * dt)));
		newZ = (lockedOffset.z * cos(speed * (-1 * dt))) + (lockedOffset.x * sin(speed * (-1 * dt)));
	}
	Vector3 newOffset(newX, lockedOffset.y, newZ);
	lockedOffset =  newOffset;
}

void LevelGenerator::InitialiseAssets() {
	cubeMesh = renderer->LoadMesh("cube.msh");
	sphereMesh = renderer->LoadMesh("sphere.msh");
	capsuleMesh = renderer->LoadMesh("Capsule.msh");
	charMesh = renderer->LoadMesh("goat.msh");
	enemyMesh = renderer->LoadMesh("Keeper.msh");
	bonusMesh = renderer->LoadMesh("apple.msh");
	capsuleMesh = renderer->LoadMesh("capsule.msh");

	basicTex = renderer->LoadTexture("checkerboard.png");
	basicShader = renderer->LoadShader("scene.vert", "scene.frag");

	InitCamera();
	LevelGenerator::InitWorld();
}

// generate gameobjects

GameObject* LevelGenerator::GeneratePlayerObject(const Vector3& position, const std::string& objectName) {
	float meshSize = 1.0f;
	float inverseMass = 0.3f;

	GameObject* character = new GameObject(objectName);
	SphereVolume* volume = new SphereVolume(1.0f, true);

	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), charMesh, nullptr, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia(false);

	world->AddGameObject(character);

	return character;
}

GameObject* LevelGenerator::GenerateEnemyObject(const Vector3& position, const std::string& objectName) {
	float meshSize = 3.0f;
	float inverseMass = 0.5f;

	GameObject* character = new GameObject(objectName);

	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.9f, 0.3f) * meshSize);
	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), enemyMesh, nullptr, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia(false);

	world->AddGameObject(character);

	return character;
}

GameObject* LevelGenerator::GenerateGoal(Vector3 position, int level) {
	GameObject* temp = AddSphereToWorld(position, 2.0f, false, 0.0f, "Goal");
	temp->GetRenderObject()->SetColour(Vector4(1, 1, 0, 1));
	switch (level) {
	case(Physics):
		physicsGoals.push_back(temp);
		break;
	case(Pathfinding):
		pathfindingGoals.push_back(temp);
		break;
	case(Networking):
		networkingGoals.push_back(temp);
		break;
	}
	return temp;
}

// physics level functions

void LevelGenerator::GenerateWall(Vector2 bottomLeftPos, float componentSize, int wallXSize, int wallYSize, bool zAxis) {
	Vector3 base = Vector3(bottomLeftPos.x, -13, bottomLeftPos.y);
	Vector3 componentPosition;
	GameObject* temp;
	for (int y = 0; y < wallYSize; y++) {
		for (int x = 0; x < wallXSize; x++) {
			switch (zAxis) {
			case(false):
				componentPosition = Vector3(base.x + (x * (componentSize + 1)), base.y + (y * (componentSize + 1)), base.z);
				temp = AddOBBCubeToWorld(componentPosition, Vector3(componentSize, componentSize, componentSize), 10.0f, "Wall Component");
				temp->GetRenderObject()->SetColour(Vector4(0.2f, 0.2f, 0.2f, 1));
				break;
			case(true):
				componentPosition = Vector3(base.x, base.y + (y * (componentSize + 1)), base.z + (x * (componentSize + 1)));
				temp = AddOBBCubeToWorld(componentPosition, Vector3(componentSize, componentSize, componentSize), 10.0f, "Wall Component");
				temp->GetRenderObject()->SetColour(Vector4(0.2f, 0.2f, 0.2f, 1));
				break;
			}
		}
	}
}

void LevelGenerator::GeneratePhysicsLevelStaticWalls() {
	GameObject* wall0 = AddAABBCubeToWorld(Vector3(-67.5, -8, 30), Vector3(15, 10, 90), 0, "Wall");
	wall0->GetRenderObject()->SetColour(Vector4(0.2f, 0.2f, 0.2f, 1));

	GameObject* wall1 = AddAABBCubeToWorld(Vector3(-7.5, -8, -30), Vector3(15, 10, 90), 0, "Wall");
	wall1->GetRenderObject()->SetColour(Vector4(0.2f, 0.2f, 0.2f, 1));

	GameObject* wall2 = AddAABBCubeToWorld(Vector3(52.5, -8, 30), Vector3(15, 10, 90), 0, "Wall");
	wall2->GetRenderObject()->SetColour(Vector4(0.2f, 0.2f, 0.2f, 1));

	GameObject* wall3 = AddAABBCubeToWorld(Vector3(105, -8, -30), Vector3(15, 10, 90), 0, "Wall");
	wall3->GetRenderObject()->SetColour(Vector4(0.2f, 0.2f, 0.2f, 1));

	GameObject* floor = AddFloorToWorld(Vector3(0, -20, 0), "Floor Object");
	floor->GetRenderObject()->SetColour(Vector4(0.2f, 0.2f, 0.2f, 1));
}

// pathfinding functions

void LevelGenerator::GenerateLevelWalls(const std::string& filename) {
	std::ifstream infile(Assets::DATADIR + filename);

	infile >> nodeSize;
	infile >> gridWidth;
	infile >> gridHeight;

	for (int z = 0; z < gridHeight; z++) {
		for (int x = 0; x < gridWidth; x++) {
			char type = 0;
			infile >> type;
			if (type == 'x') {
				GameObject* wallTemp = AddAABBCubeToWorld(Vector3((float)(x * nodeSize), -8, (float)(z * nodeSize)), Vector3(5, 10, 5), 0.0f, "Wall Object");
				wallTemp->GetRenderObject()->SetColour(Vector4(0.2f, 0.2f, 0.2f, 1));
			}
				if (type == 'o') {
				GameObject* temp = GenerateGoal(Vector3((float)(x * nodeSize), -15, (float)(z * nodeSize)), Pathfinding);
				pathfindingGoals.push_back(temp);
			}
		}
	}
}

void LevelGenerator::CreatePath() {
	testNodes.clear();
	NavigationGrid grid("TestGrid1.txt");

	NavigationPath outPath;

	Vector3 startPos = enemyObject->GetTransform().GetPosition();
	Vector3 endPos = playerObject->GetTransform().GetPosition();

	bool found = grid.FindPath(startPos, endPos, outPath);

	Vector3 pos;
	while (outPath.PopWaypoint(pos)) {
		testNodes.push_back(pos);
	}
}

void LevelGenerator::MoveObjectOnPath(float speed) {
	// draw current path
	for (int i = 1; i < testNodes.size(); ++i) {
		Vector3 a = testNodes[i - 1];
		Vector3 b = testNodes[i];

		Debug::DrawLine(a, b, Debug::BLUE);
	}

	Vector3 currentPos = enemyObject->GetTransform().GetPosition();
	Vector3 targetNode;
	switch (testNodes.size() < 3)  {
	case(true):
		targetNode = playerObject->GetTransform().GetPosition();
		break;
	case(false):
		targetNode = testNodes[currentNode + 2];
		break;
	}

	Vector3 force = targetNode - currentPos;
	Vector3 finalForce = MakeForceUnfiform(force, speed);

	enemyObject->GetPhysicsObject()->AddForce(finalForce);
}

void LevelGenerator::ChaseObject(GameObject* chasing, GameObject* chased, float speed) {
	Vector3 force = chased->GetTransform().GetPosition() - chasing->GetTransform().GetPosition();
	Vector3 finalForce = MakeForceUnfiform(force, speed);
	chasing->GetPhysicsObject()->AddForce(finalForce);
}

// networking functions

void LevelGenerator::CreatePlayer1() {
	std::cout << "START AS SERVER!!!" << std::endl;
	StartAsServer();

	LockCameraToObject(player1Object);
	world->GetMainCamera().SetPosition(player1Object->GetTransform().GetPosition() + lockedOffset);
}

void LevelGenerator::CreatePlayer2() {
	std::cout << "START AS CLIENT!!!" << std::endl;
	StartAsClient(127, 0, 0, 1);

	LockCameraToObject(player2Object);
	world->GetMainCamera().SetPosition(player2Object->GetTransform().GetPosition() + lockedOffset);

	useGravity = false;
}

void LevelGenerator::UpdatePlayer2() {
	Vector3 cameraPos = lastPlayer2Input.cameraPosition;
	Vector3 characterPos = player2Object->GetTransform().GetPosition();
	Vector3 temp = Vector3(cameraPos.x, characterPos.y, cameraPos.z);

	Vector3 fwdAxis = cameraPos - characterPos;

	Vector3 rightAxis = Vector3::Cross(fwdAxis, Vector3(0, 1, 0));

	if (lastPlayer2Input.buttonStates[0] == ' ')
		std::cout << "JUMP" << std::endl;
	if (lastPlayer2Input.buttonStates[1] == 'W')
		player2Object->GetPhysicsObject()->AddForce(-fwdAxis);
	if (lastPlayer2Input.buttonStates[2] == 'A')
		player2Object->GetPhysicsObject()->AddForce( rightAxis);
	if (lastPlayer2Input.buttonStates[3] == 'S')
		player2Object->GetPhysicsObject()->AddForce( fwdAxis);
	if (lastPlayer2Input.buttonStates[4] == 'D')
		player2Object->GetPhysicsObject()->AddForce(-rightAxis);

	RayCollision closestCollision;
	Vector3 rayPos = player2Object->GetTransform().GetPosition();
	Vector3 rayDirection = Vector3(0, -1, 0);
	Ray r = Ray(rayPos, rayDirection);

	if (lastPlayer2Input.buttonStates[5] == ' ') {
		if (world->Raycast(r, closestCollision, true, player2Object)) {
			if ((player2Object->GetTransform().GetPosition().y - closestCollision.collidedAt.y) <= 1.5f)
				player2Object->GetPhysicsObject()->AddForce(Vector3(0, 1000, 0));
		}
	}

	Quaternion rotation = GetObjectRotation(player2Object->GetTransform().GetPosition(), cameraPos);
	Quaternion flip = Quaternion(0, 0, 1.0f, 0);
	player2Object->GetTransform().SetOrientation(rotation * flip);
}

bool LevelGenerator::PlayerWin() {
	switch (networkRole) {
	case(Server):
		if (player1Points > player2Points)
			return true;
		if (player1Points < player2Points)
			return false;
		break;
	case(Client):
		if (player1Points < player2Points)
			return true;
		if (player1Points > player2Points)
			return false;
		break;
	}
}

void LevelGenerator::GenerateNetworkingLevelWalls() {
	// add walls, dont need to be synchronised as they are static
	GameObject* wall0 = AddAABBCubeToWorld(Vector3(-43, -8, 48), Vector3(20, 10, 10), 0, "Wall");
	wall0->GetRenderObject()->SetColour(Vector4(0.2f, 0.2f, 0.2f, 1));
	GameObject* wall1 = AddAABBCubeToWorld(Vector3(43, -8, 48), Vector3(20, 10, 10), 0, "Wall");
	wall1->GetRenderObject()->SetColour(Vector4(0.2f, 0.2f, 0.2f, 1));
	GameObject* wall2 = AddAABBCubeToWorld(Vector3(-43, -8, -48), Vector3(20, 10, 10), 0, "Wall");
	wall2->GetRenderObject()->SetColour(Vector4(0.2f, 0.2f, 0.2f, 1));
	GameObject* wall3 = AddAABBCubeToWorld(Vector3(43, -8, -48), Vector3(20, 10, 10), 0, "Wall");
	wall3->GetRenderObject()->SetColour(Vector4(0.2f, 0.2f, 0.2f, 1));

	GameObject* wall4 = AddAABBCubeToWorld(Vector3(-53, -8, 28), Vector3(10, 10, 10), 0, "Wall");
	wall4->GetRenderObject()->SetColour(Vector4(0.2f, 0.2f, 0.2f, 1));
	GameObject* wall5 = AddAABBCubeToWorld(Vector3(53, -8, 28), Vector3(10, 10, 10), 0, "Wall");
	wall5->GetRenderObject()->SetColour(Vector4(0.2f, 0.2f, 0.2f, 1));
	GameObject* wall6 = AddAABBCubeToWorld(Vector3(-53, -8, -28), Vector3(10, 10, 10), 0, "Wall");
	wall6->GetRenderObject()->SetColour(Vector4(0.2f, 0.2f, 0.2f, 1));
	GameObject* wall7 = AddAABBCubeToWorld(Vector3(53, -8, -28), Vector3(10, 10, 10), 0, "Wall");
	wall7->GetRenderObject()->SetColour(Vector4(0.2f, 0.2f, 0.2f, 1));
}

void LevelGenerator::GenerateNetworkingLevelGoals() {
	GameObject* goal1 = GenerateGoal(Vector3(30, -10, 27), Networking);

	GameObject* goal2 = GenerateGoal(Vector3(30, -10, -27), Networking);

	GameObject* goal3 = GenerateGoal(Vector3(-30, -10, 27), Networking);

	GameObject* goal4 = GenerateGoal(Vector3(-30, -10, -27), Networking);
}

// display functions

void LevelGenerator::DisplayPointsAndItemsLeft() {
	Debug::Print("Timer: " + std::to_string((int)timer), Vector2(0,5), Debug::BLACK);
	Debug::Print("Points: " + std::to_string(currentPoints), Vector2(0, 10), Debug::BLACK);
	Debug::Print("Items Gathered: " + std::to_string(itemsGathered), Vector2(0, 15), Debug::BLACK);
	Debug::Print("Items Left: " + std::to_string(itemsLeft), Vector2(0, 20), Debug::BLACK);
}

void LevelGenerator::DisplayPlayer1Data() {
	Debug::Print("Points: " + std::to_string(player1Points), Vector2(0, 5), Debug::BLACK);
	Debug::Print("Items Gathered: " + std::to_string(player1Items), Vector2(0, 10), Debug::BLACK);
	Debug::Print("Items Left: " + std::to_string(itemsLeft), Vector2(0, 15), Debug::BLACK);
}

void LevelGenerator::DisplayPlayer2Data() {
	Debug::Print("Points: " + std::to_string(player2Points), Vector2(0, 5), Debug::BLACK);
	Debug::Print("Items Gathered: " + std::to_string(player2Items), Vector2(0, 10), Debug::BLACK);
	Debug::Print("Items Left: " + std::to_string(itemsLeft), Vector2(0, 15), Debug::BLACK);
}

void LevelGenerator::DisplayMainMenu() {
	world->ClearAndErase();
	physics->Clear();
	Debug::Print(  "WELCOME TO MY GAME", Vector2(35, 50), Debug::BLACK);
	Debug::Print("PRESS SPACE TO PLAY SINGLEPLAYER", Vector2(20, 55), Debug::BLACK);
	Debug::Print("PRESS BACKSPACE TO PLAY MULTIPLAYER", Vector2(17, 60), Debug::BLACK);
}

void LevelGenerator::DisplayPhysicsMenu() {
	world->ClearAndErase();
	physics->Clear();

	Debug::Print("PHYSICS LEVEL", Vector2(40, 50), Debug::BLACK);
	Debug::Print("NAVIGATE THROUGH THE CORRIDORS AND MAKE YOUR WAY TO THE END", Vector2(2, 55), Debug::BLACK);
	Debug::Print("YOU CAN USE THE BLUE OBB TO HELP YOU BREAK THE WALLS", Vector2(1, 60), Debug::BLACK);
	Debug::Print("OR JUST CRASH THROUGH THEM", Vector2(25, 65), Debug::BLACK);
}

void LevelGenerator::DisplayPathfindingMenu() {
	world->ClearAndErase();
	physics->Clear();

	Debug::Print("PATHFINDING LEVEL", Vector2(40, 50), Debug::BLACK);
	Debug::Print("NAVIGATE THROUGH THE MAZE AND FIND THE GOLDEN ORBS", Vector2(2, 55), Debug::BLACK);
	Debug::Print("WATCH OUT FOR THE FARMER THOUGH", Vector2(25, 60), Debug::BLACK);
	Debug::Print("THEY WILL TRACK AND CHASE YOU DOWN", Vector2(18, 65), Debug::BLACK);
	Debug::Print("PRESS SPACE TO CONTINUE", Vector2(35, 70), Debug::BLACK);
}

void LevelGenerator::DisplayNetworkingMenu() {
	world->ClearAndErase();
	physics->Clear();

	Debug::Print("NETWORKING LEVEL", Vector2(40, 50), Debug::BLACK);
	Debug::Print("PRESS SPACE TO CONTINUE", Vector2(35, 55), Debug::BLACK);
	Debug::Print("PRESS F9 TO START GAME AS SERVER/P1", Vector2(22, 60), Debug::BLACK);
	Debug::Print("PRESS F0 TO START GAME AS CLIENT/P2", Vector2(22, 65), Debug::BLACK);
	Debug::Print("THE PERSON WHO GETS THE MOST POINTS AFTER", Vector2(15, 70), Debug::BLACK);
	Debug::Print("ALL GOALS HAVE BEEN COLLECTED WILL WIN", Vector2(18, 75), Debug::BLACK);
}

void LevelGenerator::DisplayWinScreen() {
	world->ClearAndErase();
	physics->Clear();
	Debug::Print("YOU WIN", Vector2(40, 50), Debug::BLACK);
	Debug::Print("YOUR SCORE : " + std::to_string(currentPoints), Vector2(33, 55), Debug::BLACK);
	Debug::Print("YOUR TIME : " + std::to_string((int)timer), Vector2(33, 60), Debug::BLACK);
	Debug::Print("PRESS ESC TO EXIT", Vector2(30, 65), Debug::BLACK);
}

void LevelGenerator::DisplayLoseScreen() {
	world->ClearAndErase();
	physics->Clear();
	Debug::Print("YOU LOSE", Vector2(40, 50), Debug::BLACK);
	Debug::Print("YOUR SCORE : " + std::to_string(currentPoints), Vector2(33, 55), Debug::BLACK);
	Debug::Print("YOUR TIME : " + std::to_string((int)timer), Vector2(33, 60), Debug::BLACK);
	Debug::Print("PRESS ESC TO EXIT", Vector2(30, 65), Debug::BLACK);
}

// level functions
void LevelGenerator::GeneratePhysicsLevel() {
	currentLevel = Physics;
 	world->ClearAndErase();
	physics->Clear();

	itemsLeft = 1;
	itemsGathered = 0;

	playerObject = GeneratePlayerObject(Vector3(-90,-13,90), "Player Object");
	GameObject* heavyCube = AddOBBCubeToWorld(Vector3(-90, 20, 80), Vector3(3,3,3), 0.1f, "HeavyCube");
	heavyCube->GetRenderObject()->SetColour(Vector4(0,0,0.1f,1));
	GameObject* heavyCube1 = AddOBBCubeToWorld(Vector3(-93, 30, 80), Vector3(3, 3, 3), 0.1f, "HeavyCube");
	heavyCube1->GetRenderObject()->SetColour(Vector4(0, 0, 0.1f, 1));

	GeneratePhysicsLevelStaticWalls();

	BridgeConstraintTest(Vector3(-100,15,-200));

	GenerateWall(Vector2(-105, -13), 4.0f, 5, 3, false);
	GenerateWall(Vector2(10, -13), 3.8f, 4, 3, false);
	GenerateWall(Vector2(-50, 20), 3.8f, 4, 3, false);
	GenerateWall(Vector2(70, 20), 3.8f, 3, 3, false);

	GenerateGoal(Vector3(100,-13,90), Physics);

	LockCameraToObject(playerObject);
	world->GetMainCamera().SetPosition(playerObject->GetTransform().GetPosition() + lockedOffset);
}

void LevelGenerator::GeneratePathfindingLevel() {
	currentLevel = Pathfinding;
	itemsLeft = 5;
	itemsGathered = 0;
	world->ClearAndErase();
	physics->Clear();

	playerObject = GeneratePlayerObject(Vector3(10,-13,190), "Player Object");
	enemyObject = GenerateEnemyObject(Vector3(180,-13,10), "Enemy Object");

	currentNode = 0;
	GameObject* floor = AddFloorToWorld(Vector3(100,-20,100), "Floor Object");
	floor->GetRenderObject()->SetColour(Vector4(0.2f, 0.2f, 0.2f, 1));
	GenerateLevelWalls("TestGrid1.txt");

	stateMachine = new StateMachine();

	trackPlayer = new State([&](float dt)->void {
		Debug::DrawLine(playerObject->GetTransform().GetPosition(), enemyObject->GetTransform().GetPosition(), Debug::RED);
		CreatePath();
		MoveObjectOnPath(enemySpeed);
		}
	);

	chasePlayer = new State([&](float dt) {
		Debug::DrawLine(playerObject->GetTransform().GetPosition(), enemyObject->GetTransform().GetPosition(), Debug::GREEN);
		ChaseObject(enemyObject, playerObject, enemySpeed);
		}
	);

	trackToChase = new StateTransition(trackPlayer, chasePlayer, [&](void)->bool {
		RayCollision closestCollision;
		Vector3 rayPos = enemyObject->GetTransform().GetPosition();
		Vector3 rayDirection = playerObject->GetTransform().GetPosition() - rayPos;
		rayDirection.Normalise();
		Ray r = Ray(rayPos, rayDirection);

		if (world->Raycast(r, closestCollision, true)) {
			GameObject* temp = (GameObject*)closestCollision.node;
			if (temp->GetName() == "Player Object") {
				return true;
			}
		}
		return false;
		}
	);

	chaseToTrack = new StateTransition(chasePlayer, trackPlayer, [&](void)->bool {
		RayCollision closestCollision;
		Vector3 rayPos = enemyObject->GetTransform().GetPosition();
		Vector3 rayDirection = (playerObject->GetTransform().GetPosition() - rayPos);
		rayDirection.Normalise();
		Ray r = Ray(rayPos, rayDirection);

		if (world->Raycast(r, closestCollision, true)) {
			GameObject* temp = (GameObject*)closestCollision.node;
			if (temp->GetName() != "Player Object") {
				return true;
			}
		}
		return false;
		}
	);

	stateMachine->AddState(trackPlayer);
	stateMachine->AddState(chasePlayer);
	stateMachine->AddTransition(trackToChase);
	stateMachine->AddTransition(chaseToTrack);

	LockCameraToObject(playerObject);
	world->GetMainCamera().SetPosition(playerObject->GetTransform().GetPosition() + lockedOffset);
}

void LevelGenerator::GenerateNetworkingLevel() {
	currentLevel = Networking;
	world->ClearAndErase();
	physics->Clear();

	player1Points = 0;
	player2Points = 0;
	player1Items = 0;
	player2Items = 0;

	itemsLeft = 4;


	player1Object = GeneratePlayerObject(Vector3(-100, 0, 0), "Player 1 Object");
	NetworkObject* player1 = new NetworkObject(*player1Object, 1);
	player1Object->setNetworkObject(player1);

	player2Object = GeneratePlayerObject(Vector3(100, 0, 0), "Player 2 Object");
	NetworkObject* player2 = new NetworkObject(*player2Object, 2);
	player2Object->setNetworkObject(player2);

	GenerateNetworkingLevelWalls();

	GenerateNetworkingLevelGoals();

	InitCamera();
	InitDefaultFloor();
}

// general use functions

bool LevelGenerator::CheckVectorsEqual(const Vector3 vector1, const Vector3 vector2) {
	if (round(vector1.x) == round(vector2.x)) {
		if (round(vector1.y) == round(vector2.y)) {
			if (round(vector1.z) == round(vector2.z)) {}
			return true;
		}
	}
	return false;
}

Vector3 LevelGenerator::MakeForceUnfiform(Vector3 force, float magnitude) {
	float y = force.y;
	force.Normalise();
	Vector3 finalForce = force * magnitude;
	force.y = y;

	return finalForce;
}

bool LevelGenerator::CheckForCollision(GameObject* a, GameObject* b) {
	CollisionDetection::CollisionInfo x;
	if (CollisionDetection::ObjectIntersection(a, b, x)) {
		return true;
	}
	return false;
}

bool LevelGenerator::CheckPlayerHitEnemy() {
	return CheckForCollision(playerObject, enemyObject);
}

void LevelGenerator::CheckPlayerHitGoal() {
	if (currentLevel == Physics) {
		for (auto& x : physicsGoals) {
			if (CheckForCollision(playerObject, x)) {
				x->SetActive();
				x->SetBoundingVolume(nullptr);
				currentPoints += 300 + (timer * 0.2);
				itemsLeft--;
				itemsGathered++;
			}
		}
	}

	if (currentLevel == Pathfinding) {
		for (auto& x : pathfindingGoals) {
			if (CheckForCollision(playerObject, x)) {
				x->SetActive();
				x->SetBoundingVolume(nullptr);
				currentPoints += 300 + (timer * 0.2);
				itemsLeft--;
				itemsGathered++;
			}
		}
	}

	if (currentLevel == Networking) {
		for (auto& x : networkingGoals) {
			if (CheckForCollision(player1Object, x)) {
				x->SetActive();
				x->SetBoundingVolume(nullptr);
				player1Points += 300;
				itemsLeft--;
				player1Items++;
				if (networkRole == Server)
					currentPoints = player1Points;
			}
			if (CheckForCollision(player2Object, x)) {
				x->SetActive();
				x->SetBoundingVolume(nullptr);
				player2Points += 300;
				itemsLeft--;
				player2Items++;
				if (networkRole == Client) {
					currentPoints = player2Points;
				}
			}
		}
	}
}

void LevelGenerator::EnemyChasePlayer() {
	enemyObject->GetPhysicsObject()->ClearForces();

}