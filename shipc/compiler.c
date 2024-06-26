

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "compiler.h"
#include "token.h"
#include "objects.h"



// main parser struct
typedef struct {
	Token current;
	Token previous;

	FunctionObj* func;
    HashMap* varMap;

	bool hadError;
	bool panicMode;
} Parser;

static Chunk* current_chunk(Parser* parser) {
	return &parser->func->body;
}

static HashNode* get_variable(Parser* parser, char* name, int length) {
    return get_node(parser->varMap, name, length);
}

static unsigned int create_variable(Parser* parser, char* name, int length) {
    Local  local;
    local.name = name;
    local.value = VAR_NIL;
    local.length = length;
    parser->func->locals[parser->varMap->count] = local;

    put_node(parser->varMap, name, length, parser->varMap->count);
    return parser->varMap->count - 1;

}

static unsigned int add_variable(Parser* parser, char* name, int length) {
    HashNode* nd_exist = get_node(parser->varMap, name, length);
    if (nd_exist != NULL) {
        return nd_exist->value;
    }
    return create_variable(parser, name, length);
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
static void advance(Scanner* scanner, Parser* parser) ;
static void synchronize(Parser* parser, Scanner* scanner) ;




////





// error related functions
static void error(Parser* parser, Scanner* scanner, const char* message) {
    // find the first letter of the line
    int line_length = parser->current.lineOffset;
    char* temp = parser->current.start;
    while (*temp != ';' && *temp != '\n') {
        line_length++;
        temp++;
    }
    if (*temp == ';') {
        line_length++;
    }
	fprintf(stderr, "error: %s\n    [main.ship:%i:%i]\n    |\n%03i | %.*s\n    |%*s^^^^ \n",
            message, parser->current.line, parser->current.lineOffset, parser->current.line, line_length, parser->current.start - parser->current.lineOffset, parser->current.lineOffset, " ");
	parser->hadError = true;
    synchronize(parser, scanner);
}

static void custom_error(Parser* parser, const char* string, ...) {
    va_list args;
    va_start(args, string);
    vfprintf(stderr, string, args);
    va_end(args);
    parser->hadError = true;
}

static void synchronize(Parser* parser, Scanner* scanner) {
    for (;;) {
        switch(parser->current.type) {
            case TOKEN_EOF:
            case TOKEN_IF:
            case TOKEN_VAR:
            case TOKEN_FN:
            case TOKEN_WHILE:
            case TOKEN_SEMICOLON:
            case TOKEN_LEFT_PAREN:
                case TOKEN_LEFT_BRACE:
            case TOKEN_RIGHT_PAREN:
            case TOKEN_RIGHT_BRACE:
                return;
            default: {
                parser->previous = parser->current;
                parser->current = tokenize(scanner);
            }
        }

    }
}

////


static void init_parser(Parser* parser) {
	parser->hadError = false;
	parser->panicMode = false;

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
        error(parser,  scanner, "Unknown token encountered");

	}
}

static void expect(Scanner* scanner, Parser* parser, TokenType type, const char *message) {
	if (parser->current.type == type) {
		advance(scanner, parser);
		return;
	}
	error(parser, scanner, message);
}



static void parse_precedence(Parser* parser, Scanner* scanner, Precedence precedence) {
	advance(scanner, parser);
	ParseFn rule = get_rule(parser->previous.type)->prefix;
	if (rule == NULL) {
		error(parser, scanner, "expected expression");
		return;
	}
	rule(parser, scanner);

	while (precedence <= get_rule(parser->current.type)->precedence) {
		advance(scanner, parser);
		ParseFn infixRule = get_rule(parser->previous.type)->infix;
		infixRule(parser, scanner);
	}
}

static void parse_number(Parser* parser, Scanner* scanner) {
	double value = strtod(parser->previous.start, NULL);
	uint8_t index = add_constant(current_chunk(parser), VAR_NUMBER(value));
	write_bytes(current_chunk(parser), OP_CONSTANT, index, scanner->line);
}

