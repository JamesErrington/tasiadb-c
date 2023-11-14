#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include "include/parser.h"

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
    NULL_TOKEN,
    INTEGER_TOKEN,
    REAL_TOKEN,
    TEXT_TOKEN,
    BLOB_TOKEN,
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
    } else if (strcmp(ident, "NULL") == 0) {
        type = NULL_TOKEN;
    } else if (strcmp(ident, "INTEGER") == 0) {
        type = INTEGER_TOKEN;
    } else if (strcmp(ident, "REAL") == 0) {
        type = REAL_TOKEN;
    } else if (strcmp(ident, "TEXT") == 0) {
        type = TEXT_TOKEN;
    } else if (strcmp(ident, "BLOB") == 0) {
        type = BLOB_TOKEN;
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
} Parser;

static Parser new_parser(const char *input) {
    Tokenizer tokenizer;
    tokenizer.start = input;
    tokenizer.current = input;

    Parser parser;
    parser.tokenizer = tokenizer;

    return parser;
}

static void advance_parser(Parser *parser) {
    parser->previous = parser->current;
    parser->current = next_token(&(parser->tokenizer));
    printf("%d '%.*s'\n", parser->current.type, parser->current.length, parser->current.value);
}

void run_tasiadb_parser(const char *input) {
    printf("Parsing '%s'\n", input);

    Parser parser = new_parser(input);
    advance_parser(&parser);
}