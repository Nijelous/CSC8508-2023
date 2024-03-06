#ifdef USEGL
#pragma once

namespace NCL {
	using namespace Maths;
	namespace CSC8503 {
		class GameObject;
		class NetworkState	{
		public:
			NetworkState();
			virtual ~NetworkState();

			int GetNetworkState() { return stateID; }
			void SetNetworkState(int newID) { stateID = newID; }

			Quaternion GetObjectOrientation() { return orientation; }
			void SetOrientation(Quaternion newOri) { orientation = newOri; }

			Vector3 GetObjectPos() { return position; }
			void SetPosition(Vector3 newPos) { position = newPos; }

			Vector3		position;
			Quaternion	orientation;
			int			stateID;
		};
	}
}

#endif