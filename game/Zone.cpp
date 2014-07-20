#include "sys_local.h"

namespace Zone {

	MemoryManager *mem = NULL;

	string tagNames[] = {
			"none",
			"cvar",
			"files",
			"renderer",
			"images",
			"materials",
			"font",
			"custom",
			"max"
	};

	// allocate some zone memory
	void* MemoryManager::Allocate(int iSize, zoneTags_e tag) {
		if(tag == TAG_NONE) {
			Sys_Error("Zone::Alloc passed TAG_NONE\n");
			return NULL;
		}

		return Allocate(iSize, tagNames[tag]);
	}

	// allocate some zone memory, but use the tag name (good for VM/mod)
	void* MemoryManager::Allocate(int iSize, const string& tag) {
		if(tag.length() <= 0) {
			Sys_Error("Zone::Alloc passed zero-size tag string\n");
			return NULL;
		}

		zone[tag].zoneInUse += iSize;
		if(zone[tag].zoneInUse > zone[tag].peakUsage) {
			zone[tag].peakUsage = zone[tag].zoneInUse;
		}
		void* memory = malloc(iSize);
		ZoneChunk z(iSize, false);
		zone[tag].zone[memory] = z;
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
		Sys_Error("Zone::Free(): corrupt zone memory!");
	}

	// free some zone memory (quicker but still SLOW and not recommended)
	void MemoryManager::FastFree(void* memory, const string& tag) {
		try {
			auto memblock = zone[tag].zone.find(memory);
			if(memblock == zone[tag].zone.end()) {
				R_Printf("WARNING: could not dealloc memory block at 0x%X, memory not allocated!\n", (unsigned int)memory);
				return;
			}
			auto mpair = memblock->second;
			zone[tag].zoneInUse -= mpair.memInUse;
			zone[tag].zone.erase(memory);
			if(!mpair.isClassObject)
				free(memory);
			else
				delete memory;
		}
		catch( out_of_range e ) {
			Sys_Error("Zone::FastFree(): corrupt zone memory!");
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
				void* mem = realloc(memory, iNewSize);
				if(!mem) {
					throw false;
				}
				memory = mem;
				it->second.zone[memory] = ZoneChunk(iNewSize, false);
				return memory;
			}
			catch( out_of_range ) {
				continue;
			}
			catch( bool ) {
				break;
			}
		}
		Sys_Error("Zone::Realloc: corrupt zone memory!");
		return NULL;
	}

	MemoryManager::MemoryManager() {
		R_Printf("Initializing zone memory\n");
	}

	MemoryManager::~MemoryManager() {
		for(auto it = zone.begin(); it != zone.end(); ++it) {
			FreeAll(it->first);
		}
	}

	void MemoryManager::PrintMemUsage() {
		R_Printf("\n%-10s %20s %20s %20s %20s %20s %20s\n", "Tag", "Cur Usage (b)", "Cur Usage (KB)", "Cur Usage (MB)", "Peak Usage (b)", "Peak Usage (KB)", "Peak Usage (MB)");
		R_Printf("%-10s %20s %20s %20s %20s %20s %20s\n", "-----", "-------------", "--------------", "--------------", "--------------", "---------------", "---------------");
		for(auto it = zone.begin(); it != zone.end(); ++it) {
			R_Printf("%-10s %20i %20.2f %20.2f %20i %20.2f %20.2f\n", it->first.c_str(), it->second.zoneInUse, 
				(float)((double)it->second.zoneInUse/1024.0f), (float)((double)it->second.zoneInUse/1048576.0f),
				it->second.peakUsage,
				(float)((double)it->second.peakUsage/1024.0f), (float)((double)it->second.peakUsage/1048576.0f));
		}
	}

	// Functions which are accessed from the outside
	void Init() {
		mem = new MemoryManager();
		for(int it = TAG_NONE; it != MAX_ENGINE_TAGS; ++it)
			mem->CreateZoneTag(tagNames[it]);
	}

	void Shutdown() {
		R_Printf("Shutting down zone memory...");
		delete mem; // yea fuck up dem peasant's RAM
	}

	void* Alloc(int iSize, zoneTags_e tag) {
		return mem->Allocate(iSize, tag);
	}

	void* Alloc(int iSize, const string& tag) {
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

	void* VMAlloc(int iSize, const char* tag) {
		return mem->Allocate(iSize, tag);
	}

	void VMFastFree(void* memory, const char* tag) {
		FastFree(memory, tag);
	}

	void VMFreeAll(const char* tag) {
		FreeAll(tag);
	}

	void NewTag(const char* tag) {
		mem->CreateZoneTag(tag);
	}
}