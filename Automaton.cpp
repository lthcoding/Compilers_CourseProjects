#include "Automaton.h"

void Automaton::init(string definition_path){
	FileManager fm;
	vector<string> input = fm.read_by_string(definition_path);
	if (input.size() < 4) {
		throw "Invalid automaton definition";
	}
	unsigned int state_id, num_transition, target;
	string trigger;
	bool flag = true;
	bool is_accept;
	int idx = 1;
	start_state_idx = atoi(input[0].c_str());
	current_state_idx = start_state_idx;
	AutomatonState *new_state = nullptr;
	while (idx < input.size()) {
		if (flag) {
			if (input.size() - idx - 1 < atoi(input[idx + 2].c_str())) {
				throw "Invalid automaton definition";
			}
			state_id = atoi(input[idx].c_str());
			is_accept = (bool)atoi(input[idx + 1].c_str());
			new_state = new AutomatonState(state_id, is_accept);
			num_transition = atoi(input[idx + 2].c_str());
			idx += 3;
			if (num_transition > 0) { flag = false; }
			states[state_id] = new_state;
		}
		else {
			trigger = input[idx];
			target = atoi(input[idx + 1].c_str());
			idx += 2;
			new_state->transition[trigger] = target;
			num_transition--;
		}
		if (num_transition == 0) { flag = true; }
	}
}


bool Automaton::transit(char input) {
	// no such transition
	string catagory = "";
	catagory.push_back(input);
	if ((input >= 'a' && input <= 'z') || (input >= 'A' && input <= 'Z')) {
		catagory = "letter";
	}
	if (input >= '1' && input <= '9') {
		catagory = "1to9";
	}
	
	if (states[current_state_idx]->transition.find(catagory) != states[current_state_idx]->transition.end()) {
		current_state_idx = states[current_state_idx]->transition[catagory];
		return true;
	}
	
	return false;
}


bool Automaton::if_accept() {
	return states[current_state_idx]->is_accept;
}


void Automaton::reset() {
	current_state_idx = start_state_idx;
}