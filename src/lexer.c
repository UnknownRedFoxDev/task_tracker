#include "../lib/lexer.h"
#include <ctype.h>

char peek(Lexer *l)
{
    return l->src[l->cursor];
}

void advance(Lexer *l)
{
    l->cursor++;
}

char consume(Lexer *l)
{
    return l->src[l->cursor++];
}

bool is_special_char(char c)
{
    return (c == ' ' || c == '\0' || c == ')' || c == '(');
}

Lexer *init_lexer(const char *query)
{
    Lexer *l = calloc(1, sizeof(Lexer));
    if (l == NULL) {
        nob_log(NOB_ERROR, "failed to allocate space for a lexer");
        exit(1);
    }

    l->src = strdup(query);
    return l;
}

// Credits to Tsoding: https://youtu.be/eGvUK-3RmDs?t=1504
void report_query_error(const char *src, int cursor, const char *format, ...) NOB_PRINTF_FORMAT(3, 4);
void report_query_error(const char *src, int cursor, const char *format, ...)
{
    fprintf(stderr, "%s\n", src);
    fprintf(stderr, "%*s\n", cursor, "^");
    fprintf(stderr, "ERROR: ");
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}


Token_t next_token(Lexer *l)
{
    while (true) {
        char c = peek(l);

        switch (c) {
        case '\0':
            return (Token_t){.kind = TOKEN_EOF}; // End of the query string
        case '(': {
            advance(l);
            l->curr_word_size = 1;
            return (Token_t){.kind = TOKEN_LPAREN};
        }
        case ')': {
            advance(l);
            l->curr_word_size = 1;
            return (Token_t){.kind = TOKEN_RPAREN};
        }
        case '.': {
            advance(l); // Passes the '.'
            char tag_name[TAG_NAME_SIZE] = {0};
            size_t i = 0;

            while (true) {
                c = peek(l);
                if (!is_special_char(c)) {
                    advance(l);
                    tag_name[i++] = c;
                } else {
                    break;
                }
            }
            l->curr_word_size = i;
            return (Token_t){.kind = TOKEN_TAG, .as = { .string = strdup(tag_name) }};
        }
        case ' ':
            advance(l);
            break;
        default: { // "and"/"or"/"not" keywords processing
            char keyword[KEYWORD_MAX_SIZE] = {0};
            size_t i = 0;
            while (i < KEYWORD_MAX_SIZE) {
                c = peek(l);
                if (!is_special_char(c)) {
                    advance(l);
                    keyword[i++] = c;
                } else {
                    break;
                }
            }

            l->curr_word_size = i;
            if (strcmp(keyword, "not") == 0) {
                return (Token_t){.kind = TOKEN_NOT};
            } else if (strcmp(keyword, "and") == 0) {
                return (Token_t){.kind = TOKEN_AND};
            } else if (strcmp(keyword, "or") == 0) {
                return (Token_t){.kind = TOKEN_OR};
            } else if (strcmp(keyword, "priority") == 0) {
                return (Token_t){.kind = TOKEN_PRIORITY};
            } else if (strcmp(keyword, "lte") == 0) {
                return (Token_t){.kind = TOKEN_LTE};
            } else if (strcmp(keyword, "gte") == 0) {
                return (Token_t){.kind = TOKEN_GTE};
            } else if (strcmp(keyword, "gt") == 0) {
                return (Token_t){.kind = TOKEN_GT};
            } else if (strcmp(keyword, "lt") == 0) {
                return (Token_t){.kind = TOKEN_LT};
            }
            char *endptr = NULL;
            long integer = strtol(keyword, &endptr, 10);
            if (keyword != endptr) {
                return (Token_t){.kind = TOKEN_INT, .as = { .integer = integer }};
            }

            report_query_error(l->src, l->cursor - i + 1, "unknown token found");
            abort();
            }
        }
    }
    UNREACHABLE("next_token");
}

void dump_token(Token_t t)
{
    switch(t.kind) {
    case TOKEN_TAG: {
        assert(t.as.string != NULL && "Tag's name is somehow invalid");
        nob_log(NOB_INFO, "type: Tag, name: %s", t.as.string);
        break;
    }
    case TOKEN_INT: {
        nob_log(NOB_INFO, "type: Tag, name: %ld", t.as.integer);
        break;
    }
    case TOKEN_NOT: {
        nob_log(NOB_INFO, "type: Not");
        break;
    }
    case TOKEN_AND: {
        nob_log(NOB_INFO, "type: And");
        break;
    }
    case TOKEN_OR: {
        nob_log(NOB_INFO, "type: Or");
        break;
    }
    case TOKEN_LPAREN: {
        nob_log(NOB_INFO, "type: Left Parenthesis");
        break;
    }
    case TOKEN_RPAREN: {
        nob_log(NOB_INFO, "type: Right Parenthesis");
        break;
    }
    case TOKEN_LT: {
        nob_log(NOB_INFO, "type: Less than");
        break;
    }
    case TOKEN_LTE: {
        nob_log(NOB_INFO, "type: Less than or equal");
        break;
    }
    case TOKEN_GT: {
        nob_log(NOB_INFO, "type: Greater than");
        break;
    }
    case TOKEN_GTE: {
        nob_log(NOB_INFO, "type: Greater than or equal");
        break;
    }
    case TOKEN_PRIORITY: {
        nob_log(NOB_INFO, "type: Priority");
        break;
    }
    case TOKEN_UNK: {
        assert(t.as.string != NULL && "Unknown keyword with invalid string, how swell");
        nob_log(NOB_INFO, "type: Unknown, string: %s", t.as.string);
        break;
    }
    default:
        break;
    }
}

void clean_lexer(Lexer **l)
{
    if (*l) {
        free((*l)->src);
    }

    free(*l);
    *l = NULL;
}

