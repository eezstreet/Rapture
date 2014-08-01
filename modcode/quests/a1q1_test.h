#pragma once
#include "Quest.h"

enum A1Q1_States {
	A1Q1_STARTED = 1, // "You have started the test quest."
};

class A1Q1_Test : public Quest {
public:
	A1Q1_Test();

	virtual void Init();

	static void ChangeToState(const int toState);
};