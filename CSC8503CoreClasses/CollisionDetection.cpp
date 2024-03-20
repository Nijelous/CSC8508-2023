#include "CollisionDetection.h"
#include "CollisionVolume.h"
#include "AABBVolume.h"
#include "OBBVolume.h"
#include "SphereVolume.h"
#include "Window.h"
#include "Maths.h"
#include "Debug.h"

using namespace NCL;

bool CollisionDetection::RayPlaneIntersection(const Ray&r, const Plane&p, RayCollision& collisions) {
	float ln = Vector3::Dot(p.GetNormal(), r.GetDirection());

	if (ln == 0.0f) {
		return false; //direction vectors are perpendicular!
	}
	
	Vector3 planePoint = p.GetPointOnPlane();

	Vector3 pointDir = planePoint - r.GetPosition();

	float d = Vector3::Dot(pointDir, p.GetNormal()) / ln;

	collisions.collidedAt = r.GetPosition() + (r.GetDirection() * d);

	return true;
}

bool CollisionDetection::RayIntersection(const Ray& r,GameObject& object, RayCollision& collision) {
	bool hasCollided = false;

	const Transform& worldTransform = object.GetTransform();
	const CollisionVolume* volume	= object.GetBoundingVolume();

	if (!volume) {
		return false;
	}

	switch (volume->type) {
		case VolumeType::AABB:		hasCollided = RayAABBIntersection(r, worldTransform, (const AABBVolume&)*volume	, collision); break;
		case VolumeType::OBB:		hasCollided = RayOBBIntersection(r, worldTransform, (const OBBVolume&)*volume	, collision); break;
		case VolumeType::Sphere:	hasCollided = RaySphereIntersection(r, worldTransform, (const SphereVolume&)*volume	, collision, worldTransform.GetPosition()); break;
		case VolumeType::Capsule:	hasCollided = RayCapsuleIntersection(r, worldTransform, (const CapsuleVolume&)*volume, collision); break;
	}

	return hasCollided;
}

bool CollisionDetection::RayBoxIntersection(const Ray&r, const Vector3& boxPos, const Vector3& boxSize, RayCollision& collision) {
	Vector3 boxMin = boxPos - boxSize;
	Vector3 boxMax = boxPos + boxSize;

	Vector3 rayPos = r.GetPosition();
	Vector3 rayDir = r.GetDirection();

	Vector3 tVals(-1,-1,-1);

	// find three closest box planes
	for (int i = 0; i < 3; i++) {
		if (rayDir[i] > 0)
			tVals[i] = (boxMin[i] - rayPos[i]) / rayDir[i];
		else if (rayDir[i] < 0)
			tVals[i] = (boxMax[i] - rayPos[i]) / rayDir[i];
	}

	float bestT = tVals.GetMaxElement();
	if (bestT < 0.0f)
		return false;

	Vector3 intersection = rayPos + (rayDir * bestT);
	const float epsilon = 0.0001f; // leeway given in calculation
	for (int i = 0; i < 3; i++) {
		if (intersection[i] + epsilon < boxMin[i] || intersection[i] - epsilon > boxMax[i])
			return false;
	}
	collision.collidedAt = intersection;
	collision.rayDistance = bestT;
	return true;
}

bool CollisionDetection::RayAABBIntersection(const Ray&r, const Transform& worldTransform, const AABBVolume& volume, RayCollision& collision) {
	Vector3 boxPos = worldTransform.GetPosition() + volume.GetOffset();
	Vector3 boxSize = volume.GetHalfDimensions();
	return RayBoxIntersection(r,boxPos, boxSize, collision);
}

// returns true if a given ray collides with a given OBB
// 
// Author: Ewan Squire
bool CollisionDetection::RayOBBIntersection(const Ray&r, const Transform& worldTransform, const OBBVolume& volume, RayCollision& collision) {
	// get the boxes position and orientation
	Quaternion orientation = worldTransform.GetOrientation();
	Vector3 position = worldTransform.GetPosition() + volume.GetOffset();

	Matrix3 transform = Matrix3(orientation);
	Matrix3 invTransform = Matrix3(orientation.Conjugate());

	// translate ray into boxes local space
	Vector3 localRayPos = r.GetPosition() - position;
	Ray tempRay(invTransform * localRayPos, invTransform * r.GetDirection());

	bool collided = RayBoxIntersection(tempRay, Vector3(), volume.GetHalfDimensions(), collision);

	if (collided)
		collision.collidedAt = transform * collision.collidedAt + position;;

	return collided;
}

