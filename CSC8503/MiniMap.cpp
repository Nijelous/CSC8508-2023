#ifdef USEGL
// MiniMap.cpp
#include "MiniMap.h"

#include <iostream>

#include "GameTechRenderer.h"
#include "GameWorld.h"
#include "OGLTexture.h"
#include "Matrix3.h"
#include "PlayerObject.h"
#include "InteractableDoor.h"
#include "PrisonDoor.h"
#include "GuardObject.h"
#include "InventoryBuffSystem/FlagGameObject.h"
#include "InventoryBuffSystem/PickupGameObject.h"
#include "InventoryBuffSystem/PlayerBuffs.h"

namespace NCL {
    namespace CSC8503 {
        MiniMap::MeshRenderable::~MeshRenderable() {
            glDeleteVertexArrays(1, &vao);
            glDeleteBuffers(1, &vboPos);
        }
        void MiniMap::MeshRenderable::Create() {
            glGenVertexArrays(1, &vao);
            glBindVertexArray(vao);
            if (!positions.empty())
            {
                glGenBuffers(1, &vboPos);
                glBindBuffer(GL_ARRAY_BUFFER, vboPos);
                glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(Vector2), positions.data(), GL_STATIC_DRAW);
                glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
                glEnableVertexAttribArray(0);
            }
            if (!texcoords.empty())
            {
                glGenBuffers(1, &vboTexcoord);
                glBindBuffer(GL_ARRAY_BUFFER, vboTexcoord);
                glBufferData(GL_ARRAY_BUFFER, texcoords.size() * sizeof(Vector2), texcoords.data(), GL_STATIC_DRAW);
                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
                glEnableVertexAttribArray(1);
            }
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
        }
        void MiniMap::MeshRenderable::Update() {
            if (vboPos)
            {
                glBindBuffer(GL_ARRAY_BUFFER, vboPos);
                glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(Vector2), positions.data(), GL_STATIC_DRAW);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
            }
            if (vboTexcoord)
            {
                glBindBuffer(GL_ARRAY_BUFFER, vboTexcoord);
                glBufferData(GL_ARRAY_BUFFER, texcoords.size() * sizeof(Vector2), texcoords.data(), GL_STATIC_DRAW);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
            }
        }
        void MiniMap::MeshRenderable::Draw() {
            glBindVertexArray(vao);
            glDrawArrays(topology, 0, positions.size());
            glBindVertexArray(0);
        }
        void MiniMap::MeshRenderable::Draw(int start, int count)
        {
            glBindVertexArray(vao);
            glDrawArrays(topology, start, count);
            glBindVertexArray(0);
        }

        MiniMap::MiniMap(GameTechRenderer* renderer) : mRenderer(renderer),
            mCenter(-0.8f, -0.55f),
            mRadius(0.3f),
            mViewRadius(100.f)
        {
            mRenderer->SetMiniMap(this);
            Initialize();
        }

        MiniMap::~MiniMap() {
            delete mIconShader;
            delete mUnlitColorShader;
            delete[] mItemTextures;
        }
        void MiniMap::Initialize() {
            if (!mInitialized) {
                mUnlitColorShader = static_cast<OGLShader*>(mRenderer->LoadShader("minimap/common.vert", "minimap/unlitcolor.frag"));
                mWallShader = static_cast<OGLShader*>(mRenderer->LoadShader("minimap/common.vert", "minimap/wall.frag"));
                mIconShader = static_cast<OGLShader*>(mRenderer->LoadShader("minimap/common.vert", "minimap/tex.frag"));
                Texture* baseBuff = mRenderer->LoadTexture("buff.png");
                for (int i = 0; i < MINIMAP_NUM; ++i) {
                    mItemTextures[i] = baseBuff;
                }

                mItemTextures[MINIMAP_BUFF_FLAGSIGHT] = mRenderer->LoadTexture("radar.png");
                mItemTextures[MINIMAP_GUARD] = mRenderer->LoadTexture("guard.png");
                mItemTextures[MINIMAP_ITEM_FLAG] = mRenderer->LoadTexture("item.png");

                CreateItemRenderable();
                CreateBackGround();

                mPlayer.positions = {
                    {0.,-0.05},
                    {0.1,-0.1},
                    {0,0.2},
                    {-0.1,-0.1},
                };
                mPlayer.Create();
                mInitialized = true;
            }
        }
        void MiniMap::SetItemTexture(MiniMapItemType itemType, Texture* tex) {
            mItemTextures[itemType] = tex;
        }
        void MiniMap::SetViewRadius(float r) { mViewRadius = r; }

        void MiniMap::SetCenter(float x, float y) {
            mCenter = { x,y };
        }
        void MiniMap::SetRadius(float r) {
            mRadius = r;
        }
        void MiniMap::CreateItemRenderable() {
            mItemRenderable.positions = {
                {-1,-1},
                {1,-1},
                {1,1},
                {-1,1},
            };
            mItemRenderable.texcoords = {
                {0,1},
                {1,1},
                {1,0},
                {0,0},
            };
            mItemRenderable.topology = GL_TRIANGLE_FAN;
            mItemRenderable.Create();
        }
        void MiniMap::CreateBackGround() {
            std::vector<float> vertices;
            mBackGround.positions.emplace_back(0, 0);
            for (int i = 0;i <= 32;++i)
            {
                auto theta = float(i) / 16 * 3.1415926535;
                auto cosTheta = std::cos(theta);
                auto sinTheta = std::sin(theta);
                mBackGround.positions.emplace_back(cosTheta, sinTheta);
            }
            mBackGround.topology = GL_TRIANGLE_FAN;
            mBackGround.color = { 0.2,0.2,0.2 };
            mBackGround.Create();
        }
        void MiniMap::UpdatePlayerState() {
            //update player transformation
            auto&& gameWorld = mRenderer->gameWorld;
            auto p = gameWorld.GetMainCamera().GetPosition();
            mPlayerYawAngle = gameWorld.GetMainCamera().GetYaw();
            mPlayerPosition = { p.x, p.z };

            mMinimapToViewportMatrix = GetMinimapToViewportMatrix();

            auto rad = DegreesToRadians(-mPlayerYawAngle);
            auto cosTheta = std::cos(rad);
            auto sinTheta = std::sin(rad);

            mPlayerRotationMatrix.SetColumn(0, Vector3(cosTheta, sinTheta, 0));
            mPlayerRotationMatrix.SetColumn(1, Vector3(-sinTheta, cosTheta, 0));
            mPlayerRotationMatrix.SetColumn(2, Vector3(0, 0, 1));
        }
        void MiniMap::UpdateItemsState() {
            // check item visibility
            for (auto&& e : mItems)
            {
                auto&& pos = e.obj->GetTransform().GetPosition();
                e.pos.x = pos.x;
                e.pos.y = pos.z;
            }
        }

        void MiniMap::SearchWorld() {
            auto&& gameWorld = mRenderer->gameWorld;
            if (worldStateID == gameWorld.GetWorldStateID())return;
            worldStateID = gameWorld.GetWorldStateID();

            GameObjectIterator first;
            GameObjectIterator last;

            gameWorld.GetObjectIterators(first, last);

            //set map size
            mWorldPmin = { FLT_MAX,FLT_MAX };
            mWorldPmax = { -FLT_MAX,-FLT_MAX };

            Vector2 tempPmin;
            Vector2 tempPmax;

            bool flag = true;
            auto mergeAABB = [](Vector2& pmax, Vector2& pmin, GameObject* obj) {
                auto aabb = reinterpret_cast<AABBVolume const*>(obj->GetBoundingVolume());
                auto&& t = obj->GetTransform();
                auto offsetA = aabb->GetOffset();
                auto halfSizeA = aabb->GetHalfDimensions() - Vector3(2, 0, 0);
                Vector3 a = offsetA - halfSizeA;
                Vector3 b = offsetA + halfSizeA;
                auto&& m = t.GetMatrix();
                Vector3 points[] = {
                    { a },
                { b.x,a.y,a.z },
                { a.x,b.y,a.z },
                { b.x,b.y,a.z },
                { b },
                { a.x,b.y,b.z },
                { b.x,a.y,b.z },
                { a.x,a.y,b.z } };
                for (size_t i = 0;i < 8;++i)
                {
                    auto p = m * points[i];
                    pmin.x = (std::min)(p.x, pmin.x);
                    pmin.y = (std::min)(p.z, pmin.y);
                    pmax.x = (std::max)(p.x, pmax.x);
                    pmax.y = (std::max)(p.z, pmax.y);
                }
                };


            for (;first != last;++first)
            {
                if ((*first)->GetName() == "Floor")
                {
                    mergeAABB(mWorldPmax, mWorldPmin, (*first));
                }
                else if ((*first)->GetName() == "Wall")
                {
                    tempPmin = { FLT_MAX,FLT_MAX };
                    tempPmax = { -FLT_MAX,-FLT_MAX };
                    mergeAABB(tempPmax, tempPmin, (*first));
                    mWall.positions.emplace_back(tempPmin);
                    mWall.positions.emplace_back(tempPmax.x, tempPmin.y);
                    mWall.positions.emplace_back(tempPmax);
                    mWall.positions.emplace_back(tempPmin);
                    mWall.positions.emplace_back(tempPmax);
                    mWall.positions.emplace_back(tempPmin.x, tempPmax.y);
                }

                //door
                if (dynamic_cast<InteractableDoor const*>(*first))
                {
                    tempPmin = { FLT_MAX,FLT_MAX };
                    tempPmax = { -FLT_MAX,-FLT_MAX };
                    mergeAABB(tempPmax, tempPmin, (*first));
                    mInteractableDoor.positions.emplace_back(tempPmin);
                    mInteractableDoor.positions.emplace_back(tempPmax.x, tempPmin.y);
                    mInteractableDoor.positions.emplace_back(tempPmax);
                    mInteractableDoor.positions.emplace_back(tempPmin);
                    mInteractableDoor.positions.emplace_back(tempPmax);
                    mInteractableDoor.positions.emplace_back(tempPmin.x, tempPmax.y);
                }
                else if (dynamic_cast<PrisonDoor const*>(*first))
                {
                    tempPmin = { FLT_MAX,FLT_MAX };
                    tempPmax = { -FLT_MAX,-FLT_MAX };
                    mergeAABB(tempPmax, tempPmin, (*first));
                    mPrisonDoor.positions.emplace_back(tempPmin);
                    mPrisonDoor.positions.emplace_back(tempPmax.x, tempPmin.y);
                    mPrisonDoor.positions.emplace_back(tempPmax);
                    mPrisonDoor.positions.emplace_back(tempPmin);
                    mPrisonDoor.positions.emplace_back(tempPmax);
                    mPrisonDoor.positions.emplace_back(tempPmin.x, tempPmax.y);
                }
                else if (dynamic_cast<GuardObject const*>(*first))
                {
                    MiniMapItem item;
                    item.type = MINIMAP_GUARD;
                    item.obj = (*first);
                    mItems.emplace_back(item);
                }
                else if (dynamic_cast<FlagGameObject const*>(*first))
                {
                    MiniMapItem item;
                    item.type = MINIMAP_ITEM_FLAG;
                    item.obj = (*first);
                    mItems.emplace_back(item);
                }
                else if (auto pickUp = dynamic_cast<PickupGameObject const*>(*first))
                {
                    MiniMapItem item;
                    item.obj = (*first);
                    if (pickUp->IsBuff())
                    {
                        auto buff = pickUp->GetBuff();
                        switch (buff)
                        {
                        default:
                        case PlayerBuffs::buff::Null:
                            item.type = MINIMAP_BUFF;
                            break;
                        case PlayerBuffs::buff::disguiseBuff:
                            item.type = MINIMAP_BUFF_DISGUISEBUFF;
                            break;
                        case PlayerBuffs::buff::slow:
                            item.type = MINIMAP_BUFF_SLOW;
                            break;
                        case PlayerBuffs::buff::makeSound:
                            item.type = MINIMAP_BUFF_MAKESOUND;
                            break;
                        case PlayerBuffs::buff::slowEveryoneElse:
                            item.type = MINIMAP_BUFF_SLOWEVERYONEELSE;
                            break;
                        case PlayerBuffs::buff::everyoneElseMakesSound:
                            item.type = MINIMAP_BUFF_EVERYONEELSEMAKESSOUND;
                            break;
                        case PlayerBuffs::buff::silentSprint:
                            item.type = MINIMAP_BUFF_SILENTSPRINT;
                            break;
                        case PlayerBuffs::buff::speed:
                            item.type = MINIMAP_BUFF_SPEED;
                            break;
                        case PlayerBuffs::buff::stun:
                            item.type = MINIMAP_BUFF_STUN;
                            break;
                        case PlayerBuffs::buff::flagSight:
                            item.type = MINIMAP_BUFF_FLAGSIGHT;
                            break;
                        }
                    }
                    else
                    {
                        auto itemType = pickUp->GetItem();
                        switch (itemType)
                        {
                        default:
                        case PlayerInventory::item::none:
                            item.type = MINIMAP_ITEM;
                            break;
                        case PlayerInventory::item::disguise:
                            item.type = MINIMAP_ITEM_DISGUISE;
                            break;
                        case PlayerInventory::item::soundEmitter:
                            item.type = MINIMAP_BUFF_SOUNDEMITTER;
                            break;
                        case PlayerInventory::item::flag:
                            item.type = MINIMAP_ITEM_FLAG;
                            break;
                        case PlayerInventory::item::screwdriver:
                            item.type = MINIMAP_BUFF_SCREWDRIVER;
                            break;
                        case PlayerInventory::item::doorKey:
                            item.type = MINIMAP_BUFF_DOORKEY;
                            break;
                        case PlayerInventory::item::stunItem:
                            item.type = MINIMAP_ITEM_STUNITEM;
                            break;
                        }

                    }
                    mItems.emplace_back(item);
                }
            }
            mWall.Create();
            mInteractableDoor.Create();
            mInteractableDoor.color = { 0,1,1 };
            mPrisonDoor.Create();
            mPrisonDoor.color = { 1,0,0 };
        }

        void MiniMap::Render() {
            auto&& gameWorld = mRenderer->gameWorld;
            if (gameWorld.GetWorldStateID() <= 0)return;

            SearchWorld();
            UpdatePlayerState();
            mWorldToMinimapMatrix = GetWorldToMinimapMatrix();
            UpdateItemsState();

            auto isDepthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
            if (isDepthTestEnabled)
                glDisable(GL_DEPTH_TEST);

            auto isBlendEnabled = glIsEnabled(GL_BLEND);
            if (!isBlendEnabled)
                glEnable(GL_BLEND);

            RenderMap();
            RenderWalls();
            RenderItems();
            RenderPlayer();
            RenderBorder();

            // Unbind the texture
            glBindTexture(GL_TEXTURE_2D, 0);
            if (isDepthTestEnabled)
                glEnable(GL_DEPTH_TEST);
            if (!isBlendEnabled)
                glDisable(GL_BLEND);
        }
        Matrix3 MiniMap::GetMinimapToViewportMatrix() const {
            Vector2i windowSize = mRenderer->GetWindowSize();
            float c = float(windowSize.y) / windowSize.x;
            Matrix3 M = Matrix3::Scale(Maths::Vector3(mRadius * c, mRadius, 1));;
            M.SetColumn(2, Vector3(mCenter.x, mCenter.y, 1));
            return M;
        }
        Matrix3 MiniMap::GetWorldToMinimapMatrix() const
        {
            Matrix3 M;
            M.SetColumn(2, { -mPlayerPosition.x,-mPlayerPosition.y,1 });
            auto c = 1. / mViewRadius;
            auto m = mPlayerRotationMatrix.Transposed();
            return Matrix3::Scale(Maths::Vector3(c, -c, 1)) * m * M;
        }
        Matrix3 MiniMap::GetItemTransformationMatrix(MiniMapItem const& item, bool& visible) const {
            auto p = mWorldToMinimapMatrix * Vector3(item.pos, 1);
            Matrix3 M;
            visible = false;
            if (p.x * p.x + p.y * p.y > 1)
                return M;
            if (item.obj->IsActive())
            {
                if ((item.type & 0xF0) == MINIMAP_ITEM)
                {
                    if (!mItemVisible)
                        return M;
                }
            }
            else return M;

            visible = true;

            M.SetColumn(2, p);

            M = mMinimapToViewportMatrix * M;

            //item size
            float s = 0.1;
            M = M * Matrix3::Scale(Vector3(s, s, 1));
            return M;
        }
        void MiniMap::RenderMap() {
            mRenderer->BindShader(*mUnlitColorShader);
            glUniformMatrix3fv(glGetUniformLocation(mUnlitColorShader->GetProgramID(), "M"), 1, false, (float*)&mMinimapToViewportMatrix);
            glUniform3fv(glGetUniformLocation(mUnlitColorShader->GetProgramID(), "color"), 1, (float*)&mBackGround.color);
            glBindVertexArray(mBackGround.vao);
            glDrawArrays(GL_TRIANGLE_FAN, 0, mBackGround.positions.size());
            glBindVertexArray(0);
        }
        void MiniMap::RenderBorder()
        {
            mRenderer->BindShader(*mUnlitColorShader);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            float color[]{ 0.0,1.0,0.0 };
            glUniform3fv(glGetUniformLocation(mUnlitColorShader->GetProgramID(), "color"), 1, color);
            glBindVertexArray(mBackGround.vao);
            glDrawArrays(GL_LINE_LOOP, 1, mBackGround.positions.size() - 1);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glBindVertexArray(0);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        void MiniMap::RenderItems() {
            mRenderer->BindShader(*mIconShader);
            glBindVertexArray(mItemRenderable.vao);

            for (auto&& e : mItems) {
                bool visible = true;
                auto  M = GetItemTransformationMatrix(e, visible);
                if (visible)
                {
                    glBindTexture(GL_TEXTURE_2D, static_cast<OGLTexture*>(mItemTextures[e.type])->GetObjectID());
                    glUniformMatrix3fv(glGetUniformLocation(mIconShader->GetProgramID(), "M"), 1, false, (float*)&M);
                    glDrawArrays(mItemRenderable.topology, 0, mItemRenderable.positions.size());
                }
            }
            glBindVertexArray(0);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        void MiniMap::RenderPlayer() {
            mRenderer->BindShader(*mUnlitColorShader);
            glUniformMatrix3fv(glGetUniformLocation(mUnlitColorShader->GetProgramID(), "M"), 1, false, (float*)&mMinimapToViewportMatrix);

            float color[]{ 1.0,1.0,1.0 };
            glUniform3fv(glGetUniformLocation(mUnlitColorShader->GetProgramID(), "color"), 1, color);

            // Draw the quad
            mPlayer.topology = GL_TRIANGLE_FAN;
            mPlayer.Draw();

            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            float color2[]{ 0,0,0 };
            glUniform3fv(glGetUniformLocation(mUnlitColorShader->GetProgramID(), "color"), 1, color2);
            mPlayer.topology = GL_LINE_LOOP;
            mPlayer.Draw();
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        void MiniMap::RenderWalls() {
            mRenderer->BindShader(*mWallShader);
            //draw walls
            auto m = mMinimapToViewportMatrix * mWorldToMinimapMatrix;
            glUniformMatrix3fv(glGetUniformLocation(mWallShader->GetProgramID(), "M"), 1, false, (float*)&m);
            glUniform1fv(glGetUniformLocation(mWallShader->GetProgramID(), "viewRadius"), 1, (float*)&mViewRadius);
            glUniform2fv(glGetUniformLocation(mWallShader->GetProgramID(), "mPlayerLocation"), 1, (float*)&mPlayerPosition);

            glUniform3fv(glGetUniformLocation(mWallShader->GetProgramID(), "color"), 1, (float*)&mPrisonDoor.color);
            mPrisonDoor.Draw();
            glUniform3fv(glGetUniformLocation(mWallShader->GetProgramID(), "color"), 1, (float*)&mInteractableDoor.color);
            mInteractableDoor.Draw();
            glUniform3fv(glGetUniformLocation(mWallShader->GetProgramID(), "color"), 1, (float*)&mWall.color);
            mWall.Draw();
        }
        void MiniMap::UpdatePlayerBuffsObserver(BuffEvent buffEvent, int playerNo) {
            if (buffEvent == flagSightApplied)
            {
                mItemVisible = true;
            }
            else if (buffEvent == flagSightRemoved)
            {
                mItemVisible = false;
            }
        }
    }
}
#endif