#include "NetworkPlayer.h"

#include "DebugNetworkedGame.h"
#include "GameClient.h"
#include "NetworkedGame.h"
#include "NetworkObject.h"
#include "PhysicsObject.h"

using namespace NCL;
using namespace CSC8503;

namespace {
    constexpr int MOVE_FORWARD_INDEX = 0; 
    constexpr int MOVE_LEFT_INDEX = 1; 
    constexpr int MOVE_BACKWARDS_INDEX = 2; 
    constexpr int MOVE_RIGHT_INDEX = 3; 
}

NetworkPlayer::NetworkPlayer(NetworkedGame* game, int num) : PlayerObject(game->GetLevelManager()->GetGameWorld(), ""){
    //this->game = game;
    playerNum = num;
}

NetworkPlayer::NetworkPlayer(DebugNetworkedGame* game, int num, const std::string& objName) : PlayerObject(
    game->GetLevelManager()->GetGameWorld(), objName) {
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

void NetworkPlayer::SetPlayerInput(const PlayerInputs& playerInputs){
    mPlayerInputs = playerInputs;
    mIsClientInputReceived = true;
    mCameraYaw = playerInputs.cameraYaw;
}

void NetworkPlayer::SetIsLocalPlayer(bool isLocalPlayer){
    mIsLocalPlayer = isLocalPlayer;
}

void NetworkPlayer::SetCameraYaw(float cameraYaw){
    mCameraYaw = cameraYaw;
}

void NetworkPlayer::ResetPlayerInput(){
    mPlayerInputs = PlayerInputs();
}

void NetworkPlayer::UpdateObject(float dt){
    MovePlayer(dt);
    if (mIsLocalPlayer){
        AttachCameraToPlayer(game->GetLevelManager()->GetGameWorld());
        mCameraYaw = game->GetLevelManager()->GetGameWorld()->GetMainCamera().GetYaw();
    }

    if (mIsLocalPlayer || game->GetIsServer()){
        MatchCameraRotation(mCameraYaw);   
    }
}

void NetworkPlayer::MovePlayer(float dt){
    bool isServer = game->GetIsServer();
    
    if (mIsLocalPlayer){
        const Vector3 playerPos = mTransform.GetPosition();

        Debug::Print("Player Position: " + std::to_string(playerPos.x) + ", " + std::to_string(playerPos.y) + ", " + std::to_string(playerPos.z), Vector2(5, 30), Debug::MAGENTA);  
        
        if (Window::GetKeyboard()->KeyDown(KeyCodes::W))
            mPlayerInputs.movementButtons[MOVE_FORWARD_INDEX] = true;
 
        if (Window::GetKeyboard()->KeyDown(KeyCodes::A))
            mPlayerInputs.movementButtons[MOVE_LEFT_INDEX] = true;
         
        if (Window::GetKeyboard()->KeyDown(KeyCodes::S))
            mPlayerInputs.movementButtons[MOVE_BACKWARDS_INDEX] = true;
           
        if (Window::GetKeyboard()->KeyDown(KeyCodes::D))
            mPlayerInputs.movementButtons[MOVE_RIGHT_INDEX] = true;
        
        if (Window::GetKeyboard()->KeyDown(KeyCodes::SHIFT))
            mPlayerInputs.isSprinting = true;
        
        if (Window::GetKeyboard()->KeyDown(KeyCodes::CONTROL))
            mPlayerInputs.isCrouching = true;
            
        mPlayerInputs.cameraYaw = game->GetLevelManager()->GetGameWorld()->GetMainCamera().GetYaw();
    }

    if (isServer == false && mIsLocalPlayer){
        //TODO(eren.degirmenci): is dynamic casting here is bad ?
        const Vector3 fwdAxis = mGameWorld->GetMainCamera().GetForwardVector();
        const Vector3 rightAxis = mGameWorld->GetMainCamera().GetRightVector();
        mPlayerInputs.fwdAxis = fwdAxis;
        mPlayerInputs.rightAxis = rightAxis;
        game->GetClient()->WriteAndSendClientInputPacket(0, mPlayerInputs);
    }
    else{
        HandleMovement(dt, mPlayerInputs);
        mIsClientInputReceived = false;
        ResetPlayerInput();
    }
}

void NetworkPlayer::HandleMovement(float dt, const PlayerInputs& playerInputs){

    Vector3 fwdAxis;
    Vector3 rightAxis;
    if (mIsLocalPlayer){
        fwdAxis = mGameWorld->GetMainCamera().GetForwardVector();
        rightAxis = mGameWorld->GetMainCamera().GetRightVector();
    }
    else{
        fwdAxis = playerInputs.fwdAxis;
        rightAxis = playerInputs.rightAxis;
    }
    
    if (playerInputs.movementButtons[MOVE_FORWARD_INDEX])
        mPhysicsObject->AddForce(fwdAxis * mMovementSpeed);

    if (playerInputs.movementButtons[MOVE_LEFT_INDEX])
        mPhysicsObject->AddForce(rightAxis * mMovementSpeed);
    
    if (playerInputs.movementButtons[MOVE_BACKWARDS_INDEX])
        mPhysicsObject->AddForce(fwdAxis * mMovementSpeed);

    if (playerInputs.movementButtons[MOVE_RIGHT_INDEX])
        mPhysicsObject->AddForce(rightAxis * mMovementSpeed);

    ActivateSprint(playerInputs.isSprinting);
    ToggleCrouch(playerInputs.isCrouching);
    
    StopSliding();
}
