#pragma once
#include "QuestState.h"

#define QSD(A, B, C) vStates.push_back({A, B, C})

struct Quest {
	string sName;
	vector<QuestState> vStates;

	virtual void Init() = 0;
};