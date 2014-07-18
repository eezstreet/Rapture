#pragma once
#include "Local.h"

class Seed {
public:
	Seed();

	int GetNumber();
	int GenerateRandom();
	int GenerateRandom(int min, int max);
private:
	unsigned int seedNum;
};

class TwoPartSeed {
public:
	TwoPartSeed();

	int GetNumber();
private:
	unsigned int hiSeed;
	unsigned int loSeed;
};