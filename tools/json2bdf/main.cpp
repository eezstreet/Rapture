#include <stdlib.h>
#include <stdio.h>
#ifdef WIN32
#include <Windows.h>
#endif
#include "../../json/cJSON.h"
#include <vector>
#include <string>
#include <sstream>

using namespace std;

#ifdef BIG_ENDIAN
// swap endianness
void eswap(unsigned short &x) {
	x = (x>>8) | (x<<8);
}

void eswap(unsigned int &x) {
	x = (x>>24) | ((x<<8) & 0x00FF0000) | ((x>>8) & 0x0000FF00) | (x<<24);
}
#endif

struct bdfTileInfo_t {
	char id[64];
	int x;
	int y;
	unsigned char renderType;
	char layerOffset;
};

struct bdfEntInfo_t {
	char id[64];
	float x;
	float y;
	int spawnflags;
	int reserved;
};

unsigned char parserendertype(const char* str) {
	if(!stricmp(str, "TRT_FLOOR1")) {
		return 0;
	} else if(!stricmp(str, "TRT_FLOOR2")) {
		return 1;
	} else if(!stricmp(str, "TRT_FLOOR3")) {
		return 2;
	} else if(!stricmp(str, "TRT_WALL1")) {
		return 3;
	} else if(!stricmp(str, "TRT_WALL2")) {
		return 4;
	} else if(!stricmp(str, "TRT_WALL3")) {
		return 5;
	} else if(!stricmp(str, "TRT_SHADOW")) {
		return 6;
	} else if(!stricmp(str, "TRT_SPECIAL")) {
		return 7;
	} else {
		return atoi(str);
	}
}

