#include "sys_local.h"

namespace Cmd {
	static unordered_map<string, conCmd_t> cmdlist;

	void ProcessCommand(const char *cmd) {
		// in the future, mods will add more to this list
		vector<string> arguments = Tokenize(cmd);
		try {
			auto it = cmdlist.find(arguments[0]);
			auto cmd = (*it).second;
			cmd(arguments);
		}
		catch(out_of_range e){} // unknown cmd
	}

	void AddCommand(string cmdName, conCmd_t cmd) {
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
				retVal.push_back(str.substr(lastSplit, i));
				lastSplit = i;
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