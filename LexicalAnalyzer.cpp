#include "LexicalAnalyzer.h"

void LexicalAnalyzer::init() {
	Automaton a0;
	Automaton a1;
	Automaton a2;
	a0.init(AUTOMATON_DEF_PATH_0);
	a1.init(AUTOMATON_DEF_PATH_1);
	a2.init(AUTOMATON_DEF_PATH_2);
	automaton_array[identifier_auto] = a0;
	automaton_array[integer_auto] = a1;
	automaton_array[floating_auto] = a2;
}


bool LexicalAnalyzer::is_letter(char ch) {
	if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) {
		return true;
	}
	else {
		return false;
	}
}


bool LexicalAnalyzer::is_number(char ch) {
	if (ch >= '0' && ch <= '9') {
		return true;
	}
	else {
		return false;
	}
}


int LexicalAnalyzer::is_separator(string word) {
	if (word.size() == 1) {
		for (int i = 0; i < NUM_SEPARATOR; i++) {
			if (separators[i] == word[0]) { return i; }
		}
	}
	return -1;
}


int LexicalAnalyzer::is_assignment(string word) {
	for (int i = 0; i < NUM_ASSIGNMENT; i++) {
		if (assignments[i] == word) { return i; }
	}
	return -1;
}


int LexicalAnalyzer::is_op(string word) {
	for (int i = 0; i < NUM_OPERATOR; i++) {
		if (ops[i] == word) { return i; }
	}
	return -1;
}


int LexicalAnalyzer::is_keyword(string word) {
	for (int i = 0; i < NUM_KEYWORD; i++) {
		if (keywords[i] == word) { return i; }
	}
	return -1;
}


bool LexicalAnalyzer::automaton_scan(AutomatonType auto_type, string word) {
	bool flag = true;
	automaton_array[auto_type].reset();
	for (auto ch : word) {
		flag = automaton_array[auto_type].transit(ch);
		if (flag == false) { return false; }
	}
	return automaton_array[auto_type].if_accept();
}


token LexicalAnalyzer::scan_single_word(string word) {
	token tmp;
	int id;
	bool done = false;
	bool error = false;
	string error_info;

	if (is_letter(word[0])) {
		// consider to be a keyword
		id = is_keyword(word);
		if (id != -1) {
			tmp.type = keyword;
			tmp.id = id;
			done = true;
		}
	}

	if (!done && (is_letter(word[0]) || word[0]=='_')) {
		// not a keyword, see if identifier
		if (automaton_scan(identifier_auto, word)) {
			tmp.type = identifier;
			if (identifier_table_idx.find(word) == identifier_table_idx.end()) {
				// not exist in identifier table, need to push a new entry to the table
				identifier_table.push_back(word);
				tmp.id = identifier_table.size() - 1;
				identifier_table_idx[word] = tmp.id;
			}
			else {
				tmp.id = identifier_table_idx[word];
			}
			done = true;
		}
		else {
			// start with letter or _ , not a keyword or a valid indentifier, error
			error = true;
			error_info = word.append(" : invalid identifier");
			done = true;
		}
	}

	if (!done && is_number(word[0])) {
		// consider to be a integer or float
		auto it = find(word.begin(), word.end(), '.');
		if (it == word.end()) {
			// integer
			if (automaton_scan(integer_auto, word)) {
				tmp.type = integer;
				done = true;
			}
		}
		else {
			// float
			if (automaton_scan(floating_auto, word)) {
				tmp.type = floating;
				done = true;
			}
		}
		if (done) {
			// integer and float are in the same symbol table
			if (const_table_idx.find(word) == const_table_idx.end()) {
				const_table.push_back(word);
				tmp.id = const_table.size() - 1;
				const_table_idx[word] = tmp.id;
			}
			else {
				tmp.id = const_table_idx[word];
			}
		}
		else { 
			error = true; 
			error_info = word.append(" : invalid  constant or identifier");
			done = true;
		}
	}

	if (!done && word[0] == '/' && (word[1] == '/' || word[1] == '*')) {
		if (word[1] == '*' && (!(word[word.size() - 1] == '/' && word[word.size() - 2] == '*'))) {
			error = true;
			error_info = word.append(" : mult-line comment missing ending");
		}
		else {
			tmp.type = comment;
			if (comment_table_idx.find(word) == comment_table_idx.end()) {
				// not exist in identifier table, need to push a new entry to the table
				comment_table.push_back(word);
				tmp.id = comment_table.size() - 1;
				comment_table_idx[word] = tmp.id;
			}
			else {
				tmp.id = comment_table_idx[word];
			}
		}
		done = true;
	}

	if (!done) {
		// the words start with other characters, considered to be a separator or a assignment or a operator
		int return_val = 0;
		return_val = is_separator(word);
		if (return_val != -1) {
			tmp.type = separator;
			tmp.id = return_val;
			done = true;
		}
		else {
			return_val = is_assignment(word);
			if (return_val != -1) {
				tmp.type = assignment;
				tmp.id = return_val;
				done = true;
			}
			else {
				return_val = is_op(word);
				if (return_val != -1) {
					tmp.type = op;
					tmp.id = return_val;
					done = true;
				}
			}
		}
		if (!done) { 
			error = true; 
			error_info = word.append(" : invalid character");
			done = true;
		}
	}

	if (error) {
		tmp.type = error_type;
		tmp.id = -1;
		tmp.error_field = error_info;
	}
	return tmp;
}


