#include "sys_local.h"

namespace Zone {

	MemoryManager *mem = NULL;

	string tagNames[] = {
			"none",
			"cvar",
			"files",
			"renderer",
			"custom",
			"max"
	};

	// allocate some zone memory
	void* MemoryManager::Allocate(int iSize, zoneTags_e tag) {
		if(tag == TAG_NONE) {
			R_Printf("Zone::Alloc: passed TAG_NONE\n");
			return NULL;
		}

		zone[tagNames[tag]].zoneInUse += iSize;
		if(zone[tagNames[tag]].zoneInUse > zone[tagNames[tag]].peakUsage) {
			zone[tagNames[tag]].peakUsage = zone[tagNames[tag]].zoneInUse;
		}
		void *memory = malloc(iSize);
		ZoneChunk z(iSize, false);
		zone[tagNames[tag]].zone[memory] = z;
		return memory;
	}

	// free some zone memory (SLOW)
	void MemoryManager::Free(void* memory) {
		for(auto it = zone.begin(); it != zone.end(); ++it) {
			for(auto it2 = it->second.zone.begin(); it2 != it->second.zone.end(); ++it2) {
				if(it2->first == memory) {
					it->second.zoneInUse -= it2->second.memInUse;
					it->second.zone.erase(it2);
					free(memory);
				}
			}
		}
		R_Printf("Zone::Free(): corrupt zone memory\n");
	}

	// free some zone memory (quicker but still SLOW and not recommended)
	void MemoryManager::FastFree(void* memory, const string& tag) {
		try {
			auto mpair = zone[tag].zone.find(memory)->second;
			zone[tag].zoneInUse -= mpair.memInUse;
			zone[tag].zone.erase(memory);
			if(!mpair.isClassObject)
				free(memory);
			else
				delete memory;
		}
		catch( out_of_range e ) {
			R_Printf("Zone::FastFree(): corrupt zone memory\n");
			return;
		}
	}

	// free all memory belonging to a tag (FAST)
	void MemoryManager::FreeAll(const string& tag) {
		zone[tag].zoneInUse = 0;
		for(auto it = zone[tag].zone.begin();
			it != zone[tag].zone.end(); ++it) {
				if(!it->second.isClassObject)
					free(it->first);
				else
					delete it->first;
		}
		zone[tag].zone.clear();
	}

	void* MemoryManager::Reallocate(void* memory, size_t iNewSize) {
		for(auto it = zone.begin(); it != zone.end(); ++it) {
			try {
				auto it2 = it->second.zone.find(memory);
				if(it2->second.isClassObject) return memory; // do NOT allow reallocations on classes
				size_t difference = iNewSize - it2->second.memInUse;
				it->second.zoneInUse += difference;
				if(it->second.zoneInUse > it->second.peakUsage) {
					it->second.peakUsage = it->second.zoneInUse;
				}
				memory = realloc(memory, iNewSize);
				it->second.zone[memory] = ZoneChunk(iNewSize, false);
				return memory;
			}
			catch( out_of_range e ) {
				continue;
			}
		}
		R_Printf("Zone::Realloc: corrupt zone memory\n");
		return NULL;
	}

	MemoryManager::MemoryManager() {
		R_Printf("Initializing zone memory\n");
	}

	MemoryManager::~MemoryManager() {
		for(int i = TAG_NONE+1; i < TAG_MAX; i++) {
			FreeAll(tagNames[i]);
		}
	}

	void MemoryManager::PrintMemUsage() {
		R_Printf("\n%-10s %20s %20s %20s %20s %20s %20s\n", "Tag", "Cur Usage (b)", "Cur Usage (KB)", "Cur Usage (MB)", "Peak Usage (b)", "Peak Usage (KB)", "Peak Usage (MB)");
		R_Printf("%-10s %20s %20s %20s %20s %20s %20s\n", "-----", "-------------", "--------------", "--------------", "--------------", "---------------", "---------------");
		for(int i = TAG_NONE+1; i < TAG_MAX; i++) {
			string tag = tagNames[i];
			R_Printf("%-10s %20i %20.2f %20.2f %20i %20.2f %20.2f\n", tag.c_str(), zone[tag].zoneInUse, 
				(float)((double)zone[tag].zoneInUse/1024.0f), (float)((double)zone[tag].zoneInUse/1048576.0f),
				zone[tag].peakUsage,
				(float)((double)zone[tag].peakUsage/1024.0f), (float)((double)zone[tag].peakUsage/1048576.0f));
		}
	}

	// Functions which are accessed from the outside
	void Init() {
		mem = new MemoryManager();
		for(int it = TAG_NONE; it != TAG_MAX; ++it)
			mem->CreateZoneTag(tagNames[it]);
	}

	void Shutdown() {
		R_Printf("Shutting down zone memory...");
		delete mem; // yea fuck up dem peasant's RAM
	}

	void* Alloc(int iSize, zoneTags_e tag) {
		return mem->Allocate(iSize, tag);
	}

	void Free(void* memory) {
		mem->Free(memory);
	}

	void FastFree(void *memory, const string& tag) {
		mem->FastFree(memory, tag);
	}

	void FreeAll(const string& tag) {
		mem->FreeAll(tag);
	}

	void* Realloc(void *memory, size_t iNewSize) {
		return mem->Reallocate(memory, iNewSize);
	}

	void MemoryUsage() {
		mem->PrintMemUsage();
	}
}