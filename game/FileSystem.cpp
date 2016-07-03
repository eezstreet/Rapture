#include "sys_local.h"
#include <thread>
#include <vector>
#include <concurrentqueue.h>

unordered_map<string, AssetComponent*> m_assetComponents;

namespace Filesystem {
	/* Cvars */
	Cvar* fs_core = nullptr;
	Cvar* fs_homepath = nullptr;
	Cvar* fs_basepath = nullptr;
	Cvar* fs_game = nullptr;

	Cvar* fs_multithreaded = nullptr;
	Cvar* fs_threads = nullptr;

	/* Parallelism */
	using namespace moodycamel;
	ConcurrentQueue<AsyncFileTask> qFileTasks;
	ConcurrentQueue<AsyncResourceTask> qResourceTasks;
	MutexVariable<vector<File*>> vOpenFiles;
	vector<thread*> vWorkerThreads;
	bool thread_die = false;
	
	/* Resolution */
	unordered_map<string, string> m_assetList;

	vector<string> vSearchPaths;
	
	/* What each worker thread is running */
	void worker_thread() {
		while (!thread_die) {
			// Do a file task and then a resource task each step
			AsyncFileTask FTask;
			if (qFileTasks.try_dequeue(FTask)) {
				switch (FTask.type) {
				case AsyncFileTask::Task_Open:
					FTask.pFile->DequeOpen((fileOpenedCallback)FTask.callback);
					break;
				case AsyncFileTask::Task_Close:
					FTask.pFile->DequeClose((fileClosedCallback)FTask.callback);
					break;
				case AsyncFileTask::Task_Read:
					FTask.pFile->DequeRead(FTask.data, FTask.dataSize, (fileReadCallback)FTask.callback);
					break;
				case AsyncFileTask::Task_Write:
					FTask.pFile->DequeRead(FTask.data, FTask.dataSize, (fileWrittenCallback)FTask.callback);
					break;
				}
			}
			
			AsyncResourceTask RTask;
			if (qResourceTasks.try_dequeue(RTask)) {
				switch (RTask.type) {
					case AsyncResourceTask::Task_Request:
						RTask.pResource->DequeRetrieve((assetRequestCallback)RTask.callback);
						break;
				}
			}
		}
	}

	/* Create the thread pool */
	void InitThreadPool(int numThreads) {
		// Make sure we have a valid multithreading value
		if (!fs_multithreaded->Bool()) {
			return;
		}
		if (numThreads <= 0) {
			fs_multithreaded->SetValue(false);
			return;
		}

		// Create threadpool
		for (int i = 0; i < numThreads; i++) {
			thread* workerThread = new thread(worker_thread);
			vWorkerThreads.push_back(workerThread);
		}
	}

	/* Shutdown the thread pool */
	void ShutdownThreadPool() {
		if (!fs_multithreaded->Bool()) {
			return;
		}

		// Kill all the threads once their tasks are done
		thread_die = true;
		
		// Additionally, close any open file handles
		vector<File*> vOpenFiles_ = vOpenFiles.GetVar();
		size_t numOpen = vOpenFiles_.size();
		if (numOpen) {
			R_Message(PRIORITY_WARNING, "%i unclosed file handles", numOpen);
		}

		for (auto it = vOpenFiles_.begin(); it != vOpenFiles_.end(); ++it) {
			QueueFileClose(*it, nullptr);
		}
		vOpenFiles.Descope();

		// Wait for all of the threads to die
		for (auto it = vWorkerThreads.begin(); it != vWorkerThreads.end(); ++it) {
			(*it)->join();
		}
		vWorkerThreads.clear();
	}

	/* Resize the thread pool (done via cvar callback) */
	void ResizeThreadPool(int newValue) {
		ShutdownThreadPool();
		InitThreadPool(newValue);
	}

