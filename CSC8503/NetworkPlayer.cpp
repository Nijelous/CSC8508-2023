//#include "NetworkPlayer.h"
//#include "NetworkedGame.h"
//
//using namespace NCL;
//using namespace CSC8503;
//
//NetworkPlayer::NetworkPlayer(NetworkedGame* game, int num)	: PlayerObject(game->GetGameWorld(), ""){
//	this->game = game;
//	playerNum  = num;
//}
//
//NetworkPlayer::NetworkPlayer(NetworkedGame* game, int num, const std::string& objName) : PlayerObject(game->GetGameWorld(),objName) {
//	this->game = game;
//	playerNum = num;
//}
//
//NetworkPlayer::~NetworkPlayer()	{
//
//}
//
//void NetworkPlayer::OnCollisionBegin(GameObject* otherObject) {
//	if (game) {
//		if (dynamic_cast<NetworkPlayer*>(otherObject))
//		{
//			game->OnPlayerCollision(this, (NetworkPlayer*)otherObject);
//		}
//	}
//}
//
//void NetworkPlayer::MovePlayer(float dt){
//	PlayerObject::MovePlayer(dt);
//}
