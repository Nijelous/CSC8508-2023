/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#pragma once
#include <unordered_map>

#include "Window.h"
#include "BaseLight.h"
#include "UISystem.h"
#include "Mesh.h"
#include "Shader.h"
#include "MeshAnimation.h"
#include "MeshMaterial.h"

namespace NCL::Rendering {

	constexpr short MAX_POSSIBLE_LIGHTS = 256;

	enum RendererType {
		OGL,
		AGC
	};

	enum class VerticalSyncState {
		VSync_ON,
		VSync_OFF,
		VSync_ADAPTIVE
	};
	class RendererBase {
	public:
		friend class NCL::Window;

		RendererBase(Window& w);
		virtual ~RendererBase();

		virtual bool HasInitialised() const {return true;}

		virtual void Update(float dt) {}

		virtual RendererType GetRendererType() { return mType; }

		void Render() {
			//assert(HasInitialised());
			BeginFrame();
			RenderFrame();
			EndFrame();
			SwapBuffers();
		}

		virtual bool SetVerticalSync(VerticalSyncState s) {
			return false;
		}

		virtual Mesh* LoadMesh(const std::string& name) { return nullptr; }
		virtual void LoadMeshes(unordered_map<std::string, Mesh*>& meshMap, const std::vector<std::string>& details) {}
		virtual Texture* LoadTexture(const std::string& name) { return nullptr; }
		virtual Texture* LoadDebugTexture(const std::string& name) { return nullptr; }
		virtual Shader* LoadShader(const std::string& vertex, const std::string& fragment) { return nullptr; }
		virtual MeshAnimation* LoadAnimation(const std::string& name) { return nullptr; }
		virtual MeshMaterial* LoadMaterial(const std::string& name) { return nullptr; }

		virtual void ClearInstanceObjects() {}
		virtual void FillLightUBO() {}
		virtual void FillTextureDataUBO() {}

		virtual void SetUIObject(CSC8503::UISystem* uiSystem) {}

		virtual void AddLight(Light* lightPtr) {
			if (mLights.size() >= MAX_POSSIBLE_LIGHTS) return;
			mLights.push_back(lightPtr);

		}

		virtual void ClearLights() {
			for (int i = 0; i < mLights.size(); i++) {
				delete(mLights[i]);
			}
			mLights.clear();
		}

		void SetIsGameStarted(bool isGameStarted);

		Vector2i GetWindowSize() const { return windowSize; }


	protected:
		bool mIsGameStarted = false;

		virtual void OnWindowResize(int w, int h) = 0;
		virtual void OnWindowDetach() {}; //Most renderers won't care about this
			
		virtual void BeginFrame()	= 0;
		virtual void RenderFrame()	= 0;
		virtual void EndFrame()		= 0;
		virtual void SwapBuffers()	= 0;

		RendererType mType;

		Window& hostWindow;

		Vector2i windowSize;

		vector<Light*> mLights;
	};
}