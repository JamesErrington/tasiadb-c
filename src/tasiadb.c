#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

static const char *prompt = "tasiadb> ";

// Caller must free() returned line
static char *read_one_line(FILE *inputfd) {
    char *line = NULL;
    size_t len = 0;
    ssize_t read = getline(&line, &len, inputfd);
    // Remove newline at end if detected.
    if (read > 0 && line[read - 1] == '\n') {
        line[read - 1] = '\0';
    }

    return line;
}

typedef enum {
    OK = 0,
    ERROR = 1,
    EXIT = 2,
} MetaReturn;

static MetaReturn do_meta_command(char *line) {
    // @TODO: Refactor conditionals
    if (strcmp(line, ".help") == 0) {
        printf(".exit Exit this program.\n");
        printf(".help Display help text.\n");
        return OK;
    } else if (strcmp(line, ".exit") == 0) {
        return EXIT;
    }

    printf("Unknown command '%s'. Enter '.help' for help.\n", line);
    return ERROR;
}

static bool is_alpha(char rune) {
    return (rune >= 'a' && rune <= 'z') || (rune >= 'A' && rune <= 'Z');
}

static bool is_ident_start(char rune) {
    return is_alpha(rune) || rune == '_';
}

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

typedef enum {
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

typedef struct {
    TokenType type;
    const char *value;
    size_t length;
} Token;

typedef struct {
    const char *start;
    const char *current;
    size_t line;
} Tokenizer;

static Tokenizer init_tokenizer(const char *input) {
    Tokenizer tokenizer;

    tokenizer.start = input;
    tokenizer.current = input;
    tokenizer.line = 1;

    return tokenizer;
}

static char advance(Tokenizer *tokenizer) {
    tokenizer->current++;
    return tokenizer->current[-1];
}

static char peek(Tokenizer *tokenizer) {
    return tokenizer->current[0];
}

static Token make_token(Tokenizer *tokenizer, TokenType type) {
    Token token;

    token.type = type;
    token.value = tokenizer->start;
    token.length = tokenizer->current - tokenizer->start;

    return token;
}

static Token consume_ident(Tokenizer *tokenizer) {
    // @TODO: think about alternative implementations for this lookup.
    while (tokenizer->current[0] != '\0' && is_alpha(tokenizer->current[0])) {
        advance(tokenizer);
    }

    size_t length = tokenizer->current - tokenizer->start;
    char ident[length];
    strncpy(ident, tokenizer->start, length);
    
    if (strcmp(ident, "CREATE") == 0)
    {
        return make_token(tokenizer, CREATE_TOKEN);
    }
    else if (strcmp(ident, "TABLE") == 0)
    {
        return make_token(tokenizer, TABLE_TOKEN);
    }
    else if (strcmp(ident, "NULL") == 0)
    {
        return make_token(tokenizer, NULL_TOKEN);
    }
    else if (strcmp(ident, "INTEGER") == 0)
    {
        return make_token(tokenizer, INTEGER_TOKEN);
    }
    else if (strcmp(ident, "REAL") == 0)
    {
        return make_token(tokenizer, REAL_TOKEN);
    }
    else if (strcmp(ident, "TEXT") == 0)
    {
        return make_token(tokenizer, TEXT_TOKEN);
    }
    else if (strcmp(ident, "BLOB") == 0)
    {
        return make_token(tokenizer, BLOB_TOKEN);
    }

    return make_token(tokenizer, IDENT_TOKEN);
}

static void consume_whitespace(Tokenizer *tokenizer) {
    while(true) {
        char rune = peek(tokenizer);
        if (is_whitespace(rune))
        {
            advance(tokenizer);
        }
        else
        {
            return;
        }
    }
}

static Token next_token(Tokenizer *tokenizer) {
    consume_whitespace(tokenizer);
    tokenizer->start = tokenizer->current;

    if (tokenizer->current[0] == '\0') {
        return make_token(tokenizer, EOF_TOKEN);
    }

    char rune = advance(tokenizer);
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

static void parse_sql_command(const char *line) {
    size_t length = strlen(line);
    printf("Parsing '%s': length %zu\n", line, length);

    Tokenizer tokenizer = init_tokenizer(line);

    Token token;
    while (true) {
        token = next_token(&tokenizer);
        if (token.type == EOF_TOKEN) {
            printf("Breaking\n");
            break;
        }

        printf("%d: (%.*s)\n", token.type, token.length, token.value);
    }
}

static void process_input() {
    char *line = NULL;

    while (true) {
        printf("%s", prompt);
        line = read_one_line(stdin);

        if (line && (line[0] == '.'))
        {
            int code = do_meta_command(line);

            if (code == EXIT) {
                free(line);
                break;
            }
        }
        else
        {
            parse_sql_command(line);
        }

        free(line);
    }
}

int main() {
    process_input();
}