static void parse_grouping(Parser* parser, Scanner* scanner) {
	parse_precedence(parser, scanner, PREC_OR);
	if (parser->current.type == TOKEN_RIGHT_PAREN) {
		advance(scanner, parser);
		return;
	}
	error(parser, scanner, "unclosed '(' block");
}

static void parse_boolean(Parser* parser, Scanner* scanner) {
	TokenType op = parser->previous.type;
	parse_precedence(parser, scanner, (Precedence)(get_rule(op)->precedence + 1));
	switch (op) {
	case TOKEN_EQUAL_EQUAL: write_chunk(current_chunk(parser), OP_COMPARE, scanner->line); break;
	case TOKEN_BANG_EQUAL: {
		write_chunk(current_chunk(parser), OP_COMPARE, scanner->line);
		write_chunk(current_chunk(parser), OP_NOT, scanner->line);
		break;
	}
    case TOKEN_LESS_EQUAL: {
        write_chunk(current_chunk(parser), OP_GREATER_THAN, scanner->line);
        write_chunk(current_chunk(parser), OP_NOT, scanner->line);
        break;
    }
    case TOKEN_GREATER_EQUAL: {
        write_chunk(current_chunk(parser), OP_LESS_THAN, scanner->line);
        write_chunk(current_chunk(parser), OP_NOT, scanner->line);
        break;
    }
    case TOKEN_GREATER: {
        write_chunk(current_chunk(parser), OP_GREATER_THAN, scanner->line);
        break;
    }
    case TOKEN_LESS: {
        write_chunk(current_chunk(parser), OP_LESS_THAN, scanner->line);
        break;
    }
        default:
            error(parser, scanner, "Unexpected Binary Token"); // unreachable

	}
}





static void parse_unary(Parser* parser, Scanner* scanner) {
	TokenType operatorType = parser->previous.type;

	// Compile the operand.
	parse_precedence(parser, scanner, PREC_UNARY);

	// Emit the operator instruction.
	switch (operatorType) {
	case TOKEN_MINUS: write_chunk(current_chunk(parser), OP_NEGATE, scanner->line); break;
	case TOKEN_BANG: write_chunk(current_chunk(parser), OP_NOT, scanner->line); break;
	default: return; // Unreachable.
	}
}

static void parse_string(Parser* parser, Scanner* scanner) {
	char* str = parser->previous.start + 1; // remove the first "
	int length = parser->previous.length - 2; // not including the 2 "

	// create the string object
	StringObj* obj = create_string_obj(str, length);
	uint8_t index = add_constant(current_chunk(parser), VAR_OBJ(obj));
	write_bytes(current_chunk(parser), OP_CONSTANT, index, scanner->line);

}

static void parse_binary(Parser* parser, Scanner* scanner) {
	TokenType op = parser->previous.type;
	parse_precedence(parser, scanner, (Precedence)(get_rule(op)->precedence + 1));

	switch (op) {
	case TOKEN_PLUS: write_chunk(current_chunk(parser), OP_ADD, scanner->line); break;
	case TOKEN_MINUS: write_chunk(current_chunk(parser), OP_SUB, scanner->line); break;
	case TOKEN_STAR: write_chunk(current_chunk(parser), OP_MUL, scanner->line); break;
	case TOKEN_SLASH: write_chunk(current_chunk(parser), OP_DIV, scanner->line); break;
    case TOKEN_MODULO: write_chunk(current_chunk(parser), OP_MODULO, scanner->line); break;
    default: return; // unreachable
	}
}




static void parse_literal(Parser* parser, Scanner* scanner) {
	switch (parser->previous.type) {
	case TOKEN_FALSE: write_chunk(current_chunk(parser), OP_FALSE, scanner->line); break;
	case TOKEN_TRUE: write_chunk(current_chunk(parser), OP_TRUE, scanner->line); break;
	case TOKEN_NIL: write_chunk(current_chunk(parser), OP_NIL, scanner->line); break;
    default: return; // unreachable
	}
}

