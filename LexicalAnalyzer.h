#pragma once
#include "FileManager.h"
#include "Automaton.h"

#define NUM_AUTOMATONS 3
#define AUTOMATON_DEF_PATH_0 "./source_file/identifier.txt"
#define AUTOMATON_DEF_PATH_1 "./source_file/integer.txt"
#define AUTOMATON_DEF_PATH_2 "./source_file/floating.txt"
#define OUTPUT_TOKEN_PATH "./output/lexical_token.txt"
#define OUTPUT_TABLE_PATH "./output/lexical_table.txt"
#define NUM_KEYWORD 12
#define NUM_OPERATOR 13
#define NUM_SEPARATOR 6
#define NUM_ASSIGNMENT 5


enum AutomatonType {
	identifier_auto,
	integer_auto,
	floating_auto
};

enum SymbolType {
	keyword,
	identifier,
	integer,
	floating,
	op,  // operator
	separator,
	assignment,
	comment,
	error_type,
};

typedef struct Token {
	SymbolType type;
	int id;  // pointer(index) of entry to the corresponding symbol table of the type, -1 when error
	string error_field;  // valid when id == -1
}token;


class LexicalAnalyzer{
private:
	FileManager fm;
	Automaton automaton_array[NUM_AUTOMATONS];
	vector<token> token_string;

	// symble tables
	vector<string> identifier_table;
	vector<string> const_table;
	vector<string>comment_table;
	// find index in symbol tables    id->name
	unordered_map<string, int> identifier_table_idx;
	unordered_map<string, int> const_table_idx;
	unordered_map<string, int> comment_table_idx;
	
	const string keywords[NUM_KEYWORD] = { "main", "if", "else", "while", "int", "long", "short", "float", "double", "bool", "char", "return" };
	const string ops[NUM_OPERATOR] = { "+", "-", "*", "/", "!",  "&&", "||" , ">" , "<" , ">=" , "<=" , "==" , "!=" };
	const char separators[NUM_SEPARATOR] = { ',', ';', '(', ')', '{', '}' };
	const string assignments[NUM_ASSIGNMENT] = { "=","+=","-=","*=","/=" };

public:
	void init();
	bool is_letter(char ch);
	bool is_number(char ch);
	
	// if not, return -1 ; or return index in the corresponging table (i.e. const string ops[NUM_OPERATOR])
	int is_separator(string word);
	int is_assignment(string word);
	int is_op(string word);
	int is_keyword(string word);

	void output_symbol_tables();

	bool automaton_scan(AutomatonType auto_type, string word);
	token scan_single_word(string word);  // the word should not be empty.
	void output_table(string out_table_path);
	string print_token(token item);
	bool analyze(string source_code_path);
	vector<token> get_token_string();
};