bool CollisionDetection::RaySphereIntersection(const Ray&r, const Transform& worldTransform, const SphereVolume& volume, RayCollision& collision, Vector3 position) {
	Vector3 spherePos = position + volume.GetOffset();
	float sphereRadius = volume.GetRadius();

	Vector3 dir = (spherePos - r.GetPosition());
	float sphereProj = Vector3::Dot(dir, r.GetDirection());

	if (sphereProj < 0.0f)
		return false;

	// get closest point on ray and sphere radius
	Vector3 point = r.GetPosition() + (r.GetDirection() * sphereProj);
	float sphereDist = (point - spherePos).Length();
	if (sphereDist > sphereRadius)
		return false;

	float offset = sqrt((sphereRadius * sphereRadius) - (sphereDist * sphereDist));

	collision.rayDistance = sphereProj - (offset);
	collision.collidedAt = r.GetPosition() + (r.GetDirection() * collision.rayDistance);

	return true;
}

// returns true if a given ray intersects with a given capsule
//
// Author: Alex Fall
bool CollisionDetection::RayCapsuleIntersection(const Ray& r, const Transform& worldTransform, const CapsuleVolume& volume, RayCollision& collision) {
	Quaternion orientation = worldTransform.GetOrientation().Normalised();
	Vector3 position = worldTransform.GetPosition() + volume.GetOffset();
	float radius = volume.GetRadius();

	Vector3 capsuleDir = Matrix3(orientation) * Vector3(0, 1, 0);
	Vector3 capsuleMax = position + (capsuleDir * volume.GetHalfHeight());
	Vector3 capsuleMin = position - (capsuleDir * volume.GetHalfHeight());

	Vector3 thirdPoint = position + Vector3(1, 1, -(r.GetDirection().x + r.GetDirection().y) / r.GetDirection().z);
	Plane p = Plane::PlaneFromTri(capsuleMax, capsuleMin, thirdPoint);

	float ln = Vector3::Dot(p.GetNormal(), r.GetDirection());
	Vector3 planePointDir = p.GetPointOnPlane() - r.GetPosition();
	float d = Vector3::Dot(planePointDir, p.GetNormal()) / ln;

	Vector3 rayPoint = r.GetPosition() + (r.GetDirection() * d);
	Vector3 pointToCapsuleDir = rayPoint - position;
	float proj = Vector3::Dot(capsuleDir, pointToCapsuleDir);

	Vector3 capsulePoint = position + (capsuleDir * proj);
	if ((capsulePoint - position).Length() > volume.GetHalfHeight()) {
		if ((capsulePoint - capsuleMax).Length() > (capsulePoint - capsuleMin).Length()) capsulePoint = capsuleMin;
		else capsulePoint = capsuleMax;
	}

	Transform sphereTransform = Transform();
	sphereTransform.SetPosition(capsulePoint);

	SphereVolume sv = SphereVolume(radius);

	return RaySphereIntersection(r, sphereTransform, sv, collision, capsulePoint);
}

