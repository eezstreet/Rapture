#pragma once
#include "sys_shared.h"

class Zone {
private:
	string tagNames[]
	vector<string> tagNames;

	unordered_map<string, ZoneTag> zone;

	struct ZoneChunk {
		size_t memInUse;
		bool isClassObject;

		ZoneChunk(size_t mem, bool bClass) : 
		memInUse(mem), isClassObject(bClass) {}
		ZoneChunk() { memInUse = 0; isClassObject = false; }
	};

	struct ZoneTag {
		size_t zoneInUse;
		map<void*, ZoneChunk> zone;

		ZoneTag() : zoneInUse(0) { }
	};

	void CreateZoneTag(const string& tag) { zone[tag] = ZoneTag(); }
public:
	static enum zoneTags_e {
		TAG_NONE,
		TAG_CVAR,
		TAG_FILES,
		TAG_RENDERER,
		TAG_CUSTOM, // should only be used by mods
		TAG_MAX,
	};

	template<typename T>
	T* New(zoneTags_e tag) {
		T* retVal = new T();
		ZoneChunk zc(sizeof(T), true);
		zone[tagNames[tag]].zoneInUse += sizeof(T);
		zone[tagNames[tag]].zone[retVal] = zc;
		return retVal;
	}

	Zone();
	~Zone();
	Zone(Zone && other);
	Zone& operator= (Zone& other);

	void* Alloc(int iSize, zoneTags_e tag);
	void Free(void* memory);
	void FastFree(void* memory, const string& tag);
	void FreeAll(const string& tag);
	void* Realloc(void *memory, size_t iNewSize);
};