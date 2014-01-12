#include "sys_local.h"

namespace Zone {

	MemoryManager *mem = NULL;

	string tagNames[] = {
			"none",
			"cvar",
			"files",
			"custom",
			"max"
	};

	// allocate some zone memory
	void* MemoryManager::Allocate(int iSize, zoneTags_e tag) {
		if(tag == TAG_NONE) {
			return NULL; // Zone::Alloc: passed TAG_NONE
		}

		zone[tagNames[tag]].zoneInUse += iSize;
		void *mem = malloc(iSize);
		zone[tagNames[tag]].zone[mem] = make_pair(iSize, false);
		return mem;
	}

	// free some zone memory (SLOW)
	void MemoryManager::Free(void* memory) {
		for(auto it = zone.begin(); it != zone.end(); ++it) {
			try {
				unordered_map<void*, pair<size_t, bool>>::iterator it2 = it->second.zone.find(memory);
				it->second.zoneInUse -= it2->second.first;
				it->second.zone.erase(memory);
				if(!it2->second.second)
					free(memory);
				else
					delete memory;
				return;
			} 
			catch( out_of_range e ) {
				continue;
			}
		}
		// Zone::Free(): corrupt zone memory
	}

	// free some zone memory (quicker but still SLOW and not recommended)
	void MemoryManager::FastFree(void* memory, const string tag) {
		try {
			pair<size_t, bool> mpair = zone[tag].zone.find(memory)->second;
			zone[tag].zoneInUse -= mpair.first;
			zone[tag].zone.erase(memory);
			if(!mpair.second)
				free(memory);
			else
				delete memory;
		}
		catch( out_of_range e ) {
			// Zone::FastFree(): corrupt zone memory
			return;
		}
	}

	// free all memory belonging to a tag (FAST)
	void MemoryManager::FreeAll(string tag) {
		zone[tag].zoneInUse = 0;
		for(auto it = zone[tag].zone.begin();
			it != zone[tag].zone.end(); ++it) {
				zone[tag].zone.erase(it->first);
				if(!it->second.second)
					free(it->first);
				else
					delete it->first;
		}
	}

	void* MemoryManager::Reallocate(void* memory, size_t iNewSize) {
		for(auto it = zone.begin(); it != zone.end(); ++it) {
			try {
				unordered_map<void*, pair<size_t, bool>>::iterator it2 = it->second.zone.find(memory);
				if(it2->second.second) return memory; // do NOT allow reallocations on classes
				__int64 difference = iNewSize - it2->second.first;
				it->second.zoneInUse += difference;
				it->second.zone.erase(memory);
				memory = realloc(memory, iNewSize);
				it->second.zone[memory] = make_pair(iNewSize, false);
				return memory;
			}
			catch( out_of_range e ) {
				continue;
			}
		}
		// Zone::Realloc: corrupt zone memory
		return NULL;
	}

	MemoryManager::MemoryManager() {
	}

	MemoryManager::~MemoryManager() {
		for(int i = TAG_NONE+1; i < TAG_MAX; i++) {
			FreeAll(tagNames[i]);
		}
	}

	// Functions which are accessed from the outside
	void Init() {
		mem = new MemoryManager();
		for(int it = TAG_NONE; it != TAG_MAX; ++it)
			mem->CreateZoneTag(tagNames[it]);
	}

	void Shutdown() {
		delete mem; // yea fuck up dem peasant's RAM
	}

	void* Alloc(int iSize, zoneTags_e tag) {
		return mem->Allocate(iSize, tag);
	}

	void Free(void* memory) {
		mem->Free(memory);
	}

	void FastFree(void *memory, const string tag) {
		mem->FastFree(memory, tag);
	}

	void FreeAll(const string tag) {
		mem->FreeAll(tag);
	}

	void* Realloc(void *memory, size_t iNewSize) {
		return mem->Reallocate(memory, iNewSize);
	}
}