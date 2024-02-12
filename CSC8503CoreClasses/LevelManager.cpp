#include "LevelManager.h"
#include "GameWorld.h"
#include "RenderObject.h"
#include "PhysicsObject.h"
#include "RecastBuilder.h"
#include <filesystem>

using namespace NCL::CSC8503;

LevelManager::LevelManager() {
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
	mBuilder = new RecastBuilder();
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
}

void LevelManager::LoadLevel(int id, GameWorld* world, Mesh* mesh, Texture* albedo, Texture* normal, Shader* shader) {
	if (id > mLevelList.size() - 1 || !world) return;
	LoadMap(world, (*mLevelList[id]).GetTileMap(), Vector3(0, 0, 0), mesh, albedo, normal, shader);
	for (auto const& [key, val] : (*mLevelList[id]).GetRooms()) {
		switch ((*val).GetType()) {
		case Medium:
			for (Room* room : mRoomList) {
				if (room->GetType() == Medium) {
					LoadMap(world, room->GetTileMap(), key, mesh, albedo, normal, shader);
					break;
				}
			}
			break;
		}
	}
	mBuilder->BuildNavMesh(mLevelLayout);
	mActiveLevel = id;
}

void LevelManager::LoadMap(GameWorld* world, const std::map<Vector3, TileType>& tileMap, const Vector3& startPosition, 
	Mesh* mesh, Texture* albedo, Texture* normal, Shader* shader) {
	for (auto const& [key, val] : tileMap) {
		switch (val) {
		case Wall:
			AddWallToWorld(world, key + startPosition, mesh, albedo, normal, shader);
			break;
		case Floor:
			AddFloorToWorld(world, key + startPosition, mesh, albedo, normal, shader);
			break;
		}
	}
}

GameObject* LevelManager::AddWallToWorld(GameWorld* world, const Vector3& position, Mesh* mesh, Texture* albedo, Texture* normal, Shader* shader) {
	GameObject* wall = new GameObject("Wall");

	Vector3 wallSize = Vector3(5, 5, 5);
	AABBVolume* volume = new AABBVolume(wallSize);
	wall->SetBoundingVolume((CollisionVolume*)volume);
	wall->GetTransform()
		.SetScale(wallSize * 2)
		.SetPosition(position*10);

	wall->SetRenderObject(new RenderObject(&wall->GetTransform(), mesh, albedo, normal, shader, 1));
	wall->SetPhysicsObject(new PhysicsObject(&wall->GetTransform(), wall->GetBoundingVolume()));

	wall->GetPhysicsObject()->SetInverseMass(0);
	wall->GetPhysicsObject()->InitCubeInertia();

	wall->GetRenderObject()->SetColour(Vector4(0.2f, 0.2f, 0.2f, 1));

	world->AddGameObject(wall);

	mLevelLayout.push_back(wall);

	return wall;
}

GameObject* LevelManager::AddFloorToWorld(GameWorld* world, const Vector3& position, Mesh* mesh, Texture* albedo, Texture* normal, Shader* shader) {
	GameObject* floor = new GameObject("Floor");

	Vector3 wallSize = Vector3(5, 0.5f, 5);
	AABBVolume* volume = new AABBVolume(wallSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(wallSize * 2)
		.SetPosition(position*10);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), mesh, albedo, normal, shader, 1));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	floor->GetRenderObject()->SetColour(Vector4(0.2f, 0.2f, 0.2f, 1));

	world->AddGameObject(floor);

	mLevelLayout.push_back(floor);

	return floor;
}
