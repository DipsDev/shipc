

#include <stdio.h>
#include <stdlib.h>

#include "compiler.h"
#include "token.h"
#include  "memory.h"
#include "objects.h"



// main parser struct
typedef struct {
	Token current;
	Token previous;

	FunctionObj* func;
    HashMap* varMap;
    unsigned int varCount;

	bool hadError;
	bool panicMode;
} Parser;

static Chunk* current_chunk(Parser* parser) {
	return &parser->func->body;
}

static HashNode* get_variable(Parser* parser, char* name, int length) {
    return get_node(parser->varMap, name, length);
}

static unsigned int add_variable(Parser* parser, char* name, int length) {
    HashNode* nd_exist = get_node(parser->varMap, name, length);
    if (nd_exist != NULL) {
        return nd_exist->value;
    }

    Local local;
    local.name = name;
    local.value = VAR_NIL;
    local.length = length;
    parser->func->locals[parser->varCount] = local;

    put_node(parser->varMap, name, length, parser->varCount);
    parser->varCount++;
    return parser->varCount-1;
}


// Precedence utils
typedef enum {
	PREC_NONE,
	PREC_ASSIGNMENT,  // =
	PREC_OR,          // or
	PREC_AND,         // and
	PREC_EQUALITY,    // == !=
	PREC_COMPARISON,  // < > <= >=
	PREC_TERM,        // + -
	PREC_FACTOR,      // * /
	PREC_UNARY,       // ! -
	PREC_CALL,        // . ()
	PREC_PRIMARY
} Precedence;

typedef void (*ParseFn)(Parser* parser, Scanner* scanner);

typedef struct {
	ParseFn prefix; // before identifier. ex: -4
	ParseFn infix; // after identifier ex: 4 +
	Precedence precedence;
} ParseRule;

static ParseRule* get_rule(uint8_t token);
static void parse_statement(Parser* parser, Scanner* scanner);
static void parse_expression(Parser* parser, Scanner* scanner);






////





// error related functions
static void error(Parser* parser, const char* string) {
	fprintf(stderr, "[ERROR] %s '%.*s'\n", string, parser->current.length, parser->current.start);
	parser->hadError = true;
}

static void error_at_current(Parser* parser) {
	error(parser, "encountered an error");
	parser->hadError = true;
}

////


static void init_parser(Parser* parser) {
	parser->hadError = false;
	parser->panicMode = false;

    parser->varCount = 0;
    parser->varMap = (HashMap*) malloc(sizeof (HashMap));
    create_variable_map(parser->varMap);

	parser->func = create_func_obj("main", 4, FN_SCRIPT);
}

static void advance(Scanner* scanner, Parser* parser) {
	// set the parser.previous for the current value
	parser->previous = parser->current;

	// loop until encountered a non error token
	for (;;) {
		parser->current = tokenize(scanner);
		if (parser->current.type != TOKEN_ERROR) return;

		error_at_current(parser);
	}
}

static void expect(Scanner* scanner, Parser* parser, TokenType type, const char *message) {
	if (parser->current.type == type) {
		advance(scanner, parser);
		return;
	}
	error(parser, message);
}



static void parse_precedence(Parser* parser, Scanner* scanner, Precedence precendence) {
	advance(scanner, parser);
	ParseFn rule = get_rule(parser->previous.type)->prefix;
	if (rule == NULL) {
		error(parser, "expected expression at");
		return;
	}
	rule(parser, scanner);

	while (precendence <= get_rule(parser->current.type)->precedence) {
		advance(scanner, parser);
		ParseFn infixRule = get_rule(parser->previous.type)->infix;
		infixRule(parser, scanner);
	}
}

static void parse_number(Parser* parser, Scanner* scanner) {
	double value = strtod(parser->previous.start, NULL);
	uint8_t index = add_constant(current_chunk(parser), VAR_NUMBER(value));
	write_bytes(current_chunk(parser), OP_CONSTANT, index);
}

static void parse_grouping(Parser* parser, Scanner* scanner) {
	parse_precedence(parser, scanner, PREC_OR);
	if (parser->current.type == TOKEN_RIGHT_PAREN) {
		advance(scanner, parser);
		return;
	}
	error(parser, "unclosed '(' block at");
}

