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
			R_Printf("%s\n", s.c_str());
		}
	}

	void ProcessCommand(const char *cmd) {
		// in the future, mods will add more to this list
		vector<string> arguments = Tokenize(cmd);
		auto it = cmdlist.find(arguments[0]);
		if(it != cmdlist.end()) {
			auto cmd = (*it).second;
			cmd(arguments);
			return;
		}
		else if(CvarSystem::ProcessCvarCommand(arguments[0], arguments)) {
			return;
		}
		R_Printf("unknown cmd '%s'\n", arguments[0].c_str());
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
				while(i < str.length() && str[i] != '\"')
					i++;
				retVal.push_back(str.substr(quoteStart, i));
				lastSplit = i;
			}
		}
		retVal.push_back(str.substr(lastSplit));
		return retVal;
	}
};