// objects collisions settled here
 bool CollisionDetection::ObjectIntersection(GameObject* a, GameObject* b, CollisionInfo& collisionInfo) {
	const CollisionVolume* volA = a->GetBoundingVolume();
	const CollisionVolume* volB = b->GetBoundingVolume();

	if (!volA || !volB) {
		return false;
	}

	collisionInfo.a = a;
	collisionInfo.b = b;

	Transform& transformA = a->GetTransform();
	Transform& transformB = b->GetTransform();

	VolumeType pairType = (VolumeType)((int)volA->type | (int)volB->type);

	//Two AABBs
	if (pairType == VolumeType::AABB) {
		return AABBIntersection((AABBVolume&)*volA, transformA, (AABBVolume&)*volB, transformB, collisionInfo);
	}
	//Two Spheres
	if (pairType == VolumeType::Sphere) {
		return SphereIntersection((SphereVolume&)*volA, transformA, (SphereVolume&)*volB, transformB, collisionInfo);
	}
	//Two OBBs
	if (pairType == VolumeType::OBB) {
		return OBBIntersection((OBBVolume&)*volA, transformA, (OBBVolume&)*volB, transformB, collisionInfo);
	}
	//Two Capsules

	//AABB vs Sphere pairs
	if (volA->type == VolumeType::AABB && volB->type == VolumeType::Sphere) {
		return AABBSphereIntersection((AABBVolume&)*volA, transformA, (SphereVolume&)*volB, transformB, collisionInfo);
	}
	if (volA->type == VolumeType::Sphere && volB->type == VolumeType::AABB) {
		collisionInfo.a = b;
		collisionInfo.b = a;
 		return AABBSphereIntersection((AABBVolume&)*volB, transformB, (SphereVolume&)*volA, transformA, collisionInfo);
	}

	if (volA->type == VolumeType::AABB && volB->type == VolumeType::OBB) {
		return AABBOBBIntersection((AABBVolume&)*volA, transformA, (OBBVolume&)*volB, transformB, collisionInfo);
	}
	if (volA->type == VolumeType::OBB && volB->type == VolumeType::AABB) {
		collisionInfo.a = b;
		collisionInfo.b = a;
		return AABBOBBIntersection((AABBVolume&)*volB, transformB, (OBBVolume&)*volA, transformA, collisionInfo);
	}

	//OBB vs sphere pairs
	if (volA->type == VolumeType::OBB && volB->type == VolumeType::Sphere) {
		return OBBSphereIntersection((OBBVolume&)*volA, transformA, (SphereVolume&)*volB, transformB, collisionInfo);
	}
	if (volA->type == VolumeType::Sphere && volB->type == VolumeType::OBB) {
		collisionInfo.a = b;
		collisionInfo.b = a;
		return OBBSphereIntersection((OBBVolume&)*volB, transformB, (SphereVolume&)*volA, transformA, collisionInfo);
	}

	//OBB vs Capsule pairs
	if (volA->type == VolumeType::Capsule && volB->type == VolumeType::OBB) {
		return OBBCapsuleIntersection((CapsuleVolume&)*volA, transformA, (OBBVolume&)*volB, transformB, collisionInfo);
	}
	if (volA->type == VolumeType::OBB && volB->type == VolumeType::Capsule) {
		collisionInfo.a = b;
		collisionInfo.b = a;
		return OBBCapsuleIntersection((CapsuleVolume&)*volB, transformB, (OBBVolume&)*volA, transformA, collisionInfo);
	}

	//Capsule vs other interactions
	if (volA->type == VolumeType::Capsule && volB->type == VolumeType::Sphere) {
		return SphereCapsuleIntersection((CapsuleVolume&)*volA, transformA, (SphereVolume&)*volB, transformB, collisionInfo);
	}
	if (volA->type == VolumeType::Sphere && volB->type == VolumeType::Capsule) {
		collisionInfo.a = b;
		collisionInfo.b = a;
		return SphereCapsuleIntersection((CapsuleVolume&)*volB, transformB, (SphereVolume&)*volA, transformA, collisionInfo);
	}

	if (volA->type == VolumeType::Capsule && volB->type == VolumeType::AABB) {
		return AABBCapsuleIntersection((CapsuleVolume&)*volA, transformA, (AABBVolume&)*volB, transformB, collisionInfo);
	}
	if (volB->type == VolumeType::Capsule && volA->type == VolumeType::AABB) {
		collisionInfo.a = b;
		collisionInfo.b = a;
		return AABBCapsuleIntersection((CapsuleVolume&)*volB, transformB, (AABBVolume&)*volA, transformA, collisionInfo);
	}

	return false;
}

bool CollisionDetection::AABBTest(const Vector3& posA, const Vector3& posB, const Vector3& halfSizeA, const Vector3& halfSizeB) {
	Vector3 delta = posB - posA;
	Vector3 totalSize = halfSizeA + halfSizeB;

	if (abs(delta.x) < totalSize.x &&
		abs(delta.y) < totalSize.y &&
		abs(delta.z) < totalSize.z) {
		return true;
	}
	return false;
}

//AABB/AABB Collisions
bool CollisionDetection::AABBIntersection(const AABBVolume& volumeA, const Transform& worldTransformA,
	const AABBVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo) {

	Vector3 boxAPos = worldTransformA.GetPosition() + volumeA.GetOffset();
	Vector3 boxBPos = worldTransformB.GetPosition() + volumeB.GetOffset();

	Vector3 boxASize = volumeA.GetHalfDimensions();
	Vector3 boxBSize = volumeB.GetHalfDimensions();

	bool overlap = AABBTest(boxAPos, boxBPos, boxASize, boxBSize);

	if (overlap) {
		static const Vector3 faces[6] =
		{
			Vector3(-1, 0, 0), Vector3(1, 0, 0),
			Vector3(0, -1, 0), Vector3(0, 1, 0),
			Vector3(0, 0, -1), Vector3(0, 0, 1),
		};

		Vector3 maxA = boxAPos + boxASize;
		Vector3 minA = boxAPos - boxASize;

		Vector3 maxB = boxBPos + boxBSize;
		Vector3 minB = boxBPos - boxBSize;

		float distances[6] =
		{
			(maxB.x - minA.x),
			(maxA.x - minB.x),
			(maxB.y - minA.y),
			(maxA.y - minB.y),
			(maxB.z - minA.z),
			(maxA.z - minB.z),
		};

		// find smallest penetration extent
		float penetration = FLT_MAX;
		Vector3 bestAxis;
		for (int i = 0; i < 6; i++) {
			if (distances[i] < penetration) {
				penetration = distances[i];
				bestAxis = faces[i];
			}
		}
		// AABBs dont have orientation so use empty Vector3
		collisionInfo.AddContactPoint(Vector3(), Vector3(), bestAxis, penetration);
		return true;
	}
	return false;
}

