#include "Parser.h"


void Parser::init(vector<token> input_token_string) {
	token_string = input_token_string;
	current_token_idx = 0;
	current_token = token_string[0];
	not_end = true;
    if  (current_token.type == comment) {
		get_token();
	}
}


void Parser::get_token() {
	do {
		current_token_idx++;
		if (current_token_idx >= token_string.size()) { not_end = false; }
		else { current_token = token_string[current_token_idx]; }
	} while (current_token.type == comment && current_token_idx < token_string.size());
}


void Parser::rollback(int idx) {
	current_token_idx = idx;
	current_token = token_string[current_token_idx];
}


vector<string> Parser::print_tree(node *root,int table_space) {
	vector<string> ret;
	if (root == NULL) { return ret; }
	LexicalAnalyzer la;
	string outstr = "";
	for (int i = 0; i < table_space; i++) { outstr.push_back('\t'); }
	outstr.append("[");
	switch (root->type) {
	case prog_type:
		outstr.append("Program] : {");
		break;
	case func_type:
		outstr.append("Function, ");
		if (root->token_idx != -1) { outstr.append(la.print_token(token_string[root->token_idx])); }
		outstr.append("] : {");
		break;
	case if_type:
		outstr.append("If] : {");
		break;
	case while_type:
		outstr.append("While] : {");
		break;
	case arith_type:
		outstr.append("Arithmetic Expr ");
		if (root->token_idx != -1) { outstr.append(la.print_token(token_string[root->token_idx])); }
		outstr.append("] : {");
		break;
	case logic_type:
		outstr.append("Logical Expr ");
		if (root->token_idx != -1) { outstr.append(la.print_token(token_string[root->token_idx])); }
		outstr.append("] : {");
		break;
	case type_type:
		outstr.append("Type, ");
		if (root->token_idx != -1) { outstr.append(la.print_token(token_string[root->token_idx])); }
		outstr.append("] : {");
		break;
	case assign_type:
		outstr.append("Assignment, ");
		if (root->token_idx != -1) { outstr.append(la.print_token(token_string[root->token_idx])); }
		outstr.append("] : {");
		break;	
	case return_type:
		outstr.append("Return] : {");
		break;
	case operand_type:
		outstr.append("Operand, ");
		if (root->token_idx != -1) { outstr.append(la.print_token(token_string[root->token_idx])); }
		outstr.append("] : {");
		break;
	case argu_type:
		outstr.append("Argument, ");
		if (root->token_idx != -1) { outstr.append(la.print_token(token_string[root->token_idx])); }
		outstr.append("] : {");
		break;
	case parsing_error:
		outstr.append("[Fatal Error] : {");
		break;
	}
	if (root->error_field != "") { outstr.append(root->error_field); }
	if (root->child_nodes.size() == 0) { outstr.append(" }"); ret.push_back(outstr); }
	else {
		ret.push_back(outstr);
		for (auto child : root->child_nodes) {
			vector<string> sub = print_tree(child, table_space + 1);
			ret.insert(ret.end(), sub.begin(), sub.end());
		}
		outstr = "";
		for (int i = 0; i < table_space; i++) { outstr.push_back('\t'); }
		outstr.push_back('}');
		ret.push_back(outstr);
	}

	return ret;
}


void  Parser::print_parsing_result(pair<bool, node *> result) {
	FileManager fm;
	string output_line;
	vector<string> output;
	if (result.first) {
		output.push_back("Parsing succeded!");
	}
	else {
		output.push_back("Parsing failed!");
	}
	vector<string> tree = print_tree(result.second, 0);
	output.insert(output.end(), tree.begin(), tree.end());
	fm.output_by_line(output, PARSER_OUTPUT_PATH);

}


