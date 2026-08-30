#ifndef LEXER_H_
#define LEXER_H_

#include "commons.h"
#define TAG_NAME_SIZE 1024
#define KEYWORD_MAX_SIZE 4

typedef enum {
    TOKEN_UNK,
    TOKEN_DOT,    // .
    TOKEN_TAG,    // <tag>
    TOKEN_AND,    // and
    TOKEN_OR,     // or
    TOKEN_NOT,    // not
    TOKEN_LPAREN, // (
    TOKEN_RPAREN, // )
    TOKEN_EOF,
} Token_Kind;

typedef struct {
    Token_Kind kind;
    char *string;
} Token_t;

typedef struct {
    char *src;
    size_t curr_word_size;
    size_t cursor;
} Lexer;

// ------ Lexer
Lexer *init_lexer(const char *query);
void clean_lexer(Lexer **l);
void report_query_error(const char *src, int cursor, const char *format, ...) NOB_PRINTF_FORMAT(3, 4);

// Character parsing
bool is_special_char(char c);
char peek(Lexer *l);
void advance(Lexer *l);
char consume(Lexer *l);

// Token retrieving
Token_t next_token(Lexer *l);
void dump_token(Token_t t);
// ---------------------

#endif // LEXER_H_