static void parse_boolean(Parser* parser, Scanner* scanner) {
	TokenType op = parser->previous.type;
	parse_precedence(parser, scanner, (Precedence)(get_rule(op)->precedence + 1));
	switch (op) {
	case TOKEN_EQUAL_EQUAL: write_chunk(current_chunk(parser), OP_COMPARE); break;
	case TOKEN_BANG_EQUAL: {
		write_chunk(current_chunk(parser), OP_COMPARE);
		write_chunk(current_chunk(parser), OP_NOT);
		break;
	}
	}
}




static void parse_unary(Parser* parser, Scanner* scanner) {
	TokenType operatorType = parser->previous.type;

	// Compile the operand.
	parse_precedence(parser, scanner, PREC_UNARY);

	// Emit the operator instruction.
	switch (operatorType) {
	case TOKEN_MINUS: write_chunk(current_chunk(parser), OP_NEGATE); break;
	case TOKEN_BANG: write_chunk(current_chunk(parser), OP_NOT); break;
	default: return; // Unreachable.
	}
}

static void parse_string(Parser* parser, Scanner* scanner) {
	char* str = parser->previous.start + 1; // remove the first "
	int length = parser->previous.length - 2; // not including the 2 "

	// create the string object
	StringObj* obj = create_string_obj(str, length);
	uint8_t index = add_constant(current_chunk(parser), VAR_OBJ(obj));
	write_bytes(current_chunk(parser), OP_CONSTANT, index);

}

static void parse_binary(Parser* parser, Scanner* scanner) {
	TokenType op = parser->previous.type;
	parse_precedence(parser, scanner, (Precedence)(get_rule(op)->precedence + 1));

	switch (op) {
	case TOKEN_PLUS: write_chunk(current_chunk(parser), OP_ADD); break;
	case TOKEN_MINUS: write_chunk(current_chunk(parser), OP_SUB); break;
	case TOKEN_STAR: write_chunk(current_chunk(parser), OP_MUL); break;
	case TOKEN_SLASH: write_chunk(current_chunk(parser), OP_DIV); break;
	}
}




static void parse_literal(Parser* parser, Scanner* scanner) {
	switch (parser->previous.type) {
	case TOKEN_FALSE: write_chunk(current_chunk(parser), OP_FALSE); break;
	case TOKEN_TRUE: write_chunk(current_chunk(parser), OP_TRUE); break;
	case TOKEN_NIL: write_chunk(current_chunk(parser), OP_NIL); break;
	}
}

static void parse_debug_statement(Parser* parser, Scanner* scanner) {
	advance(scanner, parser); // eat the print keyword
	advance(scanner, parser); // eat the (
	parse_expression(parser, scanner);
	expect(scanner, parser, TOKEN_RIGHT_PAREN, "expected ) after func call at");
	expect(scanner, parser, TOKEN_SEMICOLON, "expected ; after call");
    write_chunk(current_chunk(parser), OP_SHOW_TOP);
}

static void parse_if_statement(Parser* parser, Scanner* scanner) {
	advance(scanner, parser); // eat the if keyword
	// parse the bool expr
	parse_expression(parser, scanner); 

	// add a temp value
	write_chunk(current_chunk(parser), OP_POP_JUMP_IF_FALSE);

	// save the value before the body
	int offset = current_chunk(parser)->count;
	write_bytes(current_chunk(parser), 0xff, 0xff);

	expect(scanner, parser, TOKEN_LEFT_BRACE, "expected { after if expression at"); // expect open block after boolean expression
	while (parser->current.type != TOKEN_RIGHT_BRACE && parser->current.type != TOKEN_EOF) {
		// parse the body of the if statement
		parse_statement(parser, scanner);
	}
	// expect user closing the if body
	expect(scanner, parser, TOKEN_RIGHT_BRACE, "expected } after open block at");

	// calculate the new size of the body, and modify the jmp size
	int after_body = current_chunk(parser)->count;
	int body_size = after_body - offset - 2; // subtract 2 because the if and the value
	if (body_size > UINT16_MAX) {
		error(parser, "max jump length exceeded");
	}

	// set the new size
	current_chunk(parser)->codes[offset] = (body_size >> 8) & 0xff;
	current_chunk(parser)->codes[offset + 1] = body_size & 0xff;
}