pair<bool, node *> Parser::program_paser() {
	node *root = new node(prog_type);
	bool success = true;
	int rollback_idx;
	bool fatal_error = false;
	pair<string, node *> statement_ret, func_ret;
	while (current_token_idx < token_string.size()) {
		rollback_idx = current_token_idx;
		statement_ret = statement_parser();
		if (statement_ret.first == "success") {
			root->child_nodes.push_back(statement_ret.second);
			get_token(); if (!not_end) { break; }
			continue;
		}
		rollback(rollback_idx);
		func_ret = function_parser();
		if (func_ret.first == "success") {
			root->child_nodes.push_back(func_ret.second);
			get_token(); if (!not_end) { break; }
			continue;
		}
		success = false;
		fatal_error = true;
		break;
	 }
	if (fatal_error) {
		node *fatal_error_node = new node(parsing_error);
		fatal_error_node->error_field = "Fatal Error: cannot be identified as a statement or a function.";
		fatal_error_node->child_nodes.push_back(statement_ret.second);
		fatal_error_node->child_nodes.push_back(func_ret.second);
		root->child_nodes.push_back(fatal_error_node);
	}
	return make_pair(success, root);
}


pair<string, node *> Parser::statement_parser() {
	if (current_token.type == keyword && current_token.id == 1) { return if_parser(); }
	else if (current_token.type == keyword && current_token.id == 3) { return while_parser(); }
	else {
		pair<string, node *> assign_ret = assign_parser();
		get_token(); if (!not_end) { return make_pair("error", assign_ret.second); }
		if (current_token.type != separator || current_token.id != 1) { assign_ret.first = "error"; assign_ret.second->error_field.append("Missing ; "); }
		return assign_ret;
	}
}


pair<string, node *> Parser::function_parser() {
	node *root, *new_node;
	pair<string, node *> name_ret, argument_ret, statement_ret, return_ret;
	int roll_back_idx = current_token_idx;
	bool success = true;

	name_ret = single_argument_parser();  // type of return value and function name are the same with a single argument
	root = name_ret.second; root->type = func_type;
	if (name_ret.first == "success") { get_token(); if (!not_end) { return make_pair("error", root); } }
	else { success = false; }

	argument_ret = argument_parser();
	root->child_nodes.push_back(argument_ret.second);
	if(argument_ret.first == "success") { get_token(); if (!not_end) { return make_pair("error", root); } }
	else { success = false; }

	if (current_token.type == separator && current_token.id == 4) { get_token(); if (!not_end) { return make_pair("error", root); } }
	else { root->error_field.append("Missing {. "); success = false; }
	while ((current_token.type != keyword || current_token.id != 11) && (current_token.type != separator || current_token.id != 5)) {
		statement_ret = statement_parser();
		root->child_nodes.push_back(statement_ret.second);
		if (statement_ret.first == "success") { get_token(); if (!not_end) { return make_pair("error", root); } }
		else { success = false; }
	}
	if (current_token.type == keyword && current_token.id == 11) {
		return_ret = return_parser();
		root->child_nodes.push_back(return_ret.second);
		if (return_ret.first == "success") { get_token(); if (!not_end) { return make_pair("error", root); } }
		else { success = false; }
	}
	else { success = false; root->error_field.append("Missing return statement"); }
	if (current_token.type != separator || current_token.id != 5) { root->error_field.append("Missing }. "); success = false; }
	if (success) { return make_pair("success", root); }
	else { return make_pair("error", root); }
}


pair<string, node *>  Parser::type_parser() {
	node *new_node = new node(type_type, current_token_idx);
	if (current_token.type == keyword) {
		if (current_token.id == 4 || current_token.id == 5 || current_token.id == 6 || current_token.id == 7 || current_token.id == 8 || current_token.id == 9 || current_token.id == 10) {
			return make_pair("success", new_node);
		}
	}
	new_node->error_field = "Not a type keyword";
	return make_pair("error", new_node);
}


pair<string, node *> Parser::argument_parser() {
	node *root = new node(argu_type);
	bool success = true;
	int rollback_idx;
	pair<string, node *> single_argument_ret;
	if (current_token.type == separator && current_token.id == 2) { get_token(); if (!not_end) { return make_pair("error", root); } }
	else { root->error_field.append("Missing (. "); success = false; }
	single_argument_ret = single_argument_parser();
	while (single_argument_ret.first == "success") {
		root->child_nodes.push_back(single_argument_ret.second);
		get_token(); if (!not_end) { break; }
		if (current_token.type == separator && current_token.id == 0) { get_token(); if (!not_end) { return make_pair("error", root); } }
		else { break; }
		single_argument_ret = single_argument_parser();
	}
	if (current_token.type != separator || current_token.id != 3){ root->error_field.append("Missing ). "); success = false; }
	if (success) { return make_pair("success", root); }
	else { return make_pair("error", root); }
}