	/* Create a list of asset files and list their absolute path */
	void CreateAssetList() {
		DIR* dir;
		dirent* ent;
		for (auto it = vSearchPaths.begin(); it != vSearchPaths.end(); ++it) {
			string path = *it + "/";
			if ((dir = opendir(path.c_str())) != nullptr) {
				while ((ent = readdir(dir)) != nullptr) {
					string fullPath = path + ent->d_name;
					FILE* fp = fopen(fullPath.c_str(), "rb+");
					if (fp == nullptr) {
						continue;
					}
					char buffer[4] = { 0 };
					fread(buffer, 1, 4, fp);
					if (buffer[0] == 'R' && buffer[1] == 'A'
						&& buffer[2] == 'S' && buffer[3] == 'S') {
						AssetHeader head;
						fseek(fp, 0, SEEK_SET);
						fread(&head, sizeof(head), 1, fp);
						m_assetList[head.assetName] = fullPath;
					}
					fclose(fp);
				}
				closedir(dir);
			}
		}
	}

	/* Init the filesystem */
	void Init() {
		Zone::NewTag("files");

		// Initialize cvars and parallelism
		fs_core = CvarSystem::RegisterCvar("fs_core", "Core directory; this gets loaded as a last resort.", (1 << CVAR_ROM), "core");
		fs_homepath = CvarSystem::RegisterCvar("fs_homepath", "User homepath directory to write logs, screenshots, etc.", (1 << CVAR_ROM), Sys_FS_GetBasepath() /*Sys_FS_GetHomepath()*/);
		fs_basepath = CvarSystem::RegisterCvar("fs_basepath", "Directory to gamedata", (1 << CVAR_ROM), Sys_FS_GetBasepath());
		fs_game = CvarSystem::RegisterCvar("fs_game", "Which mod to use (core is still loaded)", (1 << CVAR_ROM), "");
		fs_multithreaded = CvarSystem::RegisterCvar("fs_multithreaded", "Whether to use a multithreaded filesystem", (1 << CVAR_ROM), true);
		fs_threads = CvarSystem::RegisterCvar("fs_threads", "How many threads to use. 2 is best for most systems; 4 is best for RAID or SSD drives.", 0, 2);

		fs_threads->AddCallback(ResizeThreadPool);

		InitThreadPool(fs_threads->Integer());

		// Initialize searchpaths
		vSearchPaths.push_back(string(fs_basepath->String()) + "/" + string(fs_core->String()));
		vSearchPaths.push_back(string(fs_basepath->String()) + "/" + string(fs_game->String()));
		vSearchPaths.push_back(string(fs_homepath->String()) + "/" + string(fs_core->String()));
		vSearchPaths.push_back(string(fs_homepath->String()) + "/" + string(fs_game->String()));
		// FIXME: probably include working directory as a searchpath, but that's what fs_basepath defaults to

		R_Message(PRIORITY_MESSAGE, "Searchpaths:\n");
		for (auto it = vSearchPaths.begin(); it != vSearchPaths.end(); ++it) {
			R_Message(PRIORITY_MESSAGE, "%s\n", it->c_str());
		}

		// Construct list of assets
		CreateAssetList();
		R_Message(PRIORITY_MESSAGE, "Assets:\n");
		for (auto it = m_assetList.begin(); it != m_assetList.end(); ++it) {
			R_Message(PRIORITY_MESSAGE, "%s: %s\n", it->first.c_str(), it->second.c_str());
		}
	}

	/* Shutdown the filesystem */
	void Exit() {
		Zone::FreeAll("files");

		ShutdownThreadPool();
	}