static void parse_variable(Parser* parser, Scanner* scanner) {
	// parses a variable statement:
	// var x = 5;
	advance(scanner, parser); // eat the var keyword
	Token variable_ident = parser->current;
	advance(scanner, parser);
	expect(scanner, parser, TOKEN_EQUAL, "expected '=' after variable declaration at");

	parse_precedence(parser, scanner, PREC_OR); // parse the expression value
	expect(scanner, parser, TOKEN_SEMICOLON, "expected ; ats");

    // add the variable
    unsigned int var_index = add_variable(parser, variable_ident.start, variable_ident.length);
    write_bytes(current_chunk(parser), OP_STORE_GLOBAL, var_index);
}


static void parse_identifier(Parser* parser, Scanner* scanner) {
	// add the ident string to the pool so we can either call or load it
    HashNode* var = get_variable(parser, parser->previous.start, parser->previous.length);
    if (var == NULL) {
        StringObj *obj = create_string_obj(parser->previous.start, parser->previous.length);
        uint8_t string_index = add_constant(current_chunk(parser), VAR_OBJ(obj));
        write_bytes(current_chunk(parser), OP_LOAD_GLOBAL, string_index);
    } else {
        write_bytes(current_chunk(parser), OP_LOAD_LOCAL, var->value);
    }

    if (parser->current.type != TOKEN_LEFT_PAREN) {
        return;
    }
    // allow to concat function calls, for ex: pow(2)(3)
    while (parser->current.type == TOKEN_LEFT_PAREN) {
        advance(scanner, parser); // eat the (
        // TODO: Add option to add arguments
        expect(scanner, parser, TOKEN_RIGHT_PAREN,
               "unclosed argument list of a function at"); // eat the ) => no arguments for now
        write_chunk(current_chunk(parser), OP_CALL);
    }

	
}

static void parse_return_statement(Parser* parser, Scanner* scanner) {
    if (parser->func->type == FN_SCRIPT) {
        error(parser, "invalid syntax at");
    }
    advance(scanner, parser); // eat the return keyword
    if (parser->current.type == TOKEN_SEMICOLON) { // if no expression was after the return, return nil;
        write_chunk(current_chunk(parser), OP_NIL);

    } else {
        parse_precedence(parser, scanner, PREC_OR); // parse the value
    }
    write_chunk(current_chunk(parser), OP_RETURN);
    expect(scanner, parser, TOKEN_SEMICOLON, "expected ; after statement got");
}

static void parse_func_statement(Parser* parser, Scanner* scanner) {
	advance(scanner, parser); // eat the fn keyword
	expect(scanner, parser, TOKEN_IDENTIFIER, "expected identifier at");
	Token func_tkn = parser->previous;
	expect(scanner, parser, TOKEN_LEFT_PAREN, "expected ( in function declaration at");
	// TODO: Add calling with arguments
	expect(scanner, parser, TOKEN_RIGHT_PAREN, "unclosed ) in function declaration in"); // no params currently


	

	expect(scanner, parser, TOKEN_LEFT_BRACE, "expected open block in function declaration"); // eat the {
	

	FunctionObj* obj = create_func_obj(func_tkn.start, func_tkn.length, FN_FUNCTION);
	FunctionObj* before_func = parser->func;
	parser->func = obj;

    // set the variable scope
    HashMap* saved_map = parser->varMap;
    unsigned int saved_count = parser->varCount;

    parser->varMap = (HashMap*) malloc(sizeof (HashMap));
    create_variable_map(parser->varMap);


	while (parser->current.type != TOKEN_EOF && parser->current.type != TOKEN_RIGHT_BRACE) {
		
		parse_statement(parser, scanner);
	}
    write_bytes(current_chunk(parser), OP_NIL, OP_RETURN);

    parser->varCount = saved_count;
    parser->varMap = saved_map;
    free_hash_map(parser->varMap);
	parser->func = before_func;
	expect(scanner, parser, TOKEN_RIGHT_BRACE, "unclosed block in function declaration"); // eat the }

	// Add function constant
	uint8_t index = add_constant(current_chunk(parser), VAR_OBJ(obj));
	write_bytes(current_chunk(parser), OP_CONSTANT, index);

	// register the function name
    unsigned int name_index = add_variable(parser, obj->name->value, obj->name->length);
	write_bytes(current_chunk(parser), OP_STORE_GLOBAL, name_index);



}


