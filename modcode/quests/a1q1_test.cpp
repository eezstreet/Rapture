#include "a1q1_test.h"
#include "Server.h"

A1Q1_Test::A1Q1_Test() {
	QSD(A1Q1_STARTED, A1Q1_Test::ChangeToState, nullptr);
}

void A1Q1_Test::Init() {

}

void A1Q1_Test::ChangeToState(const int toState) {
	if(toState == A1Q1_STARTED) {
		ptServer->GetClient()->NewQuest("Test Quest", "a1q1_test", A1Q1_Test::DescriptionPointer);
		R_Message(PRIORITY_MESSAGE, "a1q1 state changed to A1Q1_STARTED\n");
	}
}

void A1Q1_Test::DescriptionPointer(Client* ptClient) {
	ptClient->SetQuestLogDescription("Test Quest Description for state A1Q1_STARTED");
}