	/* Loads up a RaptureAsset and stores it in zone memory */
	/* TODO: move to hunk */
	void LoadRaptureAsset(RaptureAsset** ptAsset, const string& assetName) {
		RaptureAsset* pAsset = *ptAsset;
		auto assetPath = m_assetList[assetName];
		FILE* fp = fopen(assetPath.c_str(), "rb+");
		if (fp == nullptr) {
			R_Message(PRIORITY_ERRFATAL, "Could not load asset %s - try running as administrator\n", assetName.c_str());
			return;
		}
		fread(pAsset, sizeof(AssetHeader), 1, fp);

		if (pAsset->head.version != RASS_VERSION) {
			R_Message(PRIORITY_WARNING, "Asset file with bad version (found %i, expected %i)\n", pAsset->head.version, RASS_VERSION);
			fclose(fp);
			return;
		}

		// TODO: compression
		if (pAsset->head.compressionType != Compression_None) {
			R_Message(PRIORITY_WARNING, "Compression not supported (found in asset %s)\n", pAsset->head.assetName);
			fclose(fp);
			return;
		}

		// TODO: DLC check

		pAsset->components = (AssetComponent*)Zone::Alloc(sizeof(AssetComponent) * pAsset->head.numberComponents, "files");
		for (int i = 0; i < pAsset->head.numberComponents; i++) {
			AssetComponent* comp = &pAsset->components[i];
			fread(&comp->meta, sizeof(comp->meta), 1, fp);

			switch (comp->meta.componentType) {
				case Asset_Undefined:
					comp->data.undefinedComponent = Zone::Alloc(comp->meta.decompressedSize, "files");
					fread(comp->data.undefinedComponent, 1, comp->meta.decompressedSize, fp);
					break;
				case Asset_Data:
					if (comp->meta.componentVersion == COMP_DATA_VERSION) {
						comp->data.dataComponent = (ComponentData*)Zone::Alloc(sizeof(ComponentData), "files");
						fread(comp->data.dataComponent, sizeof(ComponentData::DataHeader), 1, fp);
						ComponentData* data = comp->data.dataComponent;
						data->data = (char*)Zone::Alloc(comp->meta.decompressedSize, "files");
						fread(data->data, sizeof(char), comp->meta.decompressedSize, fp);
					}
					else {
						R_Message(PRIORITY_WARNING, "Data component '%s' in %s has invalid version (expected %i, found %i)\n",
							comp->meta.componentName, pAsset->head.assetName, comp->meta.componentVersion, COMP_DATA_VERSION);
					}
					break;
				case Asset_Material:
					if (comp->meta.componentVersion == COMP_MATERIAL_VERSION) {
						comp->data.materialComponent = (ComponentMaterial*)Zone::Alloc(sizeof(ComponentMaterial), "files");
						fread(comp->data.materialComponent, sizeof(ComponentMaterial::MaterialHeader), 1, fp);
						ComponentMaterial* mat = comp->data.materialComponent;
						if (mat->head.mapsPresent & (1 << Maptype_Diffuse)) {
							size_t diffuseSize = sizeof(uint32_t) * mat->head.width * mat->head.height;
							mat->diffusePixels = (uint32_t*)Zone::Alloc(diffuseSize, "materials");
							fread(mat->diffusePixels, sizeof(uint32_t), mat->head.width * mat->head.height, fp);
						}
						if (mat->head.mapsPresent & (1 << Maptype_Depth)) {
							size_t depthSize = sizeof(uint16_t) * mat->head.depthWidth * mat->head.depthHeight;
							mat->depthPixels = (uint16_t*)Zone::Alloc(depthSize, "materials");
							fread(mat->depthPixels, sizeof(uint16_t), mat->head.depthWidth * mat->head.depthHeight, fp);
						}
						if (mat->head.mapsPresent & (1 << Maptype_Normal)) {
							size_t normalSize = sizeof(uint32_t) * mat->head.normalWidth * mat->head.normalHeight;
							mat->normalPixels = (uint32_t*)Zone::Alloc(normalSize, "materials");
							fread(mat->normalPixels, sizeof(uint32_t), mat->head.normalWidth * mat->head.normalHeight, fp);
						}
					}
					else {
						R_Message(PRIORITY_WARNING, "Material component '%s' in %s has invalid version (expected %i, found %i)\n",
							comp->meta.componentName, pAsset->head.assetName, comp->meta.componentVersion, COMP_MATERIAL_VERSION);
					}
					break;
				case Asset_Image:
					if (comp->meta.componentVersion == COMP_IMAGE_VERSION) {
						comp->data.imageComponent = (ComponentImage*)Zone::Alloc(sizeof(ComponentImage), "files");
						fread(comp->data.imageComponent, sizeof(ComponentImage::ImageHeader), 1, fp);
						size_t pixelSize = sizeof(uint32_t) * comp->data.imageComponent->head.width * comp->data.imageComponent->head.height;
						comp->data.imageComponent->pixels = (uint32_t*)Zone::Alloc(pixelSize, "images");
						fread(comp->data.imageComponent->pixels, pixelSize, 1, fp);
					}
					else {
						R_Message(PRIORITY_WARNING, "Image component '%s' in %s has invalid version (expected %i, found %i)\n",
							comp->meta.componentName, pAsset->head.assetName, comp->meta.componentVersion, COMP_IMAGE_VERSION);
					}
					break;
				case Asset_Font:
					if (comp->meta.componentVersion == COMP_FONT_VERSION) {
						comp->data.fontComponent = (ComponentFont*)Zone::Alloc(sizeof(ComponentFont), "files");
						fread(comp->data.fontComponent, sizeof(ComponentFont::FontHeader), 1, fp);
						comp->data.fontComponent->fontData = (uint8_t*)Zone::Alloc(comp->meta.decompressedSize, "font");
						fread(comp->data.fontComponent->fontData, 1, comp->meta.decompressedSize, fp);
					}
					else {
						R_Message(PRIORITY_WARNING, "Font component '%s' in %s has invalid version (expected %i, found %i)\n",
							comp->meta.componentName, pAsset->head.assetName, comp->meta.componentVersion, COMP_FONT_VERSION);
					}
					break;
				case Asset_Level:
					if (comp->meta.componentVersion == COMP_LEVEL_VERSION) {
						comp->data.levelComponent = (ComponentLevel*)Zone::Alloc(sizeof(ComponentLevel), "files");
						fread(comp->data.levelComponent, sizeof(ComponentLevel::LevelHeader), 1, fp);
						comp->data.levelComponent->tiles = (ComponentLevel::TileEntry*)Zone::Alloc(sizeof(ComponentLevel::TileEntry), "files");
						fread(comp->data.levelComponent->tiles, sizeof(ComponentLevel::TileEntry), comp->data.levelComponent->head.numTiles, fp);
						comp->data.levelComponent->ents = (ComponentLevel::EntityEntry*)Zone::Alloc(sizeof(ComponentLevel::EntityEntry), "files");
						fread(comp->data.levelComponent->ents, sizeof(ComponentLevel::EntityEntry), comp->data.levelComponent->head.numEntities, fp);
					}
					else {
						R_Message(PRIORITY_WARNING, "Level component '%s' in %s has invalid version (expected %i, found %i)\n",
							comp->meta.componentName, pAsset->head.assetName, comp->meta.componentVersion, COMP_LEVEL_VERSION);
					}
					break;
				case Asset_Composition:
					if (comp->meta.componentVersion == COMP_ANIM_VERSION) {
						return; // not complete in RAT
					}
					else {
						R_Message(PRIORITY_WARNING, "Comp component '%s' in %s has invalid version (expected %i, found %i)\n",
							comp->meta.componentName, pAsset->head.assetName, comp->meta.componentVersion, COMP_ANIM_VERSION);
					}
					break;
				case Asset_Tile:
					if (comp->meta.componentVersion == COMP_TILE_VERSION) {
						comp->data.tileComponent = (ComponentTile*)Zone::Alloc(sizeof(ComponentTile), "files");
						fread(comp->data.tileComponent, sizeof(ComponentTile), 1, fp);
					}
					else {
						R_Message(PRIORITY_WARNING, "Tile component '%s' in %s has invalid version (expected %i, found %i)\n",
							comp->meta.componentName, pAsset->head.assetName, comp->meta.componentVersion, COMP_TILE_VERSION);
					}
					break;
			}
			string szFullName = assetName + '/' + comp->meta.componentName;
			m_assetComponents[szFullName] = comp;
		}
		fclose(fp);
	}