static void parse_debug_statement(Parser* parser, Scanner* scanner) {
	advance(scanner, parser); // eat the (
    write_chunk(current_chunk(parser), OP_NIL, scanner->line); // push nil as the function doesn't return anything

	parse_expression(parser, scanner);
	expect(scanner, parser, TOKEN_RIGHT_PAREN, "Missing parentheses in call");
    write_chunk(current_chunk(parser), OP_SHOW_TOP, scanner->line);
}

static void parse_else_statement(Parser* parser, Scanner* scanner) {
    write_chunk(current_chunk(parser), OP_JUMP, scanner->line);
    int else_goto_offset = current_chunk(parser)->count;
    write_bytes(current_chunk(parser), 0xff, 0xff, scanner->line); // save the goto to jump over the else block


    advance(scanner, parser); // eat the while
    expect(scanner, parser, TOKEN_LEFT_BRACE, "Expected '{' after else expression");
    while (parser->current.type != TOKEN_RIGHT_BRACE && parser->current.type != TOKEN_EOF) {
        // parse the body of the if statement
        parse_statement(parser, scanner);
    }
    expect(scanner, parser, TOKEN_RIGHT_BRACE, "Unclosed '}' after block");

    // apply and calculate the new changes
    int jmp_size = current_chunk(parser)->count - else_goto_offset - 2;
    current_chunk(parser)->codes[else_goto_offset] = (jmp_size >> 8) & 0xff;
    current_chunk(parser)->codes[else_goto_offset + 1] = jmp_size & 0xff;
}

static void parse_if_statement(Parser* parser, Scanner* scanner) {
	// parse the bool expr
	parse_expression(parser, scanner);


	// add a temp value
	write_chunk(current_chunk(parser), OP_POP_JUMP_IF_FALSE, scanner->line);

	// save the value before the body
	int change_bytes_offset = current_chunk(parser)->count;
	write_bytes(current_chunk(parser), 0xff, 0xff, scanner->line);

	expect(scanner, parser, TOKEN_LEFT_BRACE, "Expected { after if expression"); // expect open block after boolean expression
	while (parser->current.type != TOKEN_RIGHT_BRACE && parser->current.type != TOKEN_EOF) {
		// parse the body of the if statement
		parse_statement(parser, scanner);
	}
	// expect user closing the if body
	expect(scanner, parser, TOKEN_RIGHT_BRACE, "Unclosed '}' after block");

	// calculate the new size of the body, and modify the jmp size
	int body_size = current_chunk(parser)->count - change_bytes_offset - 2;
	if (body_size > UINT16_MAX) {
		error(parser, scanner, "max jump length exceeded");
	}
    // test for else statement
    if (parser->current.type != TOKEN_ELSE) {
        current_chunk(parser)->codes[change_bytes_offset] = (body_size >> 8) & 0xff;
        current_chunk(parser)->codes[change_bytes_offset + 1] = body_size & 0xff;
        return;
    }

	// set the new size, the new size is 2 bytes long. to allow large jumps;
    body_size += 3; // add 3 because we add another jmp
	current_chunk(parser)->codes[change_bytes_offset] = (body_size >> 8) & 0xff;
	current_chunk(parser)->codes[change_bytes_offset + 1] = body_size & 0xff;

    parse_else_statement(parser, scanner);

}

static void parse_variable(Parser* parser, Scanner* scanner) {
	// parses a variable statement:
	// var x = 5;
	Token variable_ident = parser->current;

    // check if variable is already defined in the current scope
    if(get_variable(parser, variable_ident.start, variable_ident.length) != NULL) {
        error(parser, scanner, "redeclaration of variable");
        exit(1);
    }


	advance(scanner, parser);
	expect(scanner, parser, TOKEN_EQUAL, "Expected '=' after variable declaration");

	parse_precedence(parser, scanner, PREC_OR); // parse the expression value


    // add the variable | if we got here we know the variable is not initialized. therefore don't check if it exists.
    unsigned int var_index = create_variable(parser, variable_ident.start, variable_ident.length);
    write_bytes(current_chunk(parser), OP_STORE_FAST, var_index, scanner->line);
}