void LexicalAnalyzer::output_table(string out_table_path) {
	vector<string> output;
	string output_line;

	output.push_back("Identifier Table");
	for (int i = 0; i < identifier_table.size(); i++) {
		output_line.clear();
		output_line.append(to_string(i));
		output_line.push_back('\t');
		output_line.append(identifier_table[i]);
		output.push_back(output_line);
	}
	output.push_back("\n");

	output.push_back("Constant Table");
	for (int i = 0; i < const_table.size(); i++) {
		output_line.clear();
		output_line.append(to_string(i));
		output_line.push_back('\t');
		output_line.append(const_table[i]);
		output.push_back(output_line);
	}
	output.push_back("\n");

	output.push_back("Comment Table");
	for (int i = 0; i < comment_table.size(); i++) {
		output_line.clear();
		output_line.append(to_string(i));
		output_line.push_back('\t');
		output_line.append(comment_table[i]);
		output.push_back(output_line);
	}
	output.push_back("\n");

	output.push_back("Keyword Table");
	for (int i = 0; i < NUM_KEYWORD; i++) {
		output_line.clear();
		output_line.append(to_string(i));
		output_line.push_back('\t');
		output_line.append(keywords[i]);
		output.push_back(output_line);
	}
	output.push_back("\n");

	output.push_back("Operator Table");
	for (int i = 0; i < NUM_OPERATOR; i++) {
		output_line.clear();
		output_line.append(to_string(i));
		output_line.push_back('\t');
		output_line.append(ops[i]);
		output.push_back(output_line);
	}
	output.push_back("\n");

	output.push_back("Separator Table");
	for (int i = 0; i < NUM_SEPARATOR; i++) {
		output_line.clear();
		output_line.append(to_string(i));
		output_line.push_back('\t');
		output_line.push_back(separators[i]);
		output.push_back(output_line);
	}
	output.push_back("\n");

	output.push_back("Assignment Table");
	for (int i = 0; i < NUM_ASSIGNMENT; i++) {
		output_line.clear();
		output_line.append(to_string(i));
		output_line.push_back('\t');
		output_line.append(assignments[i]);
		output.push_back(output_line);
	}

	fm.output_by_line(output, out_table_path);
}


string LexicalAnalyzer::print_token(token item) {
	string outstr;
	switch (item.type) {
	case keyword:
		outstr = "<KEYWORD, ";
		break;
	case identifier:
		outstr = "<IDENTIFIER, ";
		break;
	case integer:
		outstr = "<CONST(int), ";
		break;
	case floating:
		outstr = "<CONST(float), ";
		break;
	case op:
		outstr = "<OPRATOR, ";
		break;
	case separator:
		outstr = "<SEPARATOR, ";
		break;
	case assignment:
		outstr = "<ASSIGNMENT, ";
		break;
	case comment:
		outstr = "<COMMENT, ";
		break;
	case error_type:
		outstr = "<ERROR, ";
		break;
	}
	if (item.type == error_type) {
		outstr.append(item.error_field);
		outstr.push_back('>');
	}
	else {
		outstr.append(to_string(item.id));
		outstr.push_back('>');
	}
	return outstr;
}