	/* Find a component residing within an asset file */
	AssetComponent* FindComponentByName(RaptureAsset* pAsset, const char* compName) {
		int i;
		if (pAsset == nullptr) {
			return nullptr;
		}
		
		for (i = 0; i < pAsset->head.numberComponents; i++) {
			AssetComponent* assetComp = &pAsset->components[i];
			if (!stricmp(assetComp->meta.componentName, compName)) {
				return assetComp;
			}
		}
		return nullptr;
	}

	/* These two resolution functions are used by other processes */
	string ResolveFilePath(const string& file, const string& mode) {
		if (mode.find('w') != mode.npos) {
			// Written file path, so just append the filename to homepath
			string s = fs_homepath->String();
			s += file;
			return s;
		}
		for (auto it = vSearchPaths.begin(); it != vSearchPaths.end(); ++it) {
			string fullPath = (*it) + '/' + file;
			FILE* fp = fopen(fullPath.c_str(), "rb+");
			if (fp != nullptr) {
				fclose(fp);
				return fullPath;
			}
		}
		return "";
	}

	string ResolveAssetPath(const string& assetName_) {
		string assetName = assetName_;
		transform(assetName.begin(), assetName.end(), assetName.begin(), ::tolower);

		auto it = m_assetList.find(assetName);
		if (it == m_assetList.end()) {
			R_Message(PRIORITY_WARNING, "Couldn't find asset with name '%s'\n", assetName_.c_str());
			return "";
		}
		return it->second;
	}

