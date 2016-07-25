#include "sys_local.h"

/**
 * The engine-local `Cmd` namespace is responsible for all things related to console commands.
 * Commands are not objects in the Rapture engine.
 */
namespace Cmd {
	static unordered_map<string, conCmd_t> cmdlist;

	// 
	static string GetFirstCommand(bool& bFoundCommand) {
		auto it = cmdlist.begin();
		if(it == cmdlist.end())
			bFoundCommand = false;
		else
			bFoundCommand = true;
		return it->first;
	}

	// Gets the next command in the command list.
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

	// Lists all commands that are registered.
	void ListCommands() {
		bool bFoundCommand = true;
		for(string s = GetFirstCommand(bFoundCommand); bFoundCommand; s = GetNextCommand(s, bFoundCommand)) {
			R_Message(PRIORITY_MESSAGE, "%s\n", s.c_str());
		}
	}

	// Processes something that we've tried to enter in the console.
	// Also prints a message when the result is not found.
	void ProcessCommand(const char *cmd) {
		// in the future, mods will add more to this list
		vector<string> arguments = Tokenize(cmd);
		static string sLastTriedToComplete = "";
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

	// Adds something to the tab completion vector.
	// This can be either a cvar or a command.
	static vector<string> vTabCompletion;
	void AddTabCompletion(const string& cmdName) {
		vTabCompletion.push_back(cmdName);
	}

	// Adds a formally-recognized command to the engine.
	void AddCommand(const string& cmdName, conCmd_t cmd) {
		AddTabCompletion(cmdName);
		cmdlist[cmdName] = cmd;
	}

	// Adds a formally-recognized command to the engine.
	// const char* variant is safe to use across DLL boundaries.
	void AddCommand(const char* cmdName, conCmd_t cmd) {
		string scmd = cmdName;
		AddTabCompletion(scmd);
		cmdlist[scmd] = cmd;
	}

	// Removes a command from the engine.
	void RemoveCommand(string cmdName) {
		cmdlist.erase(cmdName);
	}

	// Removes a command from the engine.
	// const char* variant is safe to use across DLL boundaries.
	void RemoveCommand(const char* cmdName) {
		cmdlist.erase(cmdName);
	}

	// Clears all commands that have been registered.
	void ClearCommandList() { 
		cmdlist.clear(); 
	}

	// Tab-completes a string based on information 
	string TabComplete(const string& input) {
		vector<string> vValid;
		for(auto it = vTabCompletion.begin(); it != vTabCompletion.end(); ++it) {
			string& thisOne = *it;
			if(!thisOne.compare(0, input.size(), input)) {
				vValid.push_back(thisOne);
			}
		}
		if(vValid.size() == 0) {
			R_Message(PRIORITY_MESSAGE, "No commands found.\n");
			return input;
		}
		else if(vValid.size() == 1) {
			return vValid.at(0);
		}
		R_Message(PRIORITY_MESSAGE, "\n");
		for(auto it = vValid.begin(); it != vValid.end(); ++it) {
			R_Message(PRIORITY_MESSAGE, "%s\n", (*it).c_str());
		}
		R_Message(PRIORITY_MESSAGE, "\n");
		return input;
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