static void parse_var_assignment(Parser* parser, Scanner* scanner) {
    write_chunk(current_chunk(parser), OP_NIL, scanner->line);
    Token variable_ident = parser->previous;
    HashNode* stored_variable = get_variable(parser, variable_ident.start, variable_ident.length);
    if (stored_variable == NULL) {
        custom_error(parser, "variable '%.*s' is not defined in the current scope. did you mean 'glob %.*s = ...'", variable_ident.length, variable_ident.start, variable_ident.length, variable_ident.start);
        exit(1);
    }
    expect(scanner, parser, TOKEN_EQUAL, "Expected '=' after variable declaration at");

    parse_precedence(parser, scanner, PREC_OR); // parse the expression value

    write_bytes(current_chunk(parser), OP_ASSIGN_LOCAL, stored_variable->value, scanner->line);

}



static void parse_identifier(Parser* parser, Scanner* scanner) {
    if (parser->current.type == TOKEN_EQUAL) {
        return parse_var_assignment(parser, scanner);
    }

    // add the ident string to the pool so we can either call or load i
    HashNode *var = get_variable(parser, parser->previous.start, parser->previous.length);
    if (var == NULL) {
        StringObj *obj = create_string_obj(parser->previous.start, parser->previous.length);
        uint8_t string_index = add_constant(current_chunk(parser), VAR_OBJ(obj));
        write_bytes(current_chunk(parser), OP_LOAD_GLOBAL, string_index, scanner->line);
        return;
    }
    write_bytes(current_chunk(parser), OP_LOAD_LOCAL, var->value, scanner->line);
}

static void parse_call(Parser* parser, Scanner* scanner) {

    // parse the call argument
    uint8_t argument_call = 0;
    while (parser->current.type != TOKEN_EOF && parser->current.type != TOKEN_RIGHT_PAREN) {
        parse_precedence(parser, scanner, PREC_OR); // parse the argument
        if (parser->current.type != TOKEN_RIGHT_PAREN) {
            expect(scanner, parser, TOKEN_COMMA, "Unexpected token. expected ',' between function arguments");
        }
        argument_call++;
    }
    if (argument_call >= UINT8_MAX) {
        custom_error(parser, "Too much call arguments");
    }

    expect(scanner, parser, TOKEN_RIGHT_PAREN,
           "Unclosed argument list of a function"); // eat the  => no arguments for now
           write_bytes(current_chunk(parser), OP_CALL, argument_call, scanner->line);

	
}

static void parse_return_statement(Parser* parser, Scanner* scanner) {
    if (parser->func->type == FN_SCRIPT) {
        parser->current = parser->previous;
        error(parser, scanner,  "Return keyword outside of function");
    }
    if (parser->current.type == TOKEN_SEMICOLON) { // if no expression was after the return, return nil;
        write_chunk(current_chunk(parser), OP_NIL, scanner->line);

    } else {
        parse_precedence(parser, scanner, PREC_OR); // parse the value
    }
    write_chunk(current_chunk(parser), OP_RETURN, scanner->line);
}

static void parse_array_literal(Parser* parser, Scanner* scanner) {
    unsigned int item_count = 0;
    while (parser->current.type != TOKEN_RIGHT_SQUARE_BRACE && parser->current.type != TOKEN_EOF) {
        parse_precedence(parser, scanner, PREC_OR);
        item_count++;
        if (parser->current.type != TOKEN_RIGHT_SQUARE_BRACE) {
            expect(scanner, parser, TOKEN_COMMA, "Expected , between array values");
        }
    }
    if (item_count > UINT8_MAX) {
        error(parser, scanner, "Array literal length is too large");
        return;
    }
    expect(scanner, parser, TOKEN_RIGHT_SQUARE_BRACE, "Unclosed array literal");
    write_bytes(current_chunk(parser), OP_BUILD_ARRAY, item_count, scanner->line);


}