//Sphere / Sphere Collision
bool CollisionDetection::SphereIntersection(const SphereVolume& volumeA, const Transform& worldTransformA,
	const SphereVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo) {

	float	radius		= volumeA.GetRadius() + volumeB.GetRadius();
	Vector3 delta		= (worldTransformB.GetPosition() + volumeB.GetOffset()) - (worldTransformA.GetPosition() + volumeA.GetOffset());
	float	deltaLength = delta.Length();

	// if collision
	if (deltaLength < radius) {
		float penetration = (radius - deltaLength);
		Vector3 normal =  delta.Normalised();
		Vector3 localA =  normal * volumeA.GetRadius();
		Vector3 localB = -normal * volumeB.GetRadius();

		collisionInfo.AddContactPoint(localA,localB,normal,penetration);
		return true;
	}
	// no collision
	return false;
}

// made using these sources
// https://www.youtube.com/watch?v=Zgf1DYrmSnk&list=PLSlpr6o9vURwq3oxVZSimY8iC-cdd3kIs&index=6
// https://www.youtube.com/watch?v=SUyG3aV_vpM&list=PLSlpr6o9vURwq3oxVZSimY8iC-cdd3kIs&index=7
// https://stackoverflow.com/questions/5900320/separating-axis-theorem-finding-which-edge-normals-to-use
// 
// Returns true if two given OBBs intersect with one another
// 
// Author: Ewan Squire
// 
//OBB/OBB Collision
bool CollisionDetection::OBBIntersection(const OBBVolume& volumeA, const Transform& worldTransformA,
	const OBBVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo) {

	Vector3* edgeNormals = GetOBBEdgeNormals(worldTransformA, worldTransformB);
	Vector3* OBB_A = GetOBBVertices(volumeA, worldTransformA);
	Vector3* OBB_B = GetOBBVertices(volumeB, worldTransformB);

	float minimumPenetration = FLT_MAX;
	Vector3 edgeNormalWithMinOverlap;

	for (int i = 0; i < 15; i++) {
		if (edgeNormals[i] == Vector3(0,0,0))
			continue;
		
		float minA = FLT_MAX;
		float maxA = std::numeric_limits<float>::lowest();

		float minB = FLT_MAX;
		float maxB = std::numeric_limits<float>::lowest();
		// for every vertex on the OBB
		for (int j = 0; j < 8; j++) {
			// project A and B onto the given axis created by edge normal
			float projectA = Vector3::Dot(edgeNormals[i], OBB_A[j]) / edgeNormals[i].Length();
			float projectB = Vector3::Dot(edgeNormals[i], OBB_B[j]) / edgeNormals[i].Length();

			minA = std::min(minA, projectA);
			maxA = std::max(maxA, projectA);

			minB = std::min(minB, projectB);
			maxB = std::max(maxB, projectB);
		}

		// true if either B is within A's range or B is within A's range when projected on axis
		bool collision = (minA <= minB && minB <= maxA) || (minB <= minA && minA <= maxB);

		if (!collision) {
			delete OBB_A;
			delete OBB_B;
			delete edgeNormals;
			return false;
		}

		if (std::min(maxB - minA, maxA - minB) < minimumPenetration) {
			minimumPenetration = std::min(maxB - minA, maxA - minB);
			edgeNormalWithMinOverlap = edgeNormals[i];
		}
	}

	Vector3 dir = (worldTransformB.GetPosition() + volumeB.GetOffset()) - (worldTransformA.GetPosition() + volumeA.GetOffset());
	float checkForceDir = Vector3::Dot(dir, edgeNormalWithMinOverlap);

	// if surface normal is not pointing from A->B then reverse it so it is
	if (checkForceDir < 0.0f)
		edgeNormalWithMinOverlap = -edgeNormalWithMinOverlap;

	// if loop finishes then collision has occured
	collisionInfo.AddContactPoint(Vector3(), Vector3(), edgeNormalWithMinOverlap, minimumPenetration);

	

	// empty heap
	delete edgeNormals;
	delete OBB_A;
	delete OBB_B;

	return true;
}

// Gets the Normals of every edge in every OBB as well as the result of each edge normal Crosses with one another
//
// Author: Ewan Squire
Vector3* CollisionDetection::GetOBBEdgeNormals(const Transform& transformA, const Transform& transformB) {
	Vector3* edgeNormals = new Vector3[15];

	// get edge normals of both OBBs

	edgeNormals[0] = transformA.GetOrientation() * Vector3(1, 0, 0);
	edgeNormals[1] = transformA.GetOrientation() * Vector3(0, 1, 0);
	edgeNormals[2] = transformA.GetOrientation() * Vector3(0, 0, 1);

	edgeNormals[3] = transformB.GetOrientation() * Vector3(1, 0, 0);
	edgeNormals[4] = transformB.GetOrientation() * Vector3(0, 1, 0);
	edgeNormals[5] = transformB.GetOrientation() * Vector3(0, 0, 1);

	// start adding after created projections
	int startPoint = 6;

	// cross edge normals with one another
	for (int i = 0; i < 3; i++) {
		for (int j = 3; j < 6; j++) {
			edgeNormals[startPoint] = Vector3::Cross(edgeNormals[i], edgeNormals[j]).Normalised();
			startPoint++;
		}
	}

	return edgeNormals;
}

