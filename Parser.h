#pragma once
#include "FileManager.h"
#include "LexicalAnalyzer.h"
#define PARSER_OUTPUT_PATH "./output/parsing_result.txt"


enum NodeType{
	prog_type,
	func_type,
	if_type,
	while_type,
	arith_type,
	logic_type,
	type_type,
	assign_type,
	return_type,
	operand_type,
	argu_type,
	parsing_error
};

//type=arith ; toke_idx=0 : ()
typedef struct AST_Node {
	NodeType type;
	int token_idx;
	string error_field;
	vector<struct AST_Node *> child_nodes;

	AST_Node(NodeType t) :type(t), token_idx(-1), error_field("") {}
	AST_Node(int i) :token_idx(i), error_field("") {}
	AST_Node(NodeType t, int i) :type(t), token_idx(i), error_field("") {}
}node;


class Parser{
private:
	vector<token> token_string;
	int current_token_idx;
	int not_end;
	token current_token;


public:
	void init(vector<token> input_token_string);
	void get_token();
	void rollback(int idx);
	vector<string> print_tree(node *root, int table_space);
	void print_parsing_result(pair<bool, node *> result);

	// Parser for each non-terminal symbol
	pair<bool, node *> program_paser();
	/* return string:
	*  rollback:  errors which may be solved by rollback
	*  error: errors which cannot be solved by rollback, such as an error occurs after if_paser scanned the if keyword
	*  fatal error: cannot figure out the boundary of an error 
	*  success
	*/
	pair<string, node *> statement_parser();
	pair<string, node *> function_parser();
	pair<string, node *> if_parser();
	pair<string, node *> while_parser();
	pair<string, node *> assign_parser();
	pair<string, node *> assignop_parser();
	pair<string, node *> type_parser();
	pair<string, node *> argument_parser();
	pair<string, node *> single_argument_parser();
	pair<string, node *> return_parser();
	pair<string, node *> expression_parser();
	pair<string, node *> arithmetic_parser();
	pair<string, node *> mult_div_parser();
	pair<string, node *> arith_sub_expr_parser();
	pair<string, node *> logical_parser();
	pair<string, node *> comparison_parser();
	pair<string, node *> logical_sub_parser();
	pair<string, node *> operand_parser();
};

