/* The functions in here are associated with the front-end of the compiler,
 * including opening and reading the source files as well as controlling
 * the flow of objects between the various stages of compilation.
 */

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"
#include "datastructures/datastructures.h"
#include "enums.h"
#include "init.h"
#include "lexer.h"
#include "misc.h"
#include "parse/parse.h"
#include "parse/parse_block_collect.h"
#include "parse/parse_type.h"

#define TEST_PARSER

char* program_source_buffer;
char* filename;

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

    program_source_buffer[i] = '\n';

    fclose(fp);

    return i;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Woah: error: no files supplied.\n");
        exit(NO_FILE_SUPPLIED);
    }

    get_keywords();

    filename              = get_filename(argc, argv);
    program_source_buffer = calloc(0x10000, sizeof(char));

    malloc_error(program_source_buffer, MAIN_PROGRAM_SOURCE_BUFFER);

    int psb_len  = get_file_contents();
    Array tokens = generate_tokens(program_source_buffer, psb_len);

#ifdef TEST_PARSER

    struct ParseExpr* expr = parse_general_expression(
        tokens, NULL, 0, tokens->index - 1
    );

    fprint_parse_expression(stdout, expr, program_source_buffer);
    printf("\n");

    exit(0);

#endif


    // Blocks defined in the file and in imports. Modified by collect_blocks()
    // defined in /src/parse/parse.c
    Array blocks[6] = { 
        make_array(), /* functions */
        make_array(), /* structs   */
        make_array(), /* types     */
        make_array(), /* imports   */
        make_array(), /* exports   */
        make_array()  /* globals   */
    };

    collect_blocks(tokens, blocks);

    // Currently I'm only going to implement functions, structs and globals.
    // This block will only exist until these features exist.
    {
        bool implemented_error = false;

        if (blocks[1]->index > 0) {
            error_message("Structs haven't been implemented yet.\n");
            implemented_error = true;
        }

        if (blocks[2]->index > 0) {
            error_message("Type definitions haven't been implemented yet.\n");
            implemented_error = true;
        }

        if (blocks[3]->index > 0) {
            error_message("Module imports haven't been implemented yet.\n");
            implemented_error = true;
        }

        if (blocks[4]->index > 0) {
            error_message("Module declaration and export hasn't been implemented yet.\n");
            implemented_error = true;
        }

        if (implemented_error) {
            exit(-1);
        }
    }
}
