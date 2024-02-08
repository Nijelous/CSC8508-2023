#include "NetworkPlayer.h"

#include "DebugNetworkedGame.h"
#include "GameClient.h"
#include "NetworkedGame.h"
#include "NetworkObject.h"
#include "PhysicsObject.h"

using namespace NCL;
using namespace CSC8503;

NetworkPlayer::NetworkPlayer(NetworkedGame* game, int num) : PlayerObject(game->GetGameWorld(), ""){
    this->game = game;
    playerNum = num;
}

NetworkPlayer::NetworkPlayer(NetworkedGame* game, int num, const std::string& objName) : PlayerObject(
    game->GetGameWorld(), objName){
    this->game = game;
    playerNum = num;
}

NetworkPlayer::~NetworkPlayer(){
}

void NetworkPlayer::OnCollisionBegin(GameObject* otherObject){
    if (game){
        if (dynamic_cast<NetworkPlayer*>(otherObject)){
            game->OnPlayerCollision(this, (NetworkPlayer*)otherObject);
        }
    }
}

void NetworkPlayer::MovePlayer(float dt){
    PlayerInputs playerInputs;

    if (Window::GetKeyboard()->KeyDown(KeyCodes::W))
        playerInputs.movementButtons[0] = true;
    if (Window::GetKeyboard()->KeyDown(KeyCodes::A))
        playerInputs.movementButtons[1] = true;
    if (Window::GetKeyboard()->KeyDown(KeyCodes::S))
        playerInputs.movementButtons[2] = true;
    if (Window::GetKeyboard()->KeyDown(KeyCodes::D))
        playerInputs.movementButtons[3] = true;
    if (Window::GetKeyboard()->KeyDown(KeyCodes::SHIFT))
        playerInputs.isSprinting = true;
    if (Window::GetKeyboard()->KeyDown(KeyCodes::CONTROL))
        playerInputs.isCrouching = true;
    
    auto* server = ((DebugNetworkedGame*)mGameWorld)->GetServer();
    if (server == nullptr){
        //TODO(eren.degirmenci): is dynamic casting here is bad ?
        ((DebugNetworkedGame*)mGameWorld)->GetClient()->WriteAndSendClientInputPacket(0, playerInputs);
    }

    HandleMovement(dt, playerInputs);
}

void NetworkPlayer::HandleMovement(float dt, const PlayerInputs& playerInputs){
    Vector3 fwdAxis = mGameWorld->GetMainCamera().GetForwardVector();
    Vector3 rightAxis = mGameWorld->GetMainCamera().GetRightVector();
    
    if (playerInputs.movementButtons[0])
        mPhysicsObject->AddForce(fwdAxis * mMovementSpeed);

    if (playerInputs.movementButtons[1])
        mPhysicsObject->AddForce(rightAxis * mMovementSpeed);
    
    if (playerInputs.movementButtons[2])
        mPhysicsObject->AddForce(fwdAxis * mMovementSpeed);

    if (playerInputs.movementButtons[3])
        mPhysicsObject->AddForce(rightAxis * mMovementSpeed);

    ActivateSprint(playerInputs.isSprinting);
    ToggleCrouch(playerInputs.isCrouching);
    
    StopSliding();
}
