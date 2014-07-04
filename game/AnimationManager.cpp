#include "tr_local.h"

AnimationManager::AnimationManager(const string& sSequence, unordered_map<string, Sequence>* ptSequenceSet, int longestSequence, SequenceData sData) :
ptSequences(ptSequenceSet),
sCurrentSequence(sSequence),
iLongestSequence(longestSequence),
iCurrentFrame(0),
lastFrameTime(0),
sdSeqData(sData) {
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
	if(++iCurrentFrame >= seq->second.frameCount) {
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
		iCurrentFrame * sdSeqData.framesize, seq->second.rowNum * sdSeqData.rowheight,
		sdSeqData.framesize, sdSeqData.rowheight);
}