static void parse_func_statement(Parser* parser, Scanner* scanner) {
	expect(scanner, parser, TOKEN_IDENTIFIER, "Expected identifier");

    // create required objects
	Token func_tkn = parser->previous;
    FunctionObj* obj = create_func_obj(func_tkn.start, func_tkn.length, FN_FUNCTION);
    FunctionObj* before_func = parser->func;
    parser->func = obj;

    // set the variable scope
    HashMap* saved_map = parser->varMap;

    parser->varMap = (HashMap*) malloc(sizeof (HashMap));
    create_variable_map(parser->varMap);

    expect(scanner, parser, TOKEN_LEFT_PAREN, "Expected ( in function declaration");

    // parse arguments
    while (parser->current.type != TOKEN_RIGHT_PAREN && parser->current.type != TOKEN_EOF) {
        if (parser->current.type != TOKEN_IDENTIFIER){
            error(parser, scanner, "Unexpected token");
        }
        add_variable(parser, parser->current.start, parser->current.length);
        advance(scanner, parser);

        if (parser->current.type == TOKEN_EOF || parser->current.type == TOKEN_RIGHT_PAREN) {
            continue;
        }
        expect(scanner, parser, TOKEN_COMMA, "Expected ',' between function arguments");
    }

	expect(scanner, parser, TOKEN_RIGHT_PAREN, "Unclosed ) in function declaration in"); // no params currently
	expect(scanner, parser, TOKEN_LEFT_BRACE, "Expected open block in function declaration"); // eat the {
	


	while (parser->current.type != TOKEN_EOF && parser->current.type != TOKEN_RIGHT_BRACE) {
		
		parse_statement(parser, scanner);
	}
    write_bytes(current_chunk(parser), OP_NIL, OP_RETURN, scanner->line);

    parser->func->localCount = parser->varMap->count;

    // Free the hashmap AFTER assigning the right localCount.
    free_hash_map(parser->varMap);


    parser->varMap = saved_map;
	parser->func = before_func;
	expect(scanner, parser, TOKEN_RIGHT_BRACE, "Unclosed block in function declaration"); // eat the }

	// Add function constant
	uint8_t index = add_constant(current_chunk(parser), VAR_OBJ(obj));
	write_bytes(current_chunk(parser), OP_CONSTANT, index, scanner->line);

	// register the function name
    unsigned int name_index = add_variable(parser, obj->name->value, obj->name->length);
	write_bytes(current_chunk(parser), OP_STORE_FAST, name_index, scanner->line);



}


static void parse_global_statement(Parser* parser, Scanner* scanner) {
    advance(scanner, parser); // eat the glob keyword
    Token variable_ident = parser->current;
    advance(scanner, parser);
    expect(scanner, parser, TOKEN_EQUAL, "Expected '=' after global assignment");

    parse_precedence(parser, scanner, PREC_OR);
    expect(scanner, parser, TOKEN_SEMICOLON, "Expected ;");

    StringObj* obj =create_string_obj(variable_ident.start, variable_ident.length);
    uint8_t index = add_constant(current_chunk(parser), VAR_OBJ(obj));
    write_bytes(current_chunk(parser), OP_ASSIGN_GLOBAL, index, scanner->line);

}

static void parse_foreach_statement(Parser* parser, Scanner* scanner) {
    // foreach str |char| {
    //  print(char);
    //}
    //
    parse_expression(parser, scanner); // parse the str(iterable)
    expect(scanner, parser, TOKEN_VERTICAL_BAR, "expected | after foreach identifier");
    expect(scanner, parser, TOKEN_IDENTIFIER, "expected identifier inside vertical bars");


    // Create the iter object
    write_chunk(current_chunk(parser), OP_GET_ITER, scanner->line);


    int jmp_to = current_chunk(parser)->count;
    write_chunk(current_chunk(parser), OP_FOR_ITER, scanner->line);
    write_bytes(current_chunk(parser), 0xff, 0xff, scanner->line);

    // Create the loop variable
    Token variable_ident = parser->previous;
    unsigned int var_index = add_variable(parser, variable_ident.start, variable_ident.length);
    write_bytes(current_chunk(parser), OP_STORE_FAST, var_index, scanner->line);

    expect(scanner, parser, TOKEN_VERTICAL_BAR, "unclosed | in foreach");
    expect(scanner, parser, TOKEN_LEFT_BRACE, "Expected { after if expression"); // expect open block after boolean expression
    while (parser->current.type != TOKEN_RIGHT_BRACE && parser->current.type != TOKEN_EOF) {
        // parse the body of the if statement
        parse_statement(parser, scanner);
    }
    // expect user closing if body
    expect(scanner, parser, TOKEN_RIGHT_BRACE, "Expected } after open block");
    write_chunk(current_chunk(parser), OP_JUMP_BACKWARD, scanner->line);

    // Set the jump size
    int body_size = current_chunk(parser)->count - jmp_to + 2;
    write_bytes(current_chunk(parser), (body_size >> 8) & 0xff, body_size & 0xff, scanner->line);

    // Set the jump over the for jmp
    int jmp_over_size = body_size - 3;
    current_chunk(parser)->codes[jmp_to + 1] =(jmp_over_size >> 8) & 0xff;
    current_chunk(parser)->codes[jmp_to + 2] = jmp_over_size & 0xff;
    write_chunk(current_chunk(parser), OP_END_FOR, scanner->line);
}

