#include "SoundObject.h"
#include <fmod.hpp>

using namespace NCL::CSC8503;
using namespace FMOD;

SoundObject::SoundObject(Channel* channel) {
	mChannel = channel;
	mChannels.emplace_back(channel);
}

SoundObject::~SoundObject() {

}

void SoundObject::AddChannel(Channel* channel) {
	mChannels.emplace_back(channel);
}

std::vector<Channel*> SoundObject::GetChannels() {
	return mChannels;
}

Channel* SoundObject::GetChannel() {
	return mChannel;
}