pair<string, node *> Parser::single_argument_parser() {
	node *root = new node(argu_type);
	pair<string, node *> type_ret;
	bool success = true;
	type_ret = type_parser();
	if (type_ret.first == "error") { root->error_field.append("Missing type declaration. "); success = false; }
	else {
		root->child_nodes.push_back(type_ret.second);
		get_token(); if (!not_end) { return make_pair("error", root); }
	}
	if (current_token.type == identifier || (current_token.type == keyword && current_token.id == 0)) { root->token_idx = current_token_idx; }
	else { root->error_field.append("Missing name identifier. "); success = false; }
	if (success) { return make_pair("success", root); }
	else { return make_pair("error", root); }
}


pair<string, node *> Parser::return_parser() {
	node *root = new node(return_type);
	get_token(); if (!not_end) { return make_pair("error", root);}
	pair<string, node *> expr_ret;
	expr_ret = expression_parser();
	root->child_nodes.push_back(expr_ret.second);
	if (expr_ret.first == "error") { return make_pair("error", root); }
	else { get_token(); if(!not_end){return make_pair("error", root);} }
	if (current_token.type != separator || current_token.id != 1) { root->error_field.append("Missing ; "); return make_pair("error", root); }
	else { return make_pair("success", root); }
}


pair<string, node *> Parser::if_parser() {
	node *root = new node(if_type);
	get_token(); if (!not_end) { return make_pair("error", root); }
	bool success = true;

	if (current_token.type == separator && current_token.id == 2) { get_token(); if (!not_end) { return make_pair("error", root); } }
	else { root->error_field.append("Missing (. "); success = false; }
	pair<string, node *> expr_ret, statement_ret;
	expr_ret = expression_parser();
	root->child_nodes.push_back(expr_ret.second);
	if (expr_ret.first == "error") { success = false; }
	else{ get_token(); if (!not_end) { return make_pair("error", root); } }
	if (current_token.type == separator && current_token.id == 3) { get_token(); if (!not_end) { return make_pair("error", root); } }
	else { root->error_field.append("Missing ). "); success = false; }

	if (current_token.type == separator && current_token.id == 4) { get_token(); if (!not_end) { return make_pair("error", root); } }
	else { root->error_field.append("Missing {. "); success = false; }
	while (current_token.type != separator || current_token.id != 5) {
		statement_ret = statement_parser();
		root->child_nodes.push_back(statement_ret.second);
		if (statement_ret.first == "success") { get_token(); if (!not_end) { return make_pair("error", root); } }
		else { success = false; }
	}
	if (current_token.type == separator && current_token.id == 5) { get_token(); if (!not_end) { return make_pair("error", root); } }
	else { root->error_field.append("Missing }. "); success = false; }
	
	if (current_token.type == keyword && current_token.id == 2) {
		get_token(); if (!not_end) { return make_pair("error", root); } 
	}
	else { root->error_field.append("Missing else. "); success = false; return make_pair("error", root); }
	if (current_token.type == separator && current_token.id == 4) { get_token(); if (!not_end) { return make_pair("error", root); } }
	else { root->error_field.append("Missing {. "); success = false; }
	while (current_token.type != separator || current_token.id != 5) {
		statement_ret = statement_parser();
		root->child_nodes.push_back(statement_ret.second);
		if (statement_ret.first == "success") { get_token(); if (!not_end) { return make_pair("error", root); } }
		else { success = false; }
	}
	if (current_token.type != separator || current_token.id != 5) { root->error_field.append("Missing }. "); success = false; }
	if (success) { return make_pair("success", root); }
	else { return make_pair("error", root); }
}