// returns an array of Vector3's that contains the location of all vertices in the given OBB in world space
//
// Author: Ewan Squire
Vector3* CollisionDetection::GetOBBVertices(const OBBVolume& OBB_volume, const Transform& OBB_transform) {
	// define OBB points around (0,0) first
	
	Vector3* OBBVertices = new Vector3[8];

	OBBVertices[0] = Vector3(-OBB_volume.GetHalfDimensions().x, -OBB_volume.GetHalfDimensions().y, -OBB_volume.GetHalfDimensions().z);
	OBBVertices[1] = Vector3(-OBB_volume.GetHalfDimensions().x, -OBB_volume.GetHalfDimensions().y,  OBB_volume.GetHalfDimensions().z);
	OBBVertices[2] = Vector3( OBB_volume.GetHalfDimensions().x, -OBB_volume.GetHalfDimensions().y, -OBB_volume.GetHalfDimensions().z);
	OBBVertices[3] = Vector3( OBB_volume.GetHalfDimensions().x, -OBB_volume.GetHalfDimensions().y,  OBB_volume.GetHalfDimensions().z);

	OBBVertices[4] = Vector3(-OBB_volume.GetHalfDimensions().x,  OBB_volume.GetHalfDimensions().y, -OBB_volume.GetHalfDimensions().z);
	OBBVertices[5] = Vector3(-OBB_volume.GetHalfDimensions().x,  OBB_volume.GetHalfDimensions().y,  OBB_volume.GetHalfDimensions().z);
	OBBVertices[6] = Vector3( OBB_volume.GetHalfDimensions().x,  OBB_volume.GetHalfDimensions().y, -OBB_volume.GetHalfDimensions().z);
	OBBVertices[7] = Vector3( OBB_volume.GetHalfDimensions().x,  OBB_volume.GetHalfDimensions().y,  OBB_volume.GetHalfDimensions().z);

	// transform points into world space using OBB transform
	for (int i = 0; i < 8; i++)
		OBBVertices[i] = OBB_transform.GetOrientation() * OBBVertices[i];

	for (int i = 0; i < 8; ++i)
		OBBVertices[i] += OBB_transform.GetPosition();

	return OBBVertices;
}

//AABB - Sphere Collision
bool CollisionDetection::AABBSphereIntersection(const AABBVolume& volumeA, const Transform& worldTransformA,
	const SphereVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo) {

	Vector3 boxSize = volumeA.GetHalfDimensions();

	Vector3 delta = (worldTransformB.GetPosition() + volumeB.GetOffset()) - (worldTransformA.GetPosition() + volumeA.GetOffset());
	Vector3 closestPointOnBox = Maths::Clamp(delta, -boxSize, boxSize);

	Vector3 localPoint = delta - closestPointOnBox;
	float distance = localPoint.Length();

	// collision occurs if cloest point is on/inside sphere
	if (distance < volumeB.GetRadius()) {
		Vector3 collisionNormal = localPoint.Normalised();
		float penetration = (volumeB.GetRadius() - distance);

		Vector3 localA = Vector3();
		Vector3 localB = -collisionNormal * volumeB.GetRadius();

		collisionInfo.AddContactPoint(localA, localB, collisionNormal, penetration);
		return true;
	}

	return false;
}

//AABB - OBB Collision
// Returns true if a given AABB is colliding with a given OBB
// 
//Author: Ewan Squire
bool CollisionDetection::AABBOBBIntersection(const AABBVolume& volumeA, const Transform& worldTransformA,
	const OBBVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo) {

	OBBVolume tempAABBToOBBVolume = OBBVolume(volumeA.GetHalfDimensions(), volumeA.GetOffset());
	Transform tempTransform;
	tempTransform.SetPosition(worldTransformA.GetPosition() + volumeA.GetOffset());
	tempTransform.SetOrientation(Quaternion(0.0f, 0.0f, 0.0f, 1.0f));
	tempTransform.SetScale(worldTransformA.GetScale());

	bool collision = OBBIntersection(tempAABBToOBBVolume, tempTransform, volumeB, worldTransformB, collisionInfo);

	if (collision) {
		return true;
	}
	return false;
}

