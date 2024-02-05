#include "RenderObject.h"
#include "Mesh.h"

using namespace NCL::CSC8503;
using namespace NCL;

RenderObject::RenderObject(Transform* parentTransform, Mesh* mesh, Texture* tex, Shader* shader, float cullSphereRadius) {
	if (!tex) {
		bool a = true;
	}
	this->transform	= parentTransform;
	this->mesh		= mesh;
	this->texture	= tex;
	this->shader	= shader;
	this->colour	= Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	this->mCullSphereRadius = cullSphereRadius;
	mSqDistToCam = FLT_MAX;
}

RenderObject::RenderObject(Transform* parentTransform, Mesh* mesh, Texture* tex, Shader* shader, Vector4 colour, float cullSphereRadius) {
	if (!tex) {
		bool a = true;
	}
	this->transform = parentTransform;
	this->mesh = mesh;
	this->texture = tex;
	this->shader = shader;	
	this->colour = colour;
	this->mCullSphereRadius = cullSphereRadius;
	mSqDistToCam = FLT_MAX;
}

RenderObject::~RenderObject() {

}