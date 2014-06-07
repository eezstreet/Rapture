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
			auto command = (*it).second;
			command(arguments);
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

	string TabComplete(const string& input) {
		// TODO: perform exorcism
		string returnValue = input;
		vector<string> potentialVictims;

		static vector<string> lastVictimSet;
		static unsigned int lastVictim = -1;

		// Loop through commands first, and then cvars.
		for(auto it = cmdlist.begin(); it != cmdlist.end(); ++it) {
			if(!it->first.compare(0, input.length(), input)) {
				// Valid input, so okay
				potentialVictims.push_back(it->first);
			}
		}

		bool bFoundCommand = false;
		for(string s = CvarSystem::GetFirstCvar(bFoundCommand); bFoundCommand; s = CvarSystem::GetNextCvar(s, bFoundCommand)) {
			if(!s.compare(0, input.length(), input)) {
				potentialVictims.push_back(s);
			}
		}

		// Sort out our victims and choose the most worthy of the flock (evil laughter is heard)
		sort(potentialVictims.begin(), potentialVictims.end());

		if(lastVictimSet == potentialVictims && potentialVictims.size() > 1) {
			// Our last victim set is the same as the one now, let's autocomplete
			lastVictim++;
			if(lastVictim >= potentialVictims.size()) {
				lastVictim = 0;
			}
		}
		else {
			lastVictim = -1;
		}

		if(potentialVictims.size() == 1) {
			// Don't print a list, just complete it for us automatically, no more nonsense.
			returnValue = potentialVictims[0];
		}
		else if(potentialVictims.size() == 0) {
			R_Printf("No commands found.\n");
		}
		else if(lastVictim != -1) {
			returnValue = potentialVictims[lastVictim];
		}
		else {
			R_Printf("\n");
			for(auto it = potentialVictims.begin(); it != potentialVictims.end(); ++it) {
				R_Printf("%s\n", it->c_str()); // TODO: special handling of cvars (show their current value)
			}
			R_Printf("\n");
		}

		lastVictimSet = potentialVictims;
		return returnValue;
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
		for(auto it = retVal.begin(); it != retVal.end(); ++it) {
			// Make sure we don't have any entries with pure whitespace
			if((*it).find_first_not_of(' ') == string::npos) {
				retVal.erase(it);
			}
		}
		retVal.push_back(str.substr(lastSplit));
		return retVal;
	}
};