#ifdef USEGL
#pragma once
#include "OGLRenderer.h"
#include "OGLTexture.h"
#include "Matrix3.h"
#include <vector>
#include "PlayerObject.h"

namespace NCL {
    namespace CSC8503 {
        class GameTechRenderer;
        class GameObject;

        enum MiniMapItemType
        {
            MINIMAP_NONE,
            MINIMAP_BUFF = 0x10,
            MINIMAP_BUFF_DISGUISEBUFF,
            MINIMAP_BUFF_SLOW,
            MINIMAP_BUFF_MAKESOUND,
            MINIMAP_BUFF_SLOWEVERYONEELSE,
            MINIMAP_BUFF_EVERYONEELSEMAKESSOUND,
            MINIMAP_BUFF_SILENTSPRINT,
            MINIMAP_BUFF_SPEED,
            MINIMAP_BUFF_STUN,
            MINIMAP_BUFF_FLAGSIGHT,
            MINIMAP_BUFF_SCREWDRIVER,
            MINIMAP_BUFF_DOORKEY,
            MINIMAP_BUFF_SOUNDEMITTER,
            MINIMAP_ITEM = 0x20,
            MINIMAP_ITEM_FLAG,
            MINIMAP_ITEM_STUNITEM,
            MINIMAP_ITEM_DISGUISE,
            MINIMAP_GUARD = 0x30,
            MINIMAP_FLAG = 0x40,
            MINIMAP_NUM,
        };
        struct MiniMapItem
        {
            GameObject* obj;
            MiniMapItemType type;
            //item location, in meters
            Vector2 pos;
            bool visible{};
        };
        /**
         * @brief the unit is meter, assume minimap is square
         *
         */
        class MiniMap : public PlayerBuffsObserver {
        public:
            struct MeshRenderable
            {
                std::vector<Vector2> initPositions;
                std::vector<Vector2> positions;
                std::vector<Vector2> texcoords;
                GLenum topology = GL_TRIANGLES;
                GLuint vao{};
                GLuint vboPos{};
                GLuint vboTexcoord{};
                Vector3 color{};

                ~MeshRenderable();
                void Create();
                void Update();
                void Draw();
                void Draw(int start, int count);
            };

            MiniMap(GameTechRenderer* renderer);
            ~MiniMap();

            void Render();

            void SetItemTexture(MiniMapItemType itemType, Texture* tex);

            void SetCenter(float x, float y);
            void SetRadius(float r);

            void SetViewRadius(float r);

        protected:
            void Initialize();

            void SearchWorld();
            void UpdateItemsState();
            void UpdatePlayerState();

            void CreateItemRenderable();
            void CreateBackGround();

            void RenderMap();
            void RenderBorder();
            void RenderPlayer();
            void RenderItems();
            void RenderWalls();

            Matrix3 GetMinimapToViewportMatrix() const;
            Matrix3 GetWorldToMinimapMatrix() const;

            Matrix3 GetItemTransformationMatrix(MiniMapItem const& item, bool& visible) const;

            void UpdatePlayerBuffsObserver(BuffEvent buffEvent, int playerNo) override;

        private:
            bool mInitialized = false;
            bool mItemVisible = false;
            int worldStateID{ 0 };

            OGLShader* mUnlitColorShader;
            OGLShader* mWallShader;
            OGLShader* mIconShader;

            float mPlayerYawAngle{};
            Maths::Vector2 mPlayerPosition;    // player location

            float mViewRadius;

            Vector2 mCenter;
            float mRadius;

            // map size in meters
            Vector2 mWorldPmin;
            Vector2 mWorldPmax;

            std::vector<MiniMapItem> mItems;
            Texture* mItemTextures[MINIMAP_NUM];
            MeshRenderable mItemRenderable;

            GameTechRenderer* mRenderer;

            MeshRenderable mPlayer;
            MeshRenderable mBackGround;
            MeshRenderable mWall;
            MeshRenderable mInteractableDoor;
            MeshRenderable mPrisonDoor;

            Matrix3 mWorldToMinimapMatrix;
            Matrix3 mMinimapToViewportMatrix;
            Matrix3 mPlayerRotationMatrix;
        };
    }
}
#endif