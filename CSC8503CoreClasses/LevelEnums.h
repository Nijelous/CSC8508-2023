#pragma once
namespace NCL {
	namespace CSC8503 {
		constexpr int MAX_PLAYERS = 4;
		enum TileType {
			Wall,
			Floor,
			CornerWall,
			OutsideFloor
		};
		enum RoomType {
			Small,
			Medium,
			Large,
			LShape,
			INVALID
		};
		enum DecorationType {
			Desk,
			Painting,
			PlantTall,
			PlantPot,
			Bookshelf,
			Bed,
			Chair,
			CeilingLight,
			TV,
			Table,
			TableSmall,
			Shelf,
			Sofa,
			PoolTable
		};

		class LevelEnums {

		};
	}
}

