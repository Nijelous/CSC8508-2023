#include "LevelManager.h"
#include "GameWorld.h"
#include "../OpenGLRendering/OGLRenderer.h"
#include "RenderObject.h"
#include "PhysicsObject.h"
#include <filesystem>

using namespace NCL::CSC8503;

LevelManager::LevelManager() {
	mRoomList = std::vector<Room>();
	for (const auto& entry : std::filesystem::directory_iterator("../Assets/Levels/Rooms")) {
		Room newRoom = Room(entry.path().string());
		mRoomList.push_back(newRoom);
	}
	mLevelList = std::vector<Level>();
	for (const auto& entry : std::filesystem::directory_iterator("../Assets/Levels/Levels")) {
		Level newLevel = Level(entry.path().string());
		mLevelList.push_back(newLevel);
	}
	mActiveLevel = -1;
	mPlayerPosition = Vector3(0, 0, 0);
}

void LevelManager::UpdateLevel() {
}

void LevelManager::LoadLevel(int id, GameWorld* world, Mesh* mesh, Texture* floorAlbedo, Texture* floorNormal, Shader* shader) {
	if (id > mLevelList.size() - 1 || !world) return;
	LoadMap(mLevelList[id].GetTileMap(), Vector3(0, 0, 0), mesh, floorAlbedo, floorNormal, shader);
	for (auto const& [key, val] : mLevelList[id].GetRooms()) {
		switch (val.GetType()) {
		case Medium:
			for (Room room : mRoomList) {
				if (room.GetType() == Medium) {
					LoadMap(room.GetTileMap(), key);
					break;
				}
			}
			break;
		}
	}
}

float LevelManager::GetSqDistanceToCamera(Vector3& objectPosition) {
	return (objectPosition - mPlayerPosition).LengthSquared();
}

void LevelManager::LoadMap(const std::map<Vector3, TileType>& tileMap, const Vector3& startPosition, 
	Mesh* mesh, Texture* floorAlbedo, Texture* floorNormal, Shader* shader) {
	for (auto const& [key, val] : tileMap) {
		switch (val) {
		case Wall:
			AddWallToWorld(key + startPosition, mesh, floorAlbedo, floorNormal, shader);
			break;
		case Floor:
			AddFloorToWorld(key + startPosition, mesh, floorAlbedo, floorNormal, shader);
			break;
		}
	}
}

GameObject* LevelManager::AddWallToWorld(const Vector3& position, Mesh* mesh, Texture* albedo, Texture* normal, Shader* shader) {
	GameObject* wall = new GameObject("Wall");

	Vector3 wallSize = Vector3(1, 1, 1);
	AABBVolume* volume = new AABBVolume(wallSize);
	wall->SetBoundingVolume((CollisionVolume*)volume);
	wall->GetTransform()
		.SetScale(wallSize * 2)
		.SetPosition(position);

	wall->SetRenderObject(new RenderObject(&wall->GetTransform(), mesh, albedo, normal, shader, 1));
	wall->SetPhysicsObject(new PhysicsObject(&wall->GetTransform(), wall->GetBoundingVolume()));

	wall->GetPhysicsObject()->SetInverseMass(0);
	wall->GetPhysicsObject()->InitCubeInertia();

	wall->GetRenderObject()->SetColour(Vector4(0.2f, 0.2f, 0.2f, 1));

	return wall;
}

GameObject* LevelManager::AddFloorToWorld(const Vector3& position, Mesh* mesh, Texture* floorAlbedo, Texture* floorNormal, Shader* shader) {
	return nullptr;
}
