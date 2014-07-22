#include "tr_local.h"

unordered_map<string, AnimationManager*> AnimationManager::mAnimInstances;

AnimationManager::AnimationManager(unordered_map<string, Sequence>* ptSequenceSet, SequenceData* _ptSeqData) :
ptSequences(ptSequenceSet),
iCurrentFrame(0),
lastFrameTime(0),
ptSeqData(_ptSeqData)
{
	sCurrentSequence = ptSeqData->startingSequence;
	if(ptSequenceSet == nullptr) {
		// I'm pretty sure this is impossible, but doesn't hurt to check
		R_Error("AnimationManager::AnimationManager: null ptSequenceSet");
	}
}

void AnimationManager::PushFrame() {
	int time = SDL_GetTicks();
	auto seq = ptSequences->find(sCurrentSequence);
	if(seq == ptSequences->end()) {
		R_Error("AnimationManager::PushFrame: bad sequence %s", sCurrentSequence.c_str());
		return;
	}
	if(time-lastFrameTime < (1000 / seq->second.fps)) {
		return;
	}
	// Advance the frame
	if(++iCurrentFrame >= seq->second.frameCount && seq->second.bLoop) {
		iCurrentFrame = 0;
	}
}

void AnimationManager::DrawActiveFrame(SDL_Texture* in, SDL_Rect* pos) {
	auto seq = ptSequences->find(sCurrentSequence);
	if(seq == ptSequences->end()) {
		R_Error("AnimationManager::DrawActiveFrame: bad sequence %s", sCurrentSequence.c_str());
		return;
	}

	RenderCode::DrawImageAbsClipped((Image*)in, pos->x, pos->y, 
		iCurrentFrame * ptSeqData->framesize, seq->second.rowNum * ptSeqData->rowheight,
		ptSeqData->framesize, ptSeqData->rowheight);
}

void AnimationManager::DrawAnimated(Material* ptMaterial, int x, int y, bool bTransMap) {
	SDL_Texture* ptTexture;
	PushFrame();
	if(!ptMaterial->bLoadedResources) {
		ptMaterial->LoadResources();
	}

	if(bTransMap) {
		ptTexture = ptMaterial->ptTransResource;
	} else {
		ptTexture = ptMaterial->ptResource;
	}
	if(!ptTexture) {
		return;
	}

	SDL_Rect pos; pos.x = x; pos.y = y;
	DrawActiveFrame(ptTexture, &pos);
}

bool AnimationManager::Finished() {
	auto it = (*ptSequences)[sCurrentSequence];
	if(it.bLoop) {
		return false;
	} else if(iCurrentFrame >= it.frameCount) {
		return true;
	} else {
		return false;
	}
}

void AnimationManager::SetSequence(const char* sSequence) {
	auto it = ptSequences->find(sSequence);
	if(it == ptSequences->end()) {
		R_Message(PRIORITY_WARNING, "WARNING: bad sequence change (got: %s)\n", sSequence);
		return;
	}
	sCurrentSequence = sSequence;
	iCurrentFrame = 0;
}

const char* AnimationManager::GetCurrentSequence() {
	return sCurrentSequence.c_str();
}

AnimationManager* AnimationManager::GetAnimInstance(const char* sRef, const char* sMaterial) {
	auto it = mAnimInstances.find(sRef);
	if(it != mAnimInstances.end()) {
		return it->second;
	}
	Material* ptMat = mats->GetMaterial(sMaterial);
	if(ptMat == nullptr) {
		R_Message(PRIORITY_WARNING, "AnimationManager::GetAnimInstance: couldn't find material '%s'\n", sMaterial);
		return nullptr;
	}
	AnimationManager* ptManager = new AnimationManager(&ptMat->mSequences, &ptMat->sd);
	mAnimInstances[sRef] = ptManager;
	return ptManager;
}

void AnimationManager::KillAnimInstance(const char* sRef) {
	auto it = mAnimInstances.find(sRef);
	if(it == mAnimInstances.end()) {
		R_Message(PRIORITY_WARNING, "WARNING: cannot kill anim instance %s due to missing ref\n", sRef);
		return;
	}
	delete it->second;
}

void AnimationManager::ShutdownAnims() {
	for(auto it = mAnimInstances.begin(); it != mAnimInstances.end(); ++it) {
		delete it->second;
	}
}

void AnimationManager::Animate() {
	// FIXME: remove
}