pair<string, node *> Parser::while_parser() {
	node *root = new node(while_type);
	get_token(); if (!not_end) { return make_pair("error", root); }
	bool success = true;

	if (current_token.type == separator && current_token.id == 2) { get_token(); if (!not_end) { return make_pair("error", root); } }
	else { root->error_field.append("Missing (. "); success = false; }
	pair<string, node *> expr_ret, statement_ret;
	expr_ret = expression_parser();
	root->child_nodes.push_back(expr_ret.second);
	if (expr_ret.first == "error") { success = false; }
	else { get_token(); if (!not_end) { return make_pair("error", root); } }
	if (current_token.type == separator && current_token.id == 3) { get_token(); if (!not_end) { return make_pair("error", root); } }
	else { root->error_field.append("Missing ). "); success = false; }

	if (current_token.type == separator && current_token.id == 4) { get_token(); if (!not_end) { return make_pair("error", root); } }
	else { root->error_field.append("Missing {. "); success = false; }
	while (current_token.type != separator || current_token.id != 5) {
		statement_ret = statement_parser();
		root->child_nodes.push_back(statement_ret.second);
		if (statement_ret.first == "success") { get_token(); if (!not_end) { return make_pair("error", root); } }
		else { success = false; }
	}
	if (current_token.type != separator || current_token.id != 5) { root->error_field.append("Missing }. "); success = false; }
	if (success) { return make_pair("success", root); }
	else { return make_pair("error", root); }
}


pair<string, node *> Parser::assign_parser() {
	node *root = new node(assign_type), *new_node;
	pair<string, node *> name_ret, assignop_ret, expr_ret;
	bool success = true;
	int rollback_idx;
	name_ret = single_argument_parser();  // type and name of an identifier are the same with a single argument
	root = name_ret.second; root->type = assign_type;
	if (name_ret.first == "success") { get_token(); if (!not_end) { return make_pair("error", root); } }
	else {
		success = false;
		root->error_field.append("Wrong left value.");
	}

	assignop_ret = assignop_parser();
	root->child_nodes.push_back(assignop_ret.second);
	rollback_idx = current_token_idx;
	if(assignop_ret.first == "success"){ get_token(); if (!not_end) { return make_pair("error", root); } }
	else {
		success = false; 
		root->error_field.append("Wrong assignment operator.");
	}

	expr_ret = expression_parser();
	root->child_nodes.push_back(expr_ret.second);
	if(expr_ret.first == "error") { 
		success = false; 
		root->error_field.append("Wrong expression.");
		rollback(rollback_idx);
	}
	if (success) { return make_pair("success", root); }
	else { return make_pair("error", root); }
}


pair<string, node *> Parser::assignop_parser() {
	node *new_node = new node(assign_type, current_token_idx);
	if (current_token.type == assignment) {
		if (current_token.id == 0 || current_token.id == 1|| current_token.id == 2 || current_token.id == 3 || current_token.id == 4) {
			return make_pair("success", new_node);
		}
	}
	new_node->error_field = "Not a asssign op";
	return make_pair("error", new_node);
}


pair<string, node *> Parser::expression_parser() {
	node *root;
	bool success = true;
	int rollback_idx, arith_idx, logical_idx;
	bool fatal_error = false;
	pair<string, node *> arithmetic_ret, logical_ret;
	rollback_idx = current_token_idx;
	arithmetic_ret = arithmetic_parser();
	arith_idx = current_token_idx;
	rollback(rollback_idx);
	logical_ret = logical_parser();
	logical_idx = current_token_idx;
	if (arithmetic_ret.first == "success" && logical_ret.first == "success") {
		if(logical_idx>arith_idx){ root = logical_ret.second; rollback(logical_idx); }
		else{ root = arithmetic_ret.second; rollback(arith_idx); }
	}
	else if (arithmetic_ret.first == "success") { root = arithmetic_ret.second; rollback(arith_idx); }
	else if (logical_ret.first == "success") { root = logical_ret.second; rollback(logical_idx); }
	else { success = false; }
	if (success) { return make_pair("success", root); }
	else{ 
		node *error_node = new node(parsing_error);
		error_node->error_field = "Error: cannot be identified as a arithmetic or a logical expression.";
		error_node->child_nodes.push_back(arithmetic_ret.second);
		error_node->child_nodes.push_back(logical_ret.second);
		return make_pair("error", error_node);
	}
}