bool CollisionDetection::OBBCapsuleIntersection(const CapsuleVolume& volumeA, const Transform& worldTransformA, const OBBVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo)
{
	Quaternion orientation = worldTransformB.GetOrientation();

	Matrix3 transform = Matrix3(orientation);
	Matrix3 invTransform = Matrix3(orientation.Conjugate());

	Vector3 localCapsulePos = (worldTransformA.GetPosition() + volumeA.GetOffset()) - (worldTransformB.GetPosition() + volumeB.GetOffset());

	Transform localCapsuleTransform;
	localCapsuleTransform.SetPosition((invTransform * localCapsulePos) + worldTransformB.GetPosition() + volumeB.GetOffset());
	localCapsuleTransform.SetOrientation((invTransform * worldTransformA.GetOrientation()));

	AABBVolume AABB = AABBVolume(volumeB.GetHalfDimensions(), volumeB.GetOffset());

	bool collided = AABBCapsuleIntersection(volumeA, localCapsuleTransform, AABB, worldTransformB, collisionInfo);

	if (collided) {
		collisionInfo.point.localA = transform * collisionInfo.point.localA;
		collisionInfo.point.localB = transform * collisionInfo.point.localB;
		collisionInfo.point.normal = transform * collisionInfo.point.normal;
	}

	return collided;
}

//AABB - Capsule Collision
bool CollisionDetection::AABBCapsuleIntersection(const CapsuleVolume& volumeA, const Transform& worldTransformA,
	const AABBVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo) {
	Vector3 boxSize = volumeB.GetHalfDimensions();

	Vector3 delta = (worldTransformA.GetPosition() + volumeA.GetOffset()) - (worldTransformB.GetPosition() + volumeB.GetOffset());
	Vector3 transformAPos = worldTransformA.GetPosition() + volumeA.GetOffset();

	Vector3 closestPointOnBox = (worldTransformB.GetPosition() + volumeB.GetOffset()) + Maths::Clamp(delta, -boxSize, boxSize);

	Quaternion capsuleOrientation = worldTransformA.GetOrientation().Normalised();
	Vector3 capsuleDir = Matrix3(capsuleOrientation) * Vector3(0, 1, 0);
	Vector3 capsuleMax = transformAPos + (capsuleDir * volumeA.GetHalfHeight());
	Vector3 capsuleMin = transformAPos - (capsuleDir * volumeA.GetHalfHeight());

	Vector3 pointToCapsuleDir = closestPointOnBox - transformAPos;
	float proj = Vector3::Dot(capsuleDir, pointToCapsuleDir);

	Vector3 capsulePoint = transformAPos + (capsuleDir * proj);
	if ((capsulePoint - transformAPos).Length() > volumeA.GetHalfHeight()) {
		if ((capsulePoint - capsuleMax).Length() > (capsulePoint - capsuleMin).Length()) capsulePoint = capsuleMin;
		else capsulePoint = capsuleMax;
	}

	float pointDistance = (capsulePoint - closestPointOnBox).Length();

	float offset = volumeA.GetRadius();

	if (pointDistance < offset) {
		Vector3 collisionNormal = pointToCapsuleDir.Normalised();
		float penetration = (volumeA.GetRadius() - pointDistance);

		Vector3 localA = collisionNormal * volumeA.GetRadius();
		Vector3 localB = Vector3();

		collisionInfo.AddContactPoint(localA, localB, collisionNormal, penetration);
		return true;
	}
	return false;
}

//OBB - Sphere Collision
// Returns true if a given Sphere is colliding with a given OBB
// 
//Author: Ewan Squire
bool  CollisionDetection::OBBSphereIntersection(const OBBVolume& volumeA, const Transform& worldTransformA,
	const SphereVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo) {

	Quaternion OBB_orientation = worldTransformA.GetOrientation();
	Vector3 OBB_position = worldTransformA.GetPosition() + volumeA.GetOffset();

	Vector3 spherePos = worldTransformB.GetPosition() + volumeB.GetOffset();

	Matrix3 transform = Matrix3(OBB_orientation);
	Matrix3 invTransform = Matrix3(OBB_orientation.Conjugate());

	// translate sphere into boxes local space
	Vector3 localSpherePos = invTransform * spherePos;
	// translate OBB into local space
	Vector3 localOBBPos = invTransform * OBB_position;

	Vector3 boxSize = volumeA.GetHalfDimensions();
	Vector3 delta = localSpherePos - localOBBPos;
	Vector3 closestPointOnBox = Maths::Clamp(delta, -boxSize, boxSize);
	Vector3 localPoint = delta - closestPointOnBox;
	float distance = localPoint.Length();

	if (distance < volumeB.GetRadius()) {
		// translate normal back into world space
		Vector3 newDelta = spherePos - OBB_position;
		Vector3 newClosestPointOnBox = Maths::Clamp(delta, -boxSize, boxSize);
		Vector3 realLocalPoint = newDelta - newClosestPointOnBox;
		Vector3 collisionNormal = realLocalPoint.Normalised();
		float penetration = (volumeB.GetRadius() - distance);

		Vector3 localA = Vector3();
		Vector3 localB = -collisionNormal * volumeB.GetRadius();

		collisionInfo.AddContactPoint(localA, localB, collisionNormal, penetration);
		return true;
	}

	return false;
}

