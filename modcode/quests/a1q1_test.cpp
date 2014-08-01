#include "a1q1_test.h"
#include "Server.h"

A1Q1_Test::A1Q1_Test() {
	QSD(A1Q1_STARTED, A1Q1_Test::ChangeToState, nullptr);
}

void A1Q1_Test::Init() {

}

void A1Q1_Test::ChangeToState(const int toState) {
	if(toState == A1Q1_STARTED) {
		R_Message(PRIORITY_MESSAGE, "a1q1 state changed to A1Q1_STARTED\n");
	}
}