pair<string, node *> Parser::arithmetic_parser() {
	node *root = new node(arith_type), *new_node = nullptr;
	pair<string, node *> mult_div_ret;
	bool success = true;
	mult_div_ret = mult_div_parser();
	if (mult_div_ret.first == "error") { 
		success = false; 
		root->child_nodes.push_back(mult_div_ret.second);
	}
	else { 
		if (current_token_idx + 1 < token_string.size() && token_string[current_token_idx + 1].type == op && (token_string[current_token_idx + 1].id == 0 || token_string[current_token_idx + 1].id == 1)) {
			get_token();
		}
		else { root->child_nodes.push_back(mult_div_ret.second); }
	}
	while (current_token.type == op && (current_token.id == 0 || current_token.id == 1)) {
		new_node = new node(arith_type, current_token_idx);
		new_node->child_nodes.push_back(mult_div_ret.second);
		get_token(); if (!not_end) { return make_pair("error", root); }
		mult_div_ret = mult_div_parser();
		new_node->child_nodes.push_back(mult_div_ret.second);
		if (mult_div_ret.first == "error") { success = false; }
		else {
			if (current_token_idx + 1 < token_string.size() && token_string[current_token_idx + 1].type == op && (token_string[current_token_idx + 1].id == 0 || token_string[current_token_idx + 1].id == 1)) {
				get_token();
			}
		}
		mult_div_ret.second = new_node;
	}
	if (new_node != nullptr) { root->child_nodes.push_back(new_node); }
	if (success) { return make_pair("success", root); }
	else { return make_pair("error", root); }
}


pair<string, node *> Parser::mult_div_parser(){
	node *root = new node(arith_type), *new_node = nullptr;
	pair<string, node *> arith_sub_expr_ret;
	bool success = true;
	arith_sub_expr_ret = arith_sub_expr_parser();
	if (arith_sub_expr_ret.first == "error") { 
		success = false; 
		root->child_nodes.push_back(arith_sub_expr_ret.second);
	}
	else {
		if (current_token_idx + 1 < token_string.size() && token_string[current_token_idx + 1].type == op && (token_string[current_token_idx + 1].id == 2 || token_string[current_token_idx + 1].id == 3)) {
			get_token();
		}
		else{ root->child_nodes.push_back(arith_sub_expr_ret.second); }
	}
	while (current_token.type == op && (current_token.id == 2 || current_token.id == 3)) {
		new_node = new node(arith_type, current_token_idx);
		new_node->child_nodes.push_back(arith_sub_expr_ret.second); 
		get_token(); if (!not_end) { return make_pair("error", root); }
		arith_sub_expr_ret = arith_sub_expr_parser();
		new_node->child_nodes.push_back(arith_sub_expr_ret.second);
		if (arith_sub_expr_ret.first == "error") { success = false; }
		else {
			if (current_token_idx + 1 < token_string.size() && token_string[current_token_idx + 1].type == op && (token_string[current_token_idx + 1].id == 2 || token_string[current_token_idx + 1].id == 3)) {
				get_token();
			}
		}
		arith_sub_expr_ret.second = new_node;
	}
	if (new_node != nullptr) { root->child_nodes.push_back(new_node); }
	if (success) { return make_pair("success", root); }
	else { return make_pair("error", root); }
}


pair<string, node *> Parser::arith_sub_expr_parser(){
	node *root = new node(arith_type), *new_node;
	pair<string, node *> arithmetic_ret, operand_ret;
	if (current_token.type == separator && current_token.id == 2) {
		get_token(); if (!not_end) { return make_pair("error", root); }
		arithmetic_ret = arithmetic_parser();
		root->child_nodes.push_back(arithmetic_ret.second);
		if (arithmetic_ret.first == "success") {
			get_token(); if (!not_end) { return make_pair("error", root); }
			if (current_token.type == separator && current_token.id == 3) { return make_pair("success", root); }
			else {
				root->error_field = "Missing ). ";
				return make_pair("error", root);
			}
		}
		else {
			return make_pair("error", root);
		}
	}
	else {
		return operand_parser();
	}
}


