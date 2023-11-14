#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "include/parser.h"

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
            run_tasiadb_parser(line);
        }

        free(line);
    }
}

int main() {
    process_input();
}

