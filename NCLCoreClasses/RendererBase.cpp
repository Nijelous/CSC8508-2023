#include "RendererBase.h"

using namespace NCL;
using namespace Rendering;

RendererBase::RendererBase(Window& window) : hostWindow(window)	{

}


RendererBase::~RendererBase()
{
}

void RendererBase::SetIsGameStarted(const bool isGameStarted) {
	mIsGameStarted = isGameStarted;
}
