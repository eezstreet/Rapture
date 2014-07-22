#include "sys_local.h"

namespace Cmd {
	static unordered_map<string, conCmd_t> cmdlist;

	static string GetFirstCommand(bool& bFoundCommand) {
		auto it = cmdlist.begin();
		if(it == cmdlist.end())
			bFoundCommand = false;
		else
			bFoundCommand = true;
		return it->first;
	}

	static string GetNextCommand(string lastCommand, bool& bFoundCommand) {
		auto it = cmdlist.find(lastCommand);
		if(it == cmdlist.end()) {
			bFoundCommand = false;
			return "";
		}
		else {
			++it;
			if(it == cmdlist.end()) {
				bFoundCommand = false;
				return "";
			}
			else {
				bFoundCommand = true;
			}
		}
		return it->first;
	}

	void ListCommands() {
		bool bFoundCommand = true;
		for(string s = GetFirstCommand(bFoundCommand); bFoundCommand; s = GetNextCommand(s, bFoundCommand)) {
			R_Message(PRIORITY_MESSAGE, "%s\n", s.c_str());
		}
	}

	string sLastTriedToComplete = "";
	void ProcessCommand(const char *cmd) {
		// in the future, mods will add more to this list
		vector<string> arguments = Tokenize(cmd);
		sLastTriedToComplete = "";
		auto it = cmdlist.find(arguments[0]);
		if(it != cmdlist.end()) {
			auto command = (*it).second;
			command(arguments);
			return;
		}
		else if(CvarSystem::ProcessCvarCommand(arguments[0], arguments)) {
			return;
		}
		R_Message(PRIORITY_MESSAGE, "unknown cmd '%s'\n", arguments[0].c_str());
	}

	void AddCommand(const string& cmdName, conCmd_t cmd) {
		cmdlist[cmdName] = cmd;
	}

	void RemoveCommand(string cmdName) {
		cmdlist.erase(cmdName);
	}

	void ClearCommandList() { 
		cmdlist.clear(); 
	}

	string TabComplete(const string& input) {
		bool bSimple = false;
		string useToCompare = input;
		if(sLastTriedToComplete.length() <= 0) {
			// Since we haven't tried to complete anything recently, we need to use simple mode
			bSimple = true;
		}

		if(input.compare(0, sLastTriedToComplete.length(), sLastTriedToComplete) != 0) {
			// We last tried to complete with a simpler input, so follow the same train
			bSimple = true;
		}

		if(!bSimple) {
			useToCompare = sLastTriedToComplete;
		}

		vector<string> vFoundElements;
		for(auto it = cmdlist.begin(); it != cmdlist.end(); it++) {
			if(it->first.compare(0, useToCompare.length(), useToCompare) == 0) {
				vFoundElements.push_back(it->first);
			}
		}

		bool bFoundCommand = true;
		for(auto it = CvarSystem::GetFirstCvar(bFoundCommand); bFoundCommand; it = CvarSystem::GetNextCvar(it, bFoundCommand)) {
			if(it.compare(0, useToCompare.length(), useToCompare) == 0) {
				vFoundElements.push_back(it);
			}
		}

		if(vFoundElements.size() <= 0) {
			R_Message(PRIORITY_MESSAGE, "No commands found.\n");
			return input;
		}

		sLastTriedToComplete = input;
	}

	// This uses a standard quote-token system.
	// Example:
	// cmd.exe herp derp "herp derp"
	// returns a vector with:
	// cmd.exe, herp, derp, herp derp
	inline vector<string> Tokenize(const string &str) {
		vector<string> retVal;
		size_t lastSplit = 0;
		size_t i = 0;
		for(i = 0; i < str.length(); i++) {
			char c = str[i];
			if(c == ' ') {
				retVal.push_back(str.substr(lastSplit, i-lastSplit));
				lastSplit = i+1;
			} else if(c == '\"') {
				size_t quoteStart = ++i;
				if(quoteStart >= str.length() || str.find_first_of('\"', quoteStart) == string::npos) {
					continue;
				}
				while(i < str.length() && str[i] != '\"') {
					i++;
				}
				retVal.push_back(str.substr(quoteStart, i-quoteStart));
				lastSplit = i;
			}
		}
		if(retVal.size() >= 128) {
			R_Message(PRIORITY_MESSAGE, "Command too large to process.\n");
			vector<string> returnValue;
			returnValue.push_back(str);
			return returnValue;
		}
		if(retVal.size() <= 0) {
			retVal.push_back(str);
			return retVal;
		}
		for(auto it = retVal.begin(); it != retVal.end();) {
			// Make sure we don't have any entries with pure whitespace
			if(retVal.size() <= 0) {
				break;
			}
			else if(it->size() <= 0) {
				it = retVal.erase(it);
			}
			else if((*it).find_first_not_of(' ') == string::npos) {
				it = retVal.erase(it);
			}
			else {
				++it;
			}
		}
		retVal.push_back(str.substr(lastSplit));
		return retVal;
	}
};