static void parse_var_assignment(Parser* parser, Scanner* scanner) {
	if (parser->current.type != TOKEN_EQUAL) {
		return parse_identifier(parser, scanner);
	}
    printf("!!!NOT IMPLEMENTED, VAR ASSIGN!!!\n");
	Token variable_ident = parser->previous;
	expect(scanner, parser, TOKEN_EQUAL, "expected '=' after variable declaration at");

	parse_precedence(parser, scanner, PREC_OR); // parse the expression value

	//// create the string object
	StringObj* obj = create_string_obj(variable_ident.start, variable_ident.length);
	uint8_t index = add_constant(current_chunk(parser), VAR_OBJ(obj));
	write_bytes(current_chunk(parser), OP_ASSIGN_GLOBAL, index);

}


static void parse_statement(Parser* parser, Scanner* scanner) {
	// parse statements that do not return anything
	// for ex: call(a,b,c);;
	switch (parser->current.type) {
	case TOKEN_IF: return parse_if_statement(parser, scanner);
	case TOKEN_PRINT: return parse_debug_statement(parser, scanner);
	case TOKEN_VAR: return parse_variable(parser, scanner);
	case TOKEN_FN: return parse_func_statement(parser, scanner);
    case TOKEN_RETURN: return parse_return_statement(parser, scanner);
	}
	parse_expression(parser, scanner);
	expect(scanner, parser, TOKEN_SEMICOLON, "expected ; at");
	write_chunk(current_chunk(parser), OP_POP_TOP);
}


static void parse_expression(Parser* parser, Scanner* scanner) {
	parse_precedence(parser, scanner, PREC_ASSIGNMENT);
}




static void end_compile(Parser* parser, Scanner* scanner) {
	if (parser->current.type == TOKEN_EOF) {
		write_chunk(current_chunk(parser), OP_HALT);
		return;
	}
	error(parser, "expected EOF at end of file got");
}

ParseRule rules[] = {
  [TOKEN_LEFT_PAREN] = {parse_grouping, NULL,   PREC_NONE},
  [TOKEN_RIGHT_PAREN] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_LEFT_BRACE] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_RIGHT_BRACE] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_COMMA] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_DOT] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_MINUS] = {parse_unary,    parse_binary, PREC_TERM},
  [TOKEN_PLUS] = {NULL,     parse_binary, PREC_TERM},
  [TOKEN_SEMICOLON] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_SLASH] = {NULL,     parse_binary, PREC_FACTOR},
  [TOKEN_STAR] = {NULL,     parse_binary, PREC_FACTOR},
  [TOKEN_BANG] = {parse_unary,     NULL,   PREC_NONE},
  [TOKEN_BANG_EQUAL] = {NULL,     parse_boolean,   PREC_EQUALITY},
  [TOKEN_EQUAL] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EQUAL_EQUAL] = {NULL,     parse_boolean,   PREC_EQUALITY},
  [TOKEN_GREATER] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_GREATER_EQUAL] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_LESS] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_LESS_EQUAL] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_IDENTIFIER] = {parse_var_assignment,     NULL,   PREC_NONE},
  [TOKEN_STRING] = {parse_string,     NULL,   PREC_NONE},
  [TOKEN_NUMBER] = {parse_number,   NULL,   PREC_NONE},
  [TOKEN_ELSE] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FALSE] = {parse_literal,     NULL,   PREC_NONE},
  [TOKEN_FOR] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_IF] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_NIL] = {parse_literal,     NULL,   PREC_NONE},
  [TOKEN_PRINT] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_RETURN] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_SUPER] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_THIS] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_TRUE] = {parse_literal,     NULL,   PREC_NONE},
  [TOKEN_VAR] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_WHILE] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_ERROR] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EOF] = {NULL,     NULL,   PREC_NONE},
};

static ParseRule* get_rule(uint8_t token) {
	return &rules[token];
}


FunctionObj* compile(const char* source) {
	// create objects
	Scanner scanner = create_token_scanner(source);
	Parser parser;

	// inits
	init_parser(&parser); // inits the parser

	advance(&scanner, &parser);
	while (parser.current.type != TOKEN_EOF) {
		parse_statement(&parser, &scanner);
	}
	// clean ups
	end_compile(&parser, &scanner); // end compilation



	return parser.hadError ? NULL : parser.func;
}