#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"
#include "datastructures/datastructures.h"
#include "enums.h"
#include "lexer.h"
#include "parse/parse.h"
#include "parse/parse_type.h"

extern char* program_source_buffer;
extern char* filename;

char* get_filename(int argc, char *argv[])
{
    /* Returns the string in argv containing the filename */
    for (int arg_index = 0; arg_index < argc; arg_index++) {
        int end_index  = 0;

        while (argv[arg_index][end_index] != '\x00') {
            end_index++;
        }

        if (!strncmp(".woah", &argv[arg_index][end_index - 5], 5)) {
            return argv[arg_index];
        }
    }

    fprintf(stderr, "Woah: error: no files supplied\n");
    exit(NO_FILE_SUPPLIED);
}

// Reads the contents of filename into program_source_buffer
int get_file_contents()
{
   /* Opens the source file, reads the contents into program_source_buffer
    * and returns its length.
    */
    FILE* fp = fopen(filename, "r");
    char  c;
    int   i  = 0;

    if (fp == NULL) {
        perror("Woah: error opening file");
        exit(errno);
    }

    while ((c = getc(fp)) != EOF) {
        program_source_buffer[i++] = c;
    }

    fclose(fp);

    return i;
}

void printn(char* array, int start, int end)
{
    /* Prints a string between start and end */
    for (; start < end; start++) {
        printf("%c", array[start]);
    }

    printf("'\n");
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Woah: error: no files supplied.\n");
        exit(NO_FILE_SUPPLIED);
    }

    filename              = get_filename(argc, argv);
    program_source_buffer = calloc(0x10000, sizeof(char));

    HANDLEMALLOCERR(program_source_buffer, SOURCE_BUF);

    int contents_buffer_len = get_file_contents();
    Array tokens = generate_tokens(program_source_buffer, contents_buffer_len);

    // Blocks defined in the file and in imports. Modified by collect_blocks()
    // defined in /src/parse/parse.c
    Array blocks[4] = { 
        make_array(),   // functions
        make_array(),   // structs
        make_array(),   // types
        make_array()    // module
    };


// The main function right now is more of a platform for debugging existing
// parts of the compiler as they're written.
#ifdef PRINT_TOKENS

    for (int i = 0; i < tokens->index; i++) {
        struct Token* tok = tokens->buffer[i];

        printf("line = %d, column = %d, token_type = %d, string = '", tok->line_no, tok->col_no, tok->token_type);
        printn(program_source_buffer, tok->start_i, tok->end_i);

        if (i == 1) {
            printf("tokens[1] == '[: %d\n", tok->token_type == T_OPEN_SQ_BRKT);
        }
    }

#endif

    struct WType* type = new_parse_type(tokens, 0, tokens->index - 1);

    printf("type form number: %d\n", type->type_form);
}
