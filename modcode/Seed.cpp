#include "Seed.h"

Seed::Seed() {
	srand(time(NULL));
	seedNum = (unsigned int)rand();
}

int Seed::GetNumber() {
	return seedNum;
}

int Seed::GenerateRandom() {
	srand(seedNum);
	seedNum = (unsigned int)rand();
	return seedNum++;
}

int Seed::GenerateRandom(int min, int max) {
	int lastSeed = seedNum;
	int range = max - min;
	int random;
	assert(range >= 0);
	if(range == 0) {
		return max;
	}

	srand(seedNum);
	random = min + ((range * rand()) / (RAND_MAX + 1.0));
	if(random > 1) {
		seedNum = (unsigned int)(random) * lastSeed;
	}
	else if(random == 1 || random == 0) {
		seedNum = (unsigned int)(random+2) * lastSeed;
	}
	else {
		seedNum = 1;
	}
	return random;
}

TwoPartSeed::TwoPartSeed() {
	srand(time(NULL));
	loSeed = rand();

	srand(time(NULL));
	hiSeed = rand();

	if(loSeed > hiSeed) {
		// swap the two numbers
#ifdef WIN32
		__asm {
			mov eax, loSeed
			mov ebx, hiSeed
			xor eax, ebx
			xor ebx, eax
			xor eax, ebx
		}
#else
		// slower
		loSeed = loSeed ^ hiSeed;
		hiSeed = loSeed ^ hiSeed;
		loSeed = loSeed ^ hiSeed;
#endif
	}
}