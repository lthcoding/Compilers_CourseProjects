#include"FileManager.h"
#include"LexicalAnalyzer.h"
#include"Parser.h"

int main() {
	LexicalAnalyzer scaner;
	Parser parser;
	bool lexical_pass;
	vector<token> token_string;
	scaner.init();
	lexical_pass = scaner.analyze("./source_file/source_code_0.txt");
	if (lexical_pass) {
		token_string = scaner.get_token_string();
		parser.init(token_string);
		pair<bool, node *> parsing_ret = parser.program_paser();
		parser.print_parsing_result(parsing_ret);
	}
	return 0;
}