void processfile(const char* file) {
	FILE* f = fopen(file, "rb+");
	if(!f) {
		printf("error processing file %s\n", file);
		return;
	}
	fseek(f, 0L, SEEK_END);
	size_t size = ftell(f);
	fseek(f, 0L, SEEK_SET);
	void* mem = malloc(size);
	if(!fread(mem, 1, size, f)) {
		printf("couldn't read file %s\n", file);
		free(mem);
		return;
	}

	char error[256];
	cJSON* rootNode = cJSON_ParsePooled((const char*)mem, error, sizeof(error));
	if(!rootNode) {
		printf("%s\n", error);
		free(mem);
		return;
	}

	char fileLookup[64] = {0};
	long sizeX = 0L;
	long sizeY = 0L;
	long numTiles = 0L;
	long numEnts = 0L;

	cJSON* header = cJSON_GetObjectItem(rootNode, "header");
	if(!header) {
		printf("no header in file %s\n", file);
		free(mem);
		return;
	} else {
		cJSON* json = cJSON_GetObjectItem(header, "lookup");
		strncpy(fileLookup, cJSON_ToString(json), sizeof(fileLookup));

		json = cJSON_GetObjectItem(header, "sizeX");
		sizeX = cJSON_ToInteger(json);

		json = cJSON_GetObjectItem(header, "sizeY");
		sizeY = cJSON_ToInteger(json);
	}

	cJSON* tiles = cJSON_GetObjectItem(rootNode, "tiles");
	vector<bdfTileInfo_t> vTiles;
	if(!tiles) {
		printf("no tiles in file %s\n", file);
		free(mem);
		return;
	} else {
		numTiles = cJSON_GetArraySize(tiles);
		for(int i = 0; i < numTiles; i++) {
			bdfTileInfo_t tile;
			cJSON* arr = cJSON_GetArrayItem(tiles, i);
			cJSON* json;

			json = cJSON_GetObjectItem(arr, "lookup");
			strncpy(tile.id, cJSON_ToString(json), sizeof(tile.id));
			json = cJSON_GetObjectItem(arr, "x");
			tile.x = cJSON_ToInteger(json);
			json = cJSON_GetObjectItem(arr, "y");
			tile.y = cJSON_ToInteger(json);
			json = cJSON_GetObjectItem(arr, "renderType");
			tile.renderType = parserendertype(cJSON_ToString(json));
			json = cJSON_GetObjectItem(arr, "layerOffset");
			tile.layerOffset = cJSON_ToInteger(json);

			vTiles.push_back(tile);
		}
	}

	cJSON* ents = cJSON_GetObjectItem(rootNode, "entities");
	vector<bdfEntInfo_t> vEnts;
	if(!ents) {
		printf("no ents in file %s\n", file);
		free(mem);
		return;
	} else {
		numEnts = cJSON_GetArraySize(ents);
		for(int i = 0; i < numEnts; i++) {
			bdfEntInfo_t ent;
			cJSON* arr = cJSON_GetArrayItem(tiles, i);
			cJSON* json;

			json = cJSON_GetObjectItem(arr, "lookup");
			strncpy(ent.id, cJSON_ToString(json), sizeof(ent.id));
			json = cJSON_GetObjectItem(arr, "x");
			ent.x = cJSON_ToNumber(json);
			json = cJSON_GetObjectItem(arr, "y");
			ent.y = cJSON_ToNumber(json);
			json = cJSON_GetObjectItem(arr, "spawnflags");
			ent.spawnflags = cJSON_ToInteger(json);

			vEnts.push_back(ent);
		}
	}

	fclose(f);

	string strfile = file;
	string desiredFileName = strfile.substr(0, strfile.find_last_of('.'));
	desiredFileName += ".bdf";
	FILE* out = fopen(desiredFileName.c_str(), "wb+");
	if(!out) {
		stringstream ss;
		ss << "couldn't write output file " << desiredFileName << "\n";
#ifdef WIN32
		MessageBox(NULL, ss.str().c_str(), "error", MB_OK);
#endif
		printf("%s", ss.str().c_str());
		free(mem);
		return;
	}

#ifdef BIG_ENDIAN
	eswap(sizeX);
	eswap(sizeY);
	eswap(numTiles);
	eswap(numEnts);
#endif

	unsigned short version = 1;
	unsigned int iReserved = 0;
	unsigned short stReserved = 0;
	fwrite("DRLG.BDF", 1, 8, out);
	fwrite(&version, sizeof(unsigned short), 1, out);
	fwrite(fileLookup, sizeof(char), sizeof(fileLookup), out);
	fwrite(&iReserved, sizeof(iReserved), 1, out);
	fwrite(&iReserved, sizeof(iReserved), 1, out);
	fwrite(&iReserved, sizeof(iReserved), 1, out);
	fwrite(&stReserved, sizeof(stReserved), 1, out);
	fwrite(&sizeX, sizeof(long), 1, out);
	fwrite(&sizeY, sizeof(long), 1, out);
	fwrite(&numTiles, sizeof(long), 1, out);
	fwrite(&numEnts, sizeof(long), 1, out);
	for(auto it = vTiles.begin(); it != vTiles.end(); ++it) {
		fwrite(it->id, sizeof(char), sizeof(it->id), out);
		fwrite(&it->x, sizeof(long), 1, out);
		fwrite(&it->y, sizeof(long), 1, out);
		fwrite(&it->renderType, sizeof(unsigned char), 1, out);
		fwrite(&it->layerOffset, sizeof(char), 1, out);
	}
	for(auto it = vEnts.begin(); it != vEnts.end(); ++it) {
		long reserved2 = 0;
		fwrite(it->id, sizeof(char), sizeof(it->id), out);
		fwrite(&it->x, sizeof(float), 1, out);
		fwrite(&it->y, sizeof(float), 1, out);
		fwrite(&it->spawnflags, sizeof(long), 1, out);
		fwrite(&reserved2, sizeof(long), 1, out);
	}
	fclose(out);
	free(mem);
}

int main(int argc, char** argv) {
	if(argc <= 1) {
		printf("no files selected\n");
#ifdef WIN32
		MessageBox(NULL, "no files selected", "error", MB_OK);
#endif
		return 0;
	}
	for(int i = 1; i < argc; i++) {
		processfile(argv[i]);
	}
	printf("%i files dealt with successfully\n", argc-1);
#ifdef WIN32
	MessageBox(NULL, "all files compiled successfully", "success", MB_OK);
#endif
}