#include "lexer.h"

Array generate_tokens(char *source, int source_len)
{
    Array tokens = makeArray(TOKENS_ARRAY_SIZE);
    int i = 0, line_no = 1;

    // Iterate on each new token
    while (i < source_len) {
        struct Token *tok = calloc(1, sizeof(struct Token));
        bool token_type_determined = true;

        // Skip all whitespace and comments
        if (source[i] == ' ') {
            i++;
            continue;
        } else if (source[i] == '/' && source[i + 1] == '*') {
            i += 2;

            while (!(source[i - 1] == '*' && source[i] == '/')) {
                if (source[i++] == '\n') {
                    line_no++;
                }
            }

            continue;
        } else if (source[i] == '/' && source[i] == '/') {
            while (source[i] != '\n') {
                i++;
            }

            continue;
        }

        tok->start_i = i++;

        // Go through single character tokens
        switch (source[i - 1]) {
            case '\n':
                tok->token_type = T_NEWLINE;
                line_no++;
                break;
            case '(':
                tok->token_type = T_OPEN_BRKT;
                break;
            case ')':
                tok->token_type = T_CLOSE_BRKT;
                break;
            case '[':
                tok->token_type = T_OPEN_SQ_BRKT;
                break;
            case ']':
                tok->token_type = T_CLOSE_SQ_BRKT;
                break;
            case '{':
                tok->token_type = T_OPEN_CURLY_BRKT;
                break;
            case '}':
                tok->token_type = T_CLOSE_SQ_BRKT;
                break;
            case ',':
                tok->token_type = T_COMMA;
                break;
            case '.':
                tok->token_type = T_FULL_STOP;
                break;
            case ':':
                tok->token_type = T_COLON;
                break;
            case ';':
                tok->token_type = T_SEMICOLON;
                break;
            case '&':
                tok->token_type = T_AMPERSAND;
                break;
            case '@':
                tok->token_type = T_AT_SYM;
                break;
            case '"':
                tok->token_type = T_DBL_QUOT;
                break;
            case '\'':
                tok->token_type = T_SGL_QUOT;
                break;
            case '=':
                if ((tok->token_type = eq_type(source, i - 1)) == T_EQ) {
                    i++;
                }

                break;
            case '<':
                if ((tok->token_type = lt_type(source, i - 1)) == T_LEQ || tok->token_type == T_SHL) {
                    i++;
                }

                if (tok->token_type == T_SWAP) {
                    i++;
                }

                break;
            case '>':
                if ((tok->token_type = gt_type(source, i - 1)) == T_GEQ || tok->token_type == T_SHR) {
                    i++;
                }

                break;
            case '/':
                if (source[i] == '=') {
                    tok->token_type = T_DIV_ASSIGN;
                    i++;
                } else {
                    tok->token_type = T_FWD_SLASH;
                }

                break;
            case '\\':
                tok->token_type = T_BACKSLASH;
                break;
            case '+':
                if ((tok->token_type = add_type(source, i - 1)) == T_ADD_ASSIGN
                ||   tok->token_type == T_INC) {
                    i++;
                }

                break;
            case '-':
                if ((tok->token_type = hyp_type(source, i - 1)) == T_SUB_ASSIGN
                ||   tok->token_type == T_ARROW
                ||   tok->token_type == T_DEC) {
                    i++;
                }

                break;
            case '*':
                if (source[i] == '=') {
                    tok->token_type = T_MUL_ASSIGN;
                    i++;
                } else {
                    tok->token_type = T_ASTERISK;
                }

                break;
            default:
                token_type_determined = false;
                i--;
                break;
        }

        // Testing if the token is a word
        if (!token_type_determined) {
            int depth = 0;

            while ((96 < source[i] && source[i] < 123)
                || (64 < source[i] && source[i] < 91)
                || (0 < depth && (47 < source[i] && source[i] < 58))
                || source[i] == '_') {
                token_type_determined = true;
                depth++;
                i++;
            }

            // Check if the word is a keyword
            if (strncmp("fn ", &source[tok->start_i], 3)) {
                tok->token_type = T_FN;
            } else if (strncmp("struct ", &source[tok->start_i], 7)) {
                tok->token_type = T_STRUCT;
            } else if (strncmp("use ", &source[tok->start_i], 4)) {
                tok->token_type = T_USE;
            } else if (strncmp("while ", &source[tok->start_i], 6)) {
                tok->token_type = T_WHILE;
            } else if (strncmp("for ", &source[tok->start_i], 4)) {
                tok->token_type = T_WHILE;
            } else if (strncmp("if ", &source[tok->start_i], 3)) {
                tok->token_type = T_IF;
            } else if (strncmp("elif ", &source[tok->start_i], 5)) {
                tok->token_type = T_ELIF;
            } else if (strncmp("else ", &source[tok->start_i], 5)) {
                tok->token_type = T_ELSE;
            } else if (strncmp("in ", &source[tok->start_i], 3)) {
                tok->token_type = T_IN;
            } else {
                tok->token_type = T_NAME;
            }
        }

        // Testing if the token is a numeric literal
        if (!token_type_determined) {
            if (source[i] == '0' && source[i + 1] == 'b') {
                tok->token_type = T_B2NUM;
                i += 2;

                while (source[i] == '0' || source[i] == '1' || source[i] == '_') {
                    i++;
                }
            } else if (source[i] == '0' && (source[i + 1] == 'x' || source[i + 1] == 'X')) {
                tok->token_type = T_B16NUM;
                i += 2;

                while ((47 < source[i] && source[i] < 58)
                    || (64 < source[i] && source[i] < 71)
                    || (96 < source[i] && source[i] < 103)
                    || source[i] == '_') {
                        i++;
                }
            } else if ((47 < source[i] && source[i] < 58) || source[i] == '_') {
                while ((47 < source[i] && source[i] < 58) || source[i] == '_') {
                    i++;
                }
            } else {
                fprintf(stderr, "Woah: Syntax error on line %d:", line_no);
                fprintf(stderr, "Unknown syntax '");

                for (int j = tok->start_i; source[j] != '\n'; j++) {
                    fprintf(stderr, "%c", source[j]);
                }

                fprintf(stderr, "'\n");
                exit(SYNTAX_ERROR);
            }
        }

        tok->end_i   = i;
        tok->line_no = line_no;
        arrayAdd(tokens, tok);
    }

    return tokens;
}

// is the token = or ==
int eq_type(char *source, int index)
{
    if (source[index + 1] == '=') {
        return T_EQ;
    } else {
        return T_EQUALS;
    }
}

// is the token <, << or <=
int lt_type(char *source, int index)
{
    switch (source[index + 1]) {
        case '<':
            return T_SHL;
        case '=':
            return T_LEQ;
        case '-':
            if (source[index + 2] == '>') {
                return T_SWAP;
            }
        default:
            return T_LT;
    }
}

// is the token >, >> or >=
int gt_type(char *source, int index)
{
    switch (source[index + 1]) {
        case '>':
            return T_SHR;
        case '=':
            return T_GEQ;
        default:
            return T_GT;
    }
}

// is the token +, += or ++
int add_type(char *source, int index)
{
    switch (source[index + 1]) {
        case '=':
            return T_ADD_ASSIGN;
        case '+':
            return T_INC;
        default:
            return T_PLUS;
    }
}

// is the token -, -=, -> or --
int hyp_type(char *source, int index)
{
    switch (source[index + 1]) {
        case '=':
            return T_SUB_ASSIGN;
        case '>':
            return T_ARROW;
        case '-':
            return T_DEC;
        default:
            return T_MINUS;
    }
}