static void parse_attribute(Parser * parser, Scanner *scanner) {
    if (parser->current.type != TOKEN_IDENTIFIER) {
        error(parser, scanner, "Expected identifier");
        return;
    }
    advance(scanner, parser);
    StringObj* attribute_name = create_string_obj(parser->previous.start, parser->previous.length);

    uint8_t const_index = add_constant(current_chunk(parser), VAR_OBJ(attribute_name));
    write_bytes(current_chunk(parser), OP_LOAD_ATTR, const_index, scanner->line);

}

static void parse_while_statement(Parser* parser, Scanner* scanner) {
    // save the before bool expr
    int before_bool = current_chunk(parser)->count;

    // parse the bool expr
    parse_expression(parser, scanner);


    // add a temp value
    write_chunk(current_chunk(parser), OP_POP_JUMP_IF_FALSE, scanner->line);

    // save the value before the body
    int offset = current_chunk(parser)->count;
    write_bytes(current_chunk(parser), 0xff, 0xff, scanner->line);

    expect(scanner, parser, TOKEN_LEFT_BRACE, "Expected { after if expression"); // expect open block after boolean expression
    while (parser->current.type != TOKEN_RIGHT_BRACE && parser->current.type != TOKEN_EOF) {
        // parse the body of the if statement
        parse_statement(parser, scanner);
    }
    // expect user closing the if body
    expect(scanner, parser, TOKEN_RIGHT_BRACE, "Expected } after open block");


    // calculate the new size of the body, and modify the jmp size
    int after_body = current_chunk(parser)->count; // add 3 because the JMP_BACK instruction is 3 uint8_t long
    int body_size = after_body - offset + 1;
    if (body_size > UINT16_MAX) {
        error(parser, scanner, "Max jump length exceeded");
    }

    // set the new size
    current_chunk(parser)->codes[offset] = (body_size >> 8) & 0xff;
    current_chunk(parser)->codes[offset + 1] = body_size & 0xff;

    // set the jump to the prev
    write_chunk(current_chunk(parser), OP_JUMP_BACKWARD, scanner->line);
    int total_loop_jump_size = current_chunk(parser)->count + 2 - before_bool; // add 2 to account for the upcoming 2 bytes of data vvvvvvvv
    write_bytes(current_chunk(parser), (total_loop_jump_size >> 8) & 0xff, total_loop_jump_size & 0xff, scanner->line);

}

static void parse_expression_statement(Parser* parser, Scanner* scanner) {
	// parse statements that do not return anything
	// for ex: call(a,b,c);
	parse_expression(parser, scanner);
	expect(scanner, parser, TOKEN_SEMICOLON, "Expected ;");
	write_chunk(current_chunk(parser), OP_POP_TOP, scanner->line);
}

static void parse_control_statement(Parser* parser, Scanner* scanner) {
    parse_expression(parser, scanner);
}
static void parse_declaration_statement(Parser* parser, Scanner* scanner) {
    parse_expression(parser, scanner);
    expect(scanner, parser, TOKEN_SEMICOLON, "Expected ;");
}