//Sphere - Capsule Collision
bool CollisionDetection::SphereCapsuleIntersection(
	const CapsuleVolume& volumeA, const Transform& worldTransformA,
	const SphereVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo) {

	Quaternion capsuleOrientation = worldTransformA.GetOrientation().Normalised();

	Vector3 capsuleDir = Matrix3(capsuleOrientation) * Vector3(0, 1, 0);
	Vector3 capsuleMax = worldTransformA.GetPosition() + volumeA.GetOffset() + (capsuleDir * volumeA.GetHalfHeight());
	Vector3 capsuleMin = worldTransformA.GetPosition() + volumeA.GetOffset() - (capsuleDir * volumeA.GetHalfHeight());

	Vector3 pointToCapsuleDir = (worldTransformB.GetPosition() + volumeB.GetOffset()) - (worldTransformA.GetPosition() + volumeA.GetOffset());
	float proj = Vector3::Dot(capsuleDir, pointToCapsuleDir);

	Vector3 capsulePoint = worldTransformA.GetPosition() + volumeA.GetOffset() + (capsuleDir * proj);
	if ((capsulePoint - (worldTransformA.GetPosition() + volumeA.GetOffset())).Length() > volumeA.GetHalfHeight()) {
		if ((capsulePoint - capsuleMax).Length() > (capsulePoint - capsuleMin).Length()) capsulePoint = capsuleMin;
		else capsulePoint = capsuleMax;
	}

	Transform sphereTransform = Transform();
	sphereTransform.SetPosition(capsulePoint);

	SphereVolume sv = SphereVolume(volumeA.GetRadius());

	return SphereIntersection(sv, sphereTransform, volumeB, worldTransformB, collisionInfo);
}

Matrix4 GenerateInverseView(const Camera &c) {
	float pitch = c.GetPitch();
	float yaw	= c.GetYaw();
	Vector3 position = c.GetPosition();

	Matrix4 iview =
		Matrix4::Translation(position) *
		Matrix4::Rotation(-yaw, Vector3(0, -1, 0)) *
		Matrix4::Rotation(-pitch, Vector3(-1, 0, 0));

	return iview;
}

Matrix4 GenerateInverseProjection(float aspect, float fov, float nearPlane, float farPlane) {
	float negDepth = nearPlane - farPlane;

	float invNegDepth = negDepth / (2 * (farPlane * nearPlane));

	Matrix4 m;

	float h = 1.0f / tan(fov*PI_OVER_360);

	m.array[0][0] = aspect / h;
	m.array[1][1] = tan(fov * PI_OVER_360);
	m.array[2][2] = 0.0f;

	m.array[2][3] = invNegDepth;//// +PI_OVER_360;
	m.array[3][2] = -1.0f;
	m.array[3][3] = (0.5f / nearPlane) + (0.5f / farPlane);

	return m;
}

Vector3 CollisionDetection::Unproject(const Vector3& screenPos, const PerspectiveCamera& cam) {
	Vector2i screenSize = Window::GetWindow()->GetScreenSize();

	float aspect = (float)screenSize.x / (float)screenSize.y;
	float fov		= cam.GetFieldOfVision();
	float nearPlane = cam.GetNearPlane();
	float farPlane  = cam.GetFarPlane();

	//Create our inverted matrix! Note how that to get a correct inverse matrix,
	//the order of matrices used to form it are inverted, too.
	Matrix4 invVP = GenerateInverseView(cam) * GenerateInverseProjection(aspect, fov, nearPlane, farPlane);

	Matrix4 proj  = cam.BuildProjectionMatrix(aspect);

	//Our mouse position x and y values are in 0 to screen dimensions range,
	//so we need to turn them into the -1 to 1 axis range of clip space.
	//We can do that by dividing the mouse values by the width and height of the
	//screen (giving us a range of 0.0 to 1.0), multiplying by 2 (0.0 to 2.0)
	//and then subtracting 1 (-1.0 to 1.0).
	Vector4 clipSpace = Vector4(
		(screenPos.x / (float)screenSize.x) * 2.0f - 1.0f,
		(screenPos.y / (float)screenSize.y) * 2.0f - 1.0f,
		(screenPos.z),
		1.0f
	);

	//Then, we multiply our clipspace coordinate by our inverted matrix
	Vector4 transformed = invVP * clipSpace;

	//our transformed w coordinate is now the 'inverse' perspective divide, so
	//we can reconstruct the final world space by dividing x,y,and z by w.
	return Vector3(transformed.x / transformed.w, transformed.y / transformed.w, transformed.z / transformed.w);
}