pair<string, node *> Parser::logical_parser(){
	node *root = new node(logic_type), *new_node = nullptr;
	pair<string, node *> comparison_ret;
	bool success = true;
	comparison_ret = comparison_parser();
	if (comparison_ret.first == "error") { 
		success = false; 
		root->child_nodes.push_back(comparison_ret.second);
	}
	else {
		if (current_token_idx + 1 < token_string.size() && token_string[current_token_idx + 1].type == op && (token_string[current_token_idx + 1].id == 5 || token_string[current_token_idx + 1].id == 6)) {
			get_token();
		}
		else { root->child_nodes.push_back(comparison_ret.second); }
	}
	while (current_token.type == op && (current_token.id == 5 || current_token.id == 6)) {
		new_node = new node(logic_type, current_token_idx);
		new_node->child_nodes.push_back(comparison_ret.second);
		get_token(); if (!not_end) { return make_pair("error", root); }
		comparison_ret = comparison_parser();
		new_node->child_nodes.push_back(comparison_ret.second);
		if (comparison_ret.first == "error") { success = false; }
		else {
			if (current_token_idx + 1 < token_string.size() && token_string[current_token_idx + 1].type == op && (token_string[current_token_idx + 1].id == 5 || token_string[current_token_idx + 1].id == 6)) {
				get_token();
			}
		}
		comparison_ret.second = new_node;
	}
	if (new_node != nullptr) { root->child_nodes.push_back(new_node); }
	if (success) { return make_pair("success", root); }
	else { return make_pair("error", root); }
}


pair<string, node *> Parser::comparison_parser(){
	node *root = new node(logic_type), *new_node = nullptr;
	pair<string, node *> logical_sub_ret;
	bool success = true;
	logical_sub_ret = logical_sub_parser();
	if (logical_sub_ret.first == "error") { 
		success = false; 
		root->child_nodes.push_back(logical_sub_ret.second);
	}
	else {
		if (current_token_idx + 1 < token_string.size() && token_string[current_token_idx + 1].type == op && (token_string[current_token_idx + 1].id == 7 || token_string[current_token_idx + 1].id == 8 || token_string[current_token_idx + 1].id == 9 || token_string[current_token_idx + 1].id == 10 || token_string[current_token_idx + 1].id == 11)) {
			get_token();
		}
		else { root->child_nodes.push_back(logical_sub_ret.second); }
	}
	while (current_token.type == op && (current_token.id == 7 || current_token.id == 8 || current_token.id == 9 || current_token.id == 10 || current_token.id == 11)) {
		new_node = new node(logic_type, current_token_idx);
		new_node->child_nodes.push_back(logical_sub_ret.second);
		get_token(); if (!not_end) { return make_pair("error", root); }
		logical_sub_ret = logical_sub_parser();
		new_node->child_nodes.push_back(logical_sub_ret.second);
		if (logical_sub_ret.first == "error") { success = false; }
		else {
			if (current_token_idx + 1 < token_string.size() && token_string[current_token_idx + 1].type == op && (token_string[current_token_idx + 1].id == 7 || token_string[current_token_idx + 1].id == 8 || token_string[current_token_idx + 1].id == 9 || token_string[current_token_idx + 1].id == 10 || token_string[current_token_idx + 1].id == 11)) {
				get_token();
			}
		}
		logical_sub_ret.second = new_node;
	}
	if (new_node != nullptr) { root->child_nodes.push_back(new_node); }
	if (success) { return make_pair("success", root); }
	else { return make_pair("error", root); }
}


pair<string, node *> Parser::logical_sub_parser(){
	node *root = new node(logic_type);
	pair<string, node *> logical_ret, operand_ret;
	if (current_token.type == op && current_token.id == 4) {
		root->token_idx = current_token_idx;
		get_token(); if (!not_end) { return make_pair("error", root); }
	}
	if (current_token.type == separator && current_token.id == 2) {
		get_token(); if (!not_end) { return make_pair("error", root); }
		logical_ret = logical_parser();
		root->child_nodes.push_back(logical_ret.second);
		if (logical_ret.first == "success") {
			get_token(); if (!not_end) { return make_pair("error", root); }
			if (current_token.type == separator && current_token.id == 3) { return make_pair("success", root); }
			else {
				root->error_field = "Missing ). ";
				return make_pair("error", root);
			}
		}
		else {
			return make_pair("error", root);
		}
	}
	else {
		return operand_parser();
	}
}


pair<string, node *> Parser::operand_parser() {
	node *root = new node(operand_type);
	if (current_token.type == identifier || current_token.type == integer || current_token.type == floating) {
		root->token_idx = current_token_idx;
		return make_pair("success", root);
	}
	else {
		root->error_field = "Not an operand";
		return make_pair("error", root);
	}
}