static void parse_statement(Parser* parser, Scanner* scanner) {
    switch (parser->current.type) {
        case TOKEN_FOR:
        case TOKEN_IF:
        case TOKEN_FN:
        case TOKEN_FOREACH:
        case TOKEN_WHILE:
            return parse_control_statement(parser, scanner);
        case TOKEN_VAR:
        case TOKEN_RETURN:
            return parse_declaration_statement(parser, scanner);
        default:
            return parse_expression_statement(parser, scanner);
    }
}


static void parse_expression(Parser* parser, Scanner* scanner) {
	parse_precedence(parser, scanner, PREC_ASSIGNMENT);
}




static void end_compile(Parser* parser, Scanner* scanner) {
	if (parser->current.type == TOKEN_EOF) {
		write_chunk(current_chunk(parser), OP_HALT, scanner->line);

        parser->func->localCount = parser->varMap->count;
        free_hash_map(parser->varMap);

		return;
	}
	error(parser, scanner, "Expected EOF at end of file");
}

ParseRule rules[] = {
  [TOKEN_LEFT_PAREN] = {parse_grouping, parse_call, PREC_CALL},
  [TOKEN_RIGHT_PAREN] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_LEFT_BRACE] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_RIGHT_BRACE] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_LEFT_SQUARE_BRACE] = {parse_array_literal, NULL, PREC_NONE},
  [TOKEN_RIGHT_SQUARE_BRACE] = {NULL, NULL, PREC_NONE},
  [TOKEN_COMMA] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_DOT] = {NULL,     parse_attribute,   PREC_CALL},
  [TOKEN_MINUS] = {parse_unary,    parse_binary, PREC_TERM},
  [TOKEN_PLUS] = {NULL,     parse_binary, PREC_TERM},
  [TOKEN_SEMICOLON] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_SLASH] = {NULL,     parse_binary, PREC_FACTOR},
  [TOKEN_MODULO] = {NULL, parse_binary, PREC_FACTOR},
  [TOKEN_STAR] = {NULL,     parse_binary, PREC_FACTOR},
  [TOKEN_BANG] = {parse_unary,     NULL,   PREC_NONE},
  [TOKEN_BANG_EQUAL] = {NULL,     parse_boolean,   PREC_EQUALITY},
  [TOKEN_EQUAL] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EQUAL_EQUAL] = {NULL,     parse_boolean,   PREC_EQUALITY},
  [TOKEN_GREATER] = {NULL,     parse_boolean,   PREC_EQUALITY},
  [TOKEN_GREATER_EQUAL] = {NULL,     parse_boolean,   PREC_EQUALITY},
  [TOKEN_LESS] = {NULL,     parse_boolean,   PREC_EQUALITY},
  [TOKEN_LESS_EQUAL] = {NULL,     parse_boolean,   PREC_EQUALITY},
  [TOKEN_IDENTIFIER] = {parse_identifier,    NULL,   PREC_NONE},
  [TOKEN_STRING] = {parse_string,     NULL,   PREC_NONE},
  [TOKEN_NUMBER] = {parse_number,   NULL,   PREC_NONE},
  [TOKEN_ELSE] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FALSE] = {parse_literal,     NULL,   PREC_NONE},
  [TOKEN_FN] = {parse_func_statement, NULL, PREC_NONE},
  [TOKEN_FOR] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FOREACH] = {parse_foreach_statement,     NULL,   PREC_NONE},
  [TOKEN_VERTICAL_BAR] = {NULL, NULL, PREC_NONE},
  [TOKEN_IF] = {parse_if_statement,     NULL,   PREC_NONE},
  [TOKEN_NIL] = {parse_literal,     NULL,   PREC_NONE},
  [TOKEN_PRINT] = {parse_debug_statement,     NULL,   PREC_NONE},
  [TOKEN_RETURN] = {parse_return_statement,     NULL,   PREC_NONE},
  [TOKEN_SUPER] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_THIS] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_TRUE] = {parse_literal,     NULL,   PREC_NONE},
  [TOKEN_VAR] = {parse_variable,     NULL,   PREC_NONE},
  [TOKEN_WHILE] = {parse_while_statement,     NULL,   PREC_NONE},
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