	const char* ResolveFilePath(const char* file, const char* mode) {
		return ResolveFilePath(string(file), string(mode)).c_str();
	}

	const char* ResolveAssetPath(const char* assetName) {
		return ResolveAssetPath(string(assetName)).c_str();
	}

	/* Queue up different commands */
	void QueueFileOpen(File* pFile, fileOpenedCallback callback) {
		if (pFile == nullptr) {
			return;
		}
		if (fs_multithreaded->Bool()) {
			AsyncFileTask task = { AsyncFileTask::Task_Open, pFile, callback };
			qFileTasks.enqueue(task);
		}
		else {
			pFile->DequeOpen(callback);
		}
	}

	void QueueFileRead(File* pFile, void* data, size_t dataSize, fileReadCallback callback) {
		if (pFile == nullptr) {
			return;
		}
		if (fs_multithreaded->Bool()) {
			AsyncFileTask task = { AsyncFileTask::Task_Read, pFile, callback, data, dataSize };
			qFileTasks.enqueue(task);
		}
		else {
			pFile->DequeRead(data, dataSize, callback);
		}
	}

	void QueueFileWrite(File* pFile, void* data, size_t dataSize, fileWrittenCallback callback) {
		if (pFile == nullptr) {
			return;
		}
		if (fs_multithreaded->Bool()) {
			AsyncFileTask task = { AsyncFileTask::Task_Write, pFile, callback, data, dataSize };
			qFileTasks.enqueue(task);
		}
		else {
			pFile->DequeWrite(data, dataSize, callback);
		}
	}

	void QueueFileClose(File* pFile, fileClosedCallback callback) {
		if (pFile == nullptr) {
			return;
		}
		if (fs_multithreaded->Bool()) {
			AsyncFileTask task = { AsyncFileTask::Task_Close, pFile, callback };
			qFileTasks.enqueue(task);
		}
		else {
			pFile->DequeClose(callback);
		}
	}

	void QueueResource(Resource* pRes, assetRequestCallback callback) {
		if (pRes == nullptr) {
			return;
		}
		if (fs_multithreaded->Bool()) {
			AsyncResourceTask task = { AsyncResourceTask::Task_Request, pRes, callback };
			qResourceTasks.enqueue(task);
		}
		else {
			pRes->DequeRetrieve(callback);
		}
	}
}