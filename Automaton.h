#pragma once
#include <vector>
#include <unordered_map>
#include "FileManager.h"
using namespace std;

typedef struct AutomatonState {
	unsigned int id;
	unordered_map<string, unsigned int> transition;
	bool is_accept;

	AutomatonState(unsigned int i, bool accept) {
		id = i;
		is_accept = accept;
	}
}AutomatonState;


typedef class DeterministicFiniteAutomaton{
private:
	unordered_map<int, AutomatonState*> states;
	unsigned int start_state_idx;
	unsigned int current_state_idx;

public:
	void init(string definition_path);
	bool transit(char input);
	bool if_accept();
	void reset();

}Automaton;


