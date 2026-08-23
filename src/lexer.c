#include "../lib/lexer.h"

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

Token_t next_token(Lexer *l)
{
    while (true) {
        char c = peek(l);

        switch (c) {
        case '\0':
            return (Token_t){.kind = TOKEN_EOF}; // End of the query string
        case '(': {
            advance(l);
            return (Token_t){.kind = TOKEN_LPAREN};
        }
        case ')': {
            advance(l);
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
            return (Token_t){.kind = TOKEN_TAG, .string = strdup(tag_name)};
        }
        case ' ':
            advance(l);
            break;
        default: { // "and"/"or"/"not" keywords processing
            char keyword[KEYWORD_MAX_SIZE] = {0};
            size_t i = 0;
            while (true) {
                c = peek(l);
                if (!is_special_char(c)) {
                    advance(l);
                    keyword[i++] = c;
                } else {
                    break;
                }
            }

            if (strcmp(keyword, "not") == 0) {
                return (Token_t){.kind = TOKEN_NOT};
            } else if (strcmp(keyword, "and") == 0) {
                return (Token_t){.kind = TOKEN_AND};
            } else if (strcmp(keyword, "or") == 0) {
                return (Token_t){.kind = TOKEN_OR};
            } else {
                return (Token_t){.kind = TOKEN_UNK, .string = strdup(keyword)};
            }
        }
        }
    }
    UNREACHABLE("next_token");
}

void dump_token(Token_t t)
{
    switch(t.kind) {
    case TOKEN_TAG: {
        assert(t.string != NULL && "Tag's name is somehow invalid");
        nob_log(NOB_INFO, "type: Tag, name: %s", t.string);
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
    case TOKEN_UNK: {
        assert(t.string != NULL && "Unknown keyword with invalid string, how swell");
        nob_log(NOB_INFO, "type: Unknown, string: %s", t.string);
        break;
    }
    default:
        break;
    }
}

void clean_lexer(Lexer **l)
{
    free((*l)->src);
    free(*l);
    *l = NULL;
}

