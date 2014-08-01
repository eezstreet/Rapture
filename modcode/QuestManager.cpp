#include "QuestManager.h"

//-- Include quest-specific headers here --
#include "quests/a1q1_test.h"

#define Q(A, B, C) B* C = new B(); m_Quests[A] = C; C->Init();
QuestManager::QuestManager() {
	Q("Test Quest", A1Q1_Test, a1q1);
}

QuestManager::~QuestManager() {
	m_Quests.clear();
}

void QuestManager::EnteredArea(const string& sLevelName) {
	for(auto it = m_pfQuestEnterAreaCallbacks.begin(); it != m_pfQuestEnterAreaCallbacks.end(); ++it) {
		(*(it->second))(sLevelName);
	}
}

void QuestManager::ChangeQuestState(const string& sQuestName, const int iNewState) {
	// Find the quest itself
	auto itQuest = m_Quests.find(sQuestName);
	if(itQuest == m_Quests.end()) {
		return; // Quest not found.
	}

	auto itState = m_QuestState.find(sQuestName);
	int iOldState = 0;
	if(itState != m_QuestState.end()) {
		iOldState = itState->second;
	}

	if(iOldState > 0) {
		for(auto it = itQuest->second->vStates.begin(); it != itQuest->second->vStates.end(); ++it) {
			if(it->iStateNum != iOldState) {
				continue;
			}
			auto ptfFromFunc = it->ptfAwayCallback;
			if(ptfFromFunc) {
				ptfFromFunc(iOldState);
			}
			break;
		}
	}

	if(iNewState > 0) {
		for(auto it = itQuest->second->vStates.begin(); it != itQuest->second->vStates.end(); ++it) {
			if(it->iStateNum != iNewState) {
				continue;
			}
			auto ptfToFunc = it->ptfChangeCallback;
			if(ptfToFunc) {
				ptfToFunc(iNewState);
			}
			break;
		}
	}

	m_QuestState[sQuestName] = iNewState;
}

void QuestManager::ChangeStateChangeCallback(const string& sQuestName, Q_StateChangeCallback ptfCallback) {
	m_pfQuestStateChangeCallbacks[sQuestName] = ptfCallback;
}

void QuestManager::ChangeStateAwayCallback(const string& sQuestName, Q_StateAwayCallback ptfCallback) {
	m_pfQuestStateAwayCallbacks[sQuestName] = ptfCallback;

}