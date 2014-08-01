#pragma once
#include "QuestCallbacks.h"

struct QuestState {
	const int iStateNum;
	Q_StateChangeCallback ptfChangeCallback;
	Q_StateAwayCallback ptfAwayCallback;
};