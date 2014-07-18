#pragma once

enum DRLGAlgorithm_e {
	ALGO_DRUNKEN_STAGGER,
};

enum DRLGSpecialRoom_e {
	SR_ENTRANCE,
	SR_EXIT,
};

struct DRLGData {
	char dungeonHeader[64];
	int roomSizeX, roomSizeY;
	DRLGAlgorithm_e algo;

};