bool LexicalAnalyzer::analyze(string source_code_path) {
	string source_code, output_line;
	int num_error = 0;
	vector<string> output;
	source_code = fm.read(source_code_path);

	for (int i = source_code.size() - 1; i >= 0; i--) {
		if (source_code[i] == '\n' || source_code[i] == '\t' || source_code[i] == ' ') { source_code.pop_back(); }
		else { break; }
	}

	if (source_code.size() <= 2) { throw "empty source code"; }

	if (source_code[source_code.size() - 1] != '#') {
		source_code.push_back('#');
		num_error++;
		output.push_back("error0 : Missing #");
	}

	string word, ch_as_word;
	int source_code_idx = 0;
	bool if_push = false, if_divide = false;
	token new_token;
	char ch, tmp;

	while (source_code_idx < source_code.size()) {
		if_push = false;
		if_divide = false;
		ch = source_code[source_code_idx];
		ch_as_word = "";
		ch_as_word.push_back(ch);
		if (ch == '#') {
			if (word.empty()) { break; }
			else { if_push = true; }
		}
		else if (word.empty() && (ch == ' ' || ch == '\n' || ch == '\t')) {
			source_code_idx++;
			continue;
		}
		else if (word.empty()) {
			word.push_back(ch);
			source_code_idx++;
			continue;
		}
		else if (word.size() >= 2 && word[0] == '/' && word[1] == '/') {
			// single line comment
			if (ch == '\n') {
				if_push = true;
			}
			else {
				word.push_back(ch);
			}
			source_code_idx++;
		}
		else if (word.size() >= 2 && word[0] == '/' && word[1] == '*') {
			// multi-line comment
			word.push_back(ch);
			if (ch == '/' && word[word.size() - 2] == '*') {
				if_push = true;
			}
			source_code_idx++;
		}
		else if (ch == ' ' || ch == '\n' || ch == '\t') {
			if_push = true;
			source_code_idx++;
		}
		else if ( ((is_letter(word[word.size() - 1]) || is_number(word[word.size() - 1])) && !(is_letter(ch) || is_number(ch) || ch == '.' || ch == '_')) ||
			(!(is_letter(word[word.size() - 1]) || is_number(word[word.size() - 1]) || word[word.size() - 1] == '.' || word[word.size() - 1] == '_') &&(is_letter(ch) || is_number(ch))) ) {
			if_divide = true;
			if_push = true;
			word.push_back(ch);
			source_code_idx++;
		}
		else if (is_separator(word) != -1 || is_separator(ch_as_word) != -1) {
			if_divide = true;
			if_push = true;
			word.push_back(ch);
			source_code_idx++;
		}
		else{
			word.push_back(ch);
			source_code_idx++;
		}

		if (if_push) {
			if (if_divide) {
				tmp = word[word.size() - 1];
				word.pop_back();
			}
			new_token = scan_single_word(word);
			if (new_token.type == error_type) { num_error++; }
			token_string.push_back(new_token);
			word.clear();
			if (if_divide) { word.push_back(tmp); }
		}
	 }

	// output
	string outstr;
	if (num_error == 0) { output.push_back("Lexical analysis succeeded!"); }
	else{
		outstr = "Lexical analysis failed!  ";
		outstr.append(to_string(num_error));
		outstr.append(" errors found.");
		output.push_back(outstr);
		outstr.clear();
	}
	for (auto item : token_string) {
		outstr = print_token(item);
		output.push_back(outstr);
	}
	
	fm.output_by_line(output, OUTPUT_TOKEN_PATH);
	output_table(OUTPUT_TABLE_PATH);

	if (num_error == 0) { return true; }
	else { return false; }
}


vector<token> LexicalAnalyzer::get_token_string() {
	return token_string;
}
