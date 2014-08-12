#pragma once
#include "Local.h"
#include "QuestCallbacks.h"
#include "Quest.h"

enum commonQuestStates_e {
	CQS_QUEST_MPDONEPREVIOUS = -4,		// "You cannot complete this quest. Another player completed it in a previous game."
	CQS_QUEST_ALREADYDONE = -3,			// "You cannot complete this quest. Another player completed it first."
	CQS_QUEST_COMPLETEPREVIOUS = -2,	// "This quest was completed in a previous game"
	CQS_QUEST_COMPLETE = -1,			// "Quest complete"
	CQS_QUEST_UNSTARTED = 0,			// Quest hasn't been started yet
	CQS_QUEST_REWARD = 1,				// "Return to X to get your reward."
};

class QuestManager {
private:
	// These hashmaps indicate relevancy of quest objectives.
	// For instance, if a quest has been started, it will show up in the m_QuestState map.
	// If a quest in m_QuestState has callbacks on entering an area, those callbacks are 
	// to be inside of m_pfQuestEnterAreaCallbacks, and so forth.

	// FIXME: introduce another data structure here? Lotta maps here..
	unordered_map<string, Quest*> m_Quests;
	unordered_map<string, int> m_QuestState;
	unordered_map<string, Q_EnterAreaCallback> m_pfQuestEnterAreaCallbacks;
	unordered_map<string, Q_StateChangeCallback> m_pfQuestStateChangeCallbacks;
	unordered_map<string, Q_StateAwayCallback> m_pfQuestStateAwayCallbacks;

	void ChangeStateChangeCallback(const string& sQuestName, Q_StateChangeCallback ptfCallback);
	void ChangeStateAwayCallback(const string& sQuestName, Q_StateAwayCallback ptfCallback);
public:
	void EnteredArea(const string& sLevel);
	void ChangeQuestState(const string& sQuestName, const int iNewState);

	// Semi-hacky but needed for savegame
	unordered_map<string, int>::iterator GetStateStartIterator() { return m_QuestState.begin(); };
	unordered_map<string, int>::iterator GetStateEndIterator() { return m_QuestState.end(); };

	QuestManager();
	~QuestManager();
friend struct Quest;
};