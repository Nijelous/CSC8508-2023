#ifdef USEGL

#include "SoundObject.h"

using namespace NCL::CSC8503;
using namespace FMOD;

SoundObject::SoundObject(Channel* channel) {
	mChannel = channel;
	mIsTriggered = false;
	mIsClosed = false;
	mIsLocked = false;
	mChannels.emplace_back(channel);
}
SoundObject::SoundObject() {
	mIsTriggered = false;
	mIsClosed = false;
	mIsLocked = false;
}

SoundObject::~SoundObject() {
	mChannels.clear();
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

void SoundObject::TriggerSoundEvent() {
	mIsTriggered = true;
}

void SoundObject::CloseDoorTriggered() {
	mIsClosed = true;
}

void SoundObject::LockDoorTriggered() {
	mIsLocked = true;
}

void SoundObject::SetNotTriggered() {
	mIsTriggered = false;
}

void SoundObject::CloseDoorFinished() {
	mIsClosed = false;
}

void SoundObject::LockDoorFinished() {
	mIsLocked = false;
}

bool SoundObject::GetisTiggered(){
	return mIsTriggered;
}

bool SoundObject::GetIsClosed() {
	return mIsClosed;
}

bool SoundObject::GetIsLocked() {
	return mIsLocked;
}
#endif