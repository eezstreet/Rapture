#include "sys_local.h"
#include <thread>
#include <vector>
#include <concurrentqueue.h>
#include <dirent.h>
#include <SerializedRaptureAsset.h>
#include <fstream>
#include <cereal/archives/binary.hpp>

unordered_map<string, AssetComponent*> m_assetComponents;

namespace Filesystem {
	/* Cvars */
	Cvar* fs_core = nullptr;
	Cvar* fs_homepath = nullptr;
	Cvar* fs_basepath = nullptr;
	Cvar* fs_game = nullptr;

	Cvar* fs_multithreaded = nullptr;
	Cvar* fs_threads = nullptr;
	Cvar* fs_threadsleep = nullptr;

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

			::this_thread::sleep_for(chrono::milliseconds(fs_threadsleep->Integer()));
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
		fs_threadsleep = CvarSystem::RegisterCvar("fs_threadsleep", "How long filesystem threads should sleep for between tasks.", (1 << CVAR_ARCHIVE), 50);

		fs_threads->AddCallback(ResizeThreadPool);

		InitThreadPool(fs_threads->Integer());

		// Initialize searchpaths
		vSearchPaths.push_back(string(fs_basepath->String()) + "/" + string(fs_core->String()));
		vSearchPaths.push_back(string(fs_basepath->String()) + "/" + string(fs_game->String()));
		vSearchPaths.push_back(string(fs_homepath->String()) + "/" + string(fs_core->String()));
		vSearchPaths.push_back(string(fs_homepath->String()) + "/" + string(fs_game->String()));
		// FIXME: probably include working directory as a searchpath, but that's what fs_basepath defaults to

		// Remove any duplicate searchpaths
		vector<string> swap;
		for (auto it = vSearchPaths.begin(); it != vSearchPaths.end(); ++it) {
			auto found = find(swap.begin(), swap.end(), *it);
			if (found == swap.end()) {
				swap.push_back(*it);
			}
		}
		vSearchPaths = swap;

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
		// Free misc resource data
		for (auto it = m_assetComponents.begin(); it != m_assetComponents.end(); it++) {
			AssetComponent* pComp = it->second;
			switch (pComp->meta.componentType) {
				case Asset_Undefined:
					if (pComp->meta.decompressedSize > 0) {
						free(pComp->data.undefinedComponent);
						pComp->data.undefinedComponent = nullptr;
					}
					break;
				case Asset_Data:
					if (pComp->data.dataComponent->data) {
						free(pComp->data.dataComponent->data);
						pComp->data.dataComponent->data = nullptr;
					}
					free(pComp->data.dataComponent);
					break;
				case Asset_Material:
					if (pComp->data.materialComponent->diffusePixels) {
						free(pComp->data.materialComponent->diffusePixels);
						pComp->data.materialComponent->diffusePixels = nullptr;
					}
					if (pComp->data.materialComponent->normalPixels) {
						free(pComp->data.materialComponent->normalPixels);
						pComp->data.materialComponent->normalPixels = nullptr;
					}
					if (pComp->data.materialComponent->depthPixels) {
						free(pComp->data.materialComponent->depthPixels);
						pComp->data.materialComponent->depthPixels = nullptr;
					}
					free(pComp->data.materialComponent);
					break;
				case Asset_Image:
					if (pComp->data.imageComponent->pixels) {
						free(pComp->data.imageComponent->pixels);
						pComp->data.imageComponent->pixels = nullptr;
					}
					free(pComp->data.imageComponent);
					break;
				case Asset_Font:
					if (pComp->data.fontComponent->fontData) {
						free(pComp->data.fontComponent->fontData);
						pComp->data.fontComponent->fontData = nullptr;
					}
					free(pComp->data.fontComponent);
					break;
				case Asset_Level:
					if (pComp->data.levelComponent->tiles) {
						free(pComp->data.levelComponent->tiles);
						pComp->data.levelComponent->tiles = nullptr;
					}
					if (pComp->data.levelComponent->ents) {
						free(pComp->data.levelComponent->ents);
						pComp->data.levelComponent->ents = nullptr;
					}
					free(pComp->data.levelComponent);
					break;
				case Asset_Composition:
					if (pComp->data.compComponent->components) {
						free(pComp->data.compComponent->components);
						pComp->data.compComponent->components = nullptr;
					}
					if (pComp->data.compComponent->keyframes) {
						free(pComp->data.compComponent->keyframes);
						pComp->data.compComponent->keyframes = nullptr;
					}
					free(pComp->data.compComponent);
					break;
			}
		}

		Zone::FreeAll("files");

