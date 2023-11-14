#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include "include/parser.h"
#include "include/ast.h"

static bool is_whitespace(char rune) {
    switch (rune) {
        case ' ':
        case '\t':
        case '\r':
            return true;
        default:
            return false;
    }
}

static bool is_ident_start(char rune) {
    return isalpha(rune) || rune == '_';
}

typedef enum TokenType {
    EOF_TOKEN = -1,

    _literals_start,
    IDENT_TOKEN,
    _literals_end,

    _keywords_begin,
    CREATE_TOKEN,
    TABLE_TOKEN,

    _types_start,
    INTEGER_TOKEN,
    TEXT_TOKEN,
    _types_end,
    _keywords_end,

    _operators_start,
    OPEN_PAREN_TOKEN,
    CLOSE_PAREN_TOKEN,
    SEMICOLON_TOKEN,
    COMMA_TOKEN,
} TokenType;

typedef struct Token {
    TokenType type;
    const char *value;
    size_t length;
} Token;


typedef struct Tokenizer {
    const char *start;
    const char *current;
} Tokenizer;

static char advance_tokenizer(Tokenizer *tokenizer) {
    tokenizer->current++;
    return tokenizer->current[-1];
}

static char peek_tokenizer(Tokenizer *tokenizer) {
    return tokenizer->current[0];
}

static Token make_token(Tokenizer *tokenizer, TokenType type) {
    Token token;
    token.type = type;
    token.value = tokenizer->start;
    token.length = tokenizer->current - tokenizer->start;

    return token;
}

static void consume_whitespace(Tokenizer *tokenizer) {
    while(true) {
        char rune = peek_tokenizer(tokenizer);
        if (is_whitespace(rune)) {
            advance_tokenizer(tokenizer);
            continue;
        }
        return;
    }
}

static Token consume_ident(Tokenizer *tokenizer) {
    // @TODO: think about alternative implementations for this lookup.
    while (tokenizer->current[0] != '\0' && isalpha(tokenizer->current[0])) {
        advance_tokenizer(tokenizer);
    }

    size_t length = tokenizer->current - tokenizer->start;
    char ident[length];
    strncpy(ident, tokenizer->start, length);
    
    TokenType type = IDENT_TOKEN;
    if (strcmp(ident, "CREATE") == 0) {
        type = CREATE_TOKEN;
    } else if (strcmp(ident, "TABLE") == 0) {
        type = TABLE_TOKEN;
    } else if (strcmp(ident, "INTEGER") == 0) {
        type = INTEGER_TOKEN;
    } else if (strcmp(ident, "TEXT") == 0) {
        type = TEXT_TOKEN;
    }

    return make_token(tokenizer, type);
}

static Token next_token(Tokenizer *tokenizer) {
    consume_whitespace(tokenizer);
    tokenizer->start = tokenizer->current;

    if (tokenizer->current[0] == '\0') {
        return make_token(tokenizer, EOF_TOKEN);
    }

    char rune = advance_tokenizer(tokenizer);
    switch (rune) {
        case ',':
            return make_token(tokenizer, COMMA_TOKEN);
        case '(':
            return make_token(tokenizer, OPEN_PAREN_TOKEN);
        case ')':
            return make_token(tokenizer, CLOSE_PAREN_TOKEN);
        case ';':
            return make_token(tokenizer, SEMICOLON_TOKEN);
    }

    if (is_ident_start(rune)) {
        return consume_ident(tokenizer);
    }

    Token token;
    token.type = IDENT_TOKEN;
    return token;
}

typedef struct Parser {
    Tokenizer tokenizer;
    Token previous;
    Token current;
    bool had_error;
    bool is_panicked;
} Parser;

static Parser new_parser(const char *input) {
    Tokenizer tokenizer;
    tokenizer.start = input;
    tokenizer.current = input;

    Parser parser;
    parser.tokenizer = tokenizer;
    parser.had_error = false;
    parser.is_panicked = false;

    return parser;
}

static Token peek_parser(Parser *parser) {
    return parser->current;
}

static void advance_parser(Parser *parser) {
    parser->previous = parser->current;
    parser->current = next_token(&(parser->tokenizer));
    printf("%d '%.*s'\n", parser->current.type, parser->current.length, parser->current.value);
}

// Forward Declarations
static void parse_create_table_statement(Parser *parser);
static void parse_name(Parser *parser);
static void parse_column_list(Parser *parser);
static void parse_column_list_prime(Parser *parser);
static void parse_column(Parser *parser);
static void parse_type(Parser *parser);


void run_tasiadb_parser(const char *input) {
    printf("Parsing '%s'\n", input);

    Parser parser = new_parser(input);
    advance_parser(&parser);

    parse_create_table_statement(&parser);
    if (parser.had_error == false) {
        printf("Parsed successfully!\n");
    }
}

static void error(Parser *parser, const char *message) {
    if (parser->is_panicked) return;
    parser->is_panicked = true;
    printf("Parse Error: %s\n", message);
    free(message);
    parser->had_error = true;
}

static void expect_token(Parser *parser, TokenType expected) {
    Token token = peek_parser(parser);
    if (token.type == expected) {
        advance_parser(parser);
        return;
    }

    char *message;
    asprintf(&message, "Expected %d, Actual %d", expected, token.type);
    error(parser, message);
}

static void expect_token_class(Parser *parser, TokenType start, TokenType end) {
    Token token = peek_parser(parser);
    if (token.type > start && token.type < end) {
        advance_parser(parser);
        return;
    }

    error(parser, "Expected token in class");
}

// cts := CREATE TABLE name OPEN_PAREN columnlist CLOSE_PAREN SEMICOLON
static void parse_create_table_statement(Parser *parser) {
    expect_token(parser, CREATE_TOKEN);
    expect_token(parser, TABLE_TOKEN);

    parse_name(parser);

    expect_token(parser, OPEN_PAREN_TOKEN);
    parse_column_list(parser);
    expect_token(parser, CLOSE_PAREN_TOKEN);

    expect_token(parser, SEMICOLON_TOKEN);
}

// name := IDENT
static void parse_name(Parser *parser) {
    expect_token(parser, IDENT_TOKEN);
}

// columnlist := column columnlist’
static void parse_column_list(Parser *parser) {
    parse_column(parser);
    parse_column_list_prime(parser);
}

// columnlist’ := COMMA column columnlist’ | epsilon
static void parse_column_list_prime(Parser *parser) {
    Token token = peek_parser(parser);
    if (token.type == COMMA_TOKEN) {
        expect_token(parser, COMMA_TOKEN);
        parse_column(parser);
        parse_column_list_prime(parser);
    }
}

// column := name type
static void parse_column(Parser *parser) {
    parse_name(parser);
    parse_type(parser);
}

// type := INTEGER | TEXT
static void parse_type(Parser *parser) {
    expect_token_class(parser, _types_start, _types_end);
}