Ray CollisionDetection::BuildRayFromMouse(const PerspectiveCamera& cam) {
	Vector2 screenMouse = Window::GetMouse()->GetAbsolutePosition();
	Vector2i screenSize	= Window::GetWindow()->GetScreenSize();

	//We remove the y axis mouse position from height as OpenGL is 'upside down',
	//and thinks the bottom left is the origin, instead of the top left!
	Vector3 nearPos = Vector3(screenMouse.x,
		screenSize.y - screenMouse.y,
		-0.99999f
	);

	//We also don't use exactly 1.0 (the normalised 'end' of the far plane) as this
	//causes the unproject function to go a bit weird. 
	Vector3 farPos = Vector3(screenMouse.x,
		screenSize.y - screenMouse.y,
		0.99999f
	);

	Vector3 a = Unproject(nearPos, cam);
	Vector3 b = Unproject(farPos, cam);
	Vector3 c = b - a;

	c.Normalise();

	return Ray(cam.GetPosition(), c);
}

Ray CollisionDetection::BuidRayFromCenterOfTheCamera(const PerspectiveCamera& cam) {
	Vector2i screenSize = Window::GetWindow()->GetScreenSize();

	Vector3 nearPos = Vector3((screenSize.x / 2.f),
		(screenSize.y / 2.f),
		-0.99999f
	);

	Vector3 farPos = Vector3((screenSize.x / 2.f),
		(screenSize.y / 2.f),
		0.99999f
	);

	Vector3 a = Unproject(nearPos, cam);
	Vector3 b = Unproject(farPos, cam);
	Vector3 c = b - a;

	c.Normalise();

	return Ray(cam.GetPosition(), c);

}

//http://bookofhook.com/mousepick.pdf
Matrix4 CollisionDetection::GenerateInverseProjection(float aspect, float fov, float nearPlane, float farPlane) {
	Matrix4 m;

	float t = tan(fov*PI_OVER_360);

	float neg_depth = nearPlane - farPlane;

	const float h = 1.0f / t;

	float c = (farPlane + nearPlane) / neg_depth;
	float e = -1.0f;
	float d = 2.0f*(nearPlane*farPlane) / neg_depth;

	m.array[0][0] = aspect / h;
	m.array[1][1] = tan(fov * PI_OVER_360);
	m.array[2][2] = 0.0f;

	m.array[2][3] = 1.0f / d;

	m.array[3][2] = 1.0f / e;
	m.array[3][3] = -c / (d * e);

	return m;
}

/*
And here's how we generate an inverse view matrix. It's pretty much
an exact inversion of the BuildViewMatrix function of the Camera class!
*/
Matrix4 CollisionDetection::GenerateInverseView(const Camera &c) {
	float pitch = c.GetPitch();
	float yaw	= c.GetYaw();
	Vector3 position = c.GetPosition();

	Matrix4 iview =
		Matrix4::Translation(position) *
		Matrix4::Rotation(yaw, Vector3(0, 1, 0)) *
		Matrix4::Rotation(pitch, Vector3(1, 0, 0));

	return iview;
}


/*
If you've read through the Deferred Rendering tutorial you should have a pretty
good idea what this function does. It takes a 2D position, such as the mouse
position, and 'unprojects' it, to generate a 3D world space position for it.

Just as we turn a world space position into a clip space position by multiplying
it by the model, view, and projection matrices, we can turn a clip space
position back to a 3D position by multiply it by the INVERSE of the
view projection matrix (the model matrix has already been assumed to have
'transformed' the 2D point). As has been mentioned a few times, inverting a
matrix is not a nice operation, either to understand or code. But! We can cheat
the inversion process again, just like we do when we create a view matrix using
the camera.

So, to form the inverted matrix, we need the aspect and fov used to create the
projection matrix of our scene, and the camera used to form the view matrix.

*/
Vector3	CollisionDetection::UnprojectScreenPosition(Vector3 position, float aspect, float fov, const PerspectiveCamera& c) {
	//Create our inverted matrix! Note how that to get a correct inverse matrix,
	//the order of matrices used to form it are inverted, too.
	Matrix4 invVP = GenerateInverseView(c) * GenerateInverseProjection(aspect, fov, c.GetNearPlane(), c.GetFarPlane());


	Vector2i screenSize = Window::GetWindow()->GetScreenSize();

	//Our mouse position x and y values are in 0 to screen dimensions range,
	//so we need to turn them into the -1 to 1 axis range of clip space.
	//We can do that by dividing the mouse values by the width and height of the
	//screen (giving us a range of 0.0 to 1.0), multiplying by 2 (0.0 to 2.0)
	//and then subtracting 1 (-1.0 to 1.0).
	Vector4 clipSpace = Vector4(
		(position.x / (float)screenSize.x) * 2.0f - 1.0f,
		(position.y / (float)screenSize.y) * 2.0f - 1.0f,
		(position.z) - 1.0f,
		1.0f
	);

	//Then, we multiply our clipspace coordinate by our inverted matrix
	Vector4 transformed = invVP * clipSpace;

	//our transformed w coordinate is now the 'inverse' perspective divide, so
	//we can reconstruct the final world space by dividing x,y,and z by w.
	return Vector3(transformed.x / transformed.w, transformed.y / transformed.w, transformed.z / transformed.w);
}