		ShutdownThreadPool();
	}

	/* Loads up a RaptureAsset and stores it in zone memory */
	/* TODO: move to hunk */
	void LoadRaptureAsset(RaptureAsset** ptAsset, const string& assetName) {
		RaptureAsset* pAsset = *ptAsset;
		auto assetPath = m_assetList[assetName];
		ifstream infile;

		infile.open(assetPath.c_str(), std::ios::binary);
		if (infile.bad() || infile.eof()) {
			R_Message(PRIORITY_ERRFATAL, "Could not load asset %s - try running as administrator\n", assetName.c_str());
			return;
		}

		cereal::BinaryInputArchive in(infile);
		in >> pAsset->head;

		if (pAsset->head.version != RASS_VERSION) {
			R_Message(PRIORITY_WARNING, "Asset file with bad version (found %i, expected %i)\n", pAsset->head.version, RASS_VERSION);
			infile.close();
			return;
		}

		// TODO: compression
		if (pAsset->head.compressionType != Compression_None) {
			R_Message(PRIORITY_WARNING, "Compression not supported (found in asset %s)\n", pAsset->head.assetName);
			infile.close();
			return;
		}

		// TODO: DLC check

		pAsset->components = (AssetComponent*)Zone::Alloc(sizeof(AssetComponent) * pAsset->head.numberComponents, "files");
		for (int i = 0; i < pAsset->head.numberComponents; i++) {
			in >> pAsset->components[i];

			string szFullName = assetName + '/' + pAsset->components[i].meta.componentName;
			transform(szFullName.begin(), szFullName.end(), szFullName.begin(), ::tolower);
			m_assetComponents[szFullName] = &pAsset->components[i];
		}
		infile.close();
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
	string& ResolveFilePath(string& filePath, const string& file, const string& mode) {
		string desiredSearchPath = "";
		bool bTermSlash = false;
		bool bValidPath = false;

		// Have you tried just reading the path itself????????????
		FILE* fp = fopen(file.c_str(), mode.c_str());
		if (fp != nullptr) {
			fclose(fp);
			filePath = file;
			return filePath;
		}

		if (mode.find('w') != mode.npos) {
			// We're writing, ALWAYS use homepath.
			desiredSearchPath = fs_homepath->String();
			if (desiredSearchPath[desiredSearchPath.length() - 1] == '/') {
				bTermSlash = true;
			}
			bValidPath = true;
		}
		else for (auto it = vSearchPaths.begin(); it != vSearchPaths.end(); ++it) {
			string s;
			desiredSearchPath = (*it);
			if (desiredSearchPath[desiredSearchPath.length() - 1] == '/') {
				bTermSlash = true;
			}
			else {
				bTermSlash = false;
			}
			s = (bTermSlash) ? desiredSearchPath + file : desiredSearchPath + '/' + file;
			FILE* fp = fopen(s.c_str(), mode.c_str());
			if (fp != nullptr) {
				bValidPath = true;
				fclose(fp);
				break;
			}
		}
		
		if (!bValidPath) {
			R_Message(PRIORITY_WARNING, "Couldn't resolve path for file: %s\n", file.c_str());
			return filePath;
		}
		filePath = (bTermSlash) ? desiredSearchPath + file : desiredSearchPath + '/' + file;
		return filePath;
	}

	char* ResolveFilePath(char* buffer, size_t bufferLen, const char* file, const char* mode) {
		string desiredSearchPath;
		bool bTermSlash = false;
		bool bValidPath = false;

		if (strchr(mode, 'w')) {
			// We're writing, ALWAYS use homepath.
			desiredSearchPath = fs_homepath->String();
			if (desiredSearchPath[desiredSearchPath.length() - 1] == '/') {
				bTermSlash = true;
			}
		}
		else for (auto it = vSearchPaths.begin(); it != vSearchPaths.end(); ++it) {
			string s;
			desiredSearchPath = (*it);
			if (desiredSearchPath[desiredSearchPath.length() - 1] == '/') {
				bTermSlash = true;
			}
			else {
				bTermSlash = false;
			}
			s = (bTermSlash) ? desiredSearchPath + file : desiredSearchPath + '/' + file;
			FILE* fp = fopen(s.c_str(), mode);
			if (fp != nullptr) {
				bValidPath = true;
				fclose(fp);
				break;
			}
		}

		if (!bValidPath) {
			R_Message(PRIORITY_WARNING, "Couldn't resolve path for file: %s\n", file);
			return buffer;
		}

		desiredSearchPath = (bTermSlash) ? desiredSearchPath + file : desiredSearchPath + '/' + file;
		strncpy(buffer, desiredSearchPath.c_str(), bufferLen);
		return buffer;
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

	/* Misc helper functions */
	void ListAllFilesInPath(vector<string>& vFiles, const char* extension, const char* folder) {
		string ext = extension;
		for (auto& searchpath : vSearchPaths) {
			string directory = searchpath + '/' + folder;
			DIR* dir = opendir(directory.c_str());
			if (dir == nullptr) {
				continue;
			}
			while (auto ent = readdir(dir)) {
				string filePath = searchpath + folder + '/' + ent->d_name;
				if (filePath.length() <= ext.length()) {
					continue;	// not valid
				}
				if (!filePath.compare(filePath.length() - ext.length(), ext.length(), ext)) {
					vFiles.push_back(filePath);
				}
			}
			closedir(dir);
		}
	}
}