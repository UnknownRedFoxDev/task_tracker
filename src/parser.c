#include "../lib/parser.h"
#include <stdlib.h>

const char *node_kind_to_cstr(Node_Kind kind)
{
    switch(kind) {
        case NODE_TAG:      return "TAG";
        case NODE_INT:      return "INT";
        case NODE_AND:      return "AND";
        case NODE_OR:       return "OR";
        case NODE_NOT:      return "NOT";
        case NODE_PRIORITY: return "PRIORITY";
        case NODE_LT:       return "LT";
        case NODE_GT:       return "GT";
        case NODE_LTE:      return "LTE";
        case NODE_GTE:      return "GTE";
    }
    UNREACHABLE("Node_Kind");
}

const char *token_kind_to_cstr(Token_Kind kind)
{
    switch(kind) {
        case TOKEN_UNK:      return "UNK";
        case TOKEN_DOT:      return "DOT";
        case TOKEN_TAG:      return "TAG";
        case TOKEN_INT:      return "INT";
        case TOKEN_AND:      return "AND";
        case TOKEN_OR:       return "OR";
        case TOKEN_NOT:      return "NOT";
        case TOKEN_LPAREN:   return "LPAREN";
        case TOKEN_RPAREN:   return "RPAREN";
        case TOKEN_LT:       return "LT";
        case TOKEN_GT:       return "GT";
        case TOKEN_LTE:      return "LTE";
        case TOKEN_GTE:      return "GTE";
        case TOKEN_PRIORITY: return "PRIORITY";
        case TOKEN_EOF:      return "EOF";
    }
    UNREACHABLE("Token_Kind");
}

Parser *init_parser(Lexer *l)
{
    Parser *s = calloc(1, sizeof(Parser));
    if (s == NULL) {
        nob_log(NOB_ERROR, "failed to allocate space for a parser");
        exit(1);
    }

    s->l = l;
    advance_token(s);
    return s;
}

void clean_parser(Parser **s)
{
    if (*s) {
        clean_lexer(&(*s)->l);
    }
    free((*s));
    *s = NULL;
}

Node_t *create_tag_node(const char *name)
{
    Node_t *node = malloc(sizeof(Node_t));
    if (!node) {
        nob_log(ERROR, "Failed to allocated a tag node");
        exit(1);
    }
    memset(node, 0, sizeof(Node_t));
    node->as.tag_name = strdup(name);
    node->kind = NODE_TAG;
    return node;
}

Node_t *create_int_node(long integer)
{
    Node_t *node = malloc(sizeof(Node_t));
    if (!node) {
        nob_log(ERROR, "Failed to allocated an int node");
        exit(1);
    }
    memset(node, 0, sizeof(Node_t));
    node->as.integer = integer;
    node->kind = NODE_INT;
    return node;
}

Node_t *__create_op_node(Node_Kind kind, Node_t *lhs, Node_t *rhs)
{
    Node_t *node = malloc(sizeof(Node_t));
    if (!node) {
        nob_log(ERROR, "Failed to allocated an op node");
        exit(1);
    }
    memset(node, 0, sizeof(Node_t));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node_t *create_and_node(Node_t *lhs, Node_t *rhs)
{
    return __create_op_node(NODE_AND, lhs, rhs);
}

Node_t *create_or_node(Node_t *lhs, Node_t *rhs)
{
    return __create_op_node(NODE_OR, lhs, rhs);
}

Node_t *create_not_node(Node_t *child)
{
    return __create_op_node(NODE_NOT, child, NULL);
}

Node_t *create_compare_node(Node_t *priority_node, Node_t *integer_node, Node_Kind kind)
{
    return __create_op_node(kind, priority_node, integer_node);
}

void advance_token(Parser *s)
{
    if (s->prev.kind != TOKEN_INT) {
        free(s->prev.as.string);
    }
    s->prev = s->curr;
    s->curr = next_token(s->l);
}

bool match_token_and_advance(Parser *s, Token_Kind expected_kind)
{
    if (s->curr.kind == expected_kind) {
        advance_token(s);
        return true;
    }
    return false;
}

void consume_token(Parser *s, Token_Kind expected_kind)
{
    if (s->curr.kind == expected_kind) {
        advance_token(s);
    }
}

Node_t *parse_query(Parser *s)
{
    Node_t *root = parse_expr(s);
    if (!match_token_and_advance(s, TOKEN_EOF)) { // Consume EOF
        printf("[WARNING] parse_query(): last token is not an EOF; token: %s", token_kind_to_cstr(s->curr.kind));
        if (s->curr.as.string != NULL) {
            printf(", string: |%s|", s->curr.as.string);
        }
        printf("\n");
    }
    return root;
}

Node_t *parse_expr(Parser *s)
{
    Node_t *lhs = parse_and(s);

    while (match_token_and_advance(s, TOKEN_OR)) {
        Node_t *rhs = parse_and(s);
        lhs = create_or_node(lhs, rhs);
    }

    return lhs;
}

Node_t *parse_and(Parser *s)
{
    Node_t *lhs = parse_compare(s);

    while (match_token_and_advance(s, TOKEN_AND)) {
        Node_t *rhs = parse_compare(s);
        lhs = create_and_node(lhs, rhs);
    }

    return lhs;
}

Node_t *parse_compare(Parser *s)
{
    Node_t *lhs = parse_not(s);

    if (match_token_and_advance(s, TOKEN_LT)) {
        Node_t *rhs = parse_elem(s);
        return lhs = create_compare_node(lhs, rhs, NODE_LT);
    }

    if (match_token_and_advance(s, TOKEN_LTE)) {
        Node_t *rhs = parse_elem(s);
        return lhs = create_compare_node(lhs, rhs, NODE_LTE);
    }

    if (match_token_and_advance(s, TOKEN_GT)) {
        Node_t *rhs = parse_elem(s);
        return lhs = create_compare_node(lhs, rhs, NODE_GT);
    }

    if (match_token_and_advance(s, TOKEN_GTE)) {
        Node_t *rhs = parse_elem(s);
        return lhs = create_compare_node(lhs, rhs, NODE_GTE);
    }

    return lhs;
}

Node_t *parse_not(Parser *s)
{
    if (match_token_and_advance(s, TOKEN_NOT)) {
        Node_t *node = parse_not(s);
        return create_not_node(node);
    }

    return parse_elem(s);
}

Node_t *parse_elem(Parser *s)
{
    if (match_token_and_advance(s, TOKEN_TAG)) {
        char *name = s->prev.as.string;
        if (strcmp(name, "all") == 0) {
            Node_t *open_node = create_tag_node("OPEN");
            Node_t *closed_node = create_tag_node("CLOSED");
            return create_or_node(open_node, closed_node);
        }
        return create_tag_node(name);
    }

    if (match_token_and_advance(s, TOKEN_PRIORITY)) {
        return __create_op_node(NODE_PRIORITY, NULL, NULL);
    }

    if (match_token_and_advance(s, TOKEN_INT)) {
        return create_int_node(s->prev.as.integer);
    }

    if (match_token_and_advance(s, TOKEN_LPAREN)) {
        Node_t *sub_nodes = parse_expr(s);
        if (!match_token_and_advance(s, TOKEN_RPAREN)) { // Consume the ')' left
            printf("[WARNING] parse_query(): last token is not an RPAREN; token: %s", token_kind_to_cstr(s->curr.kind));
            if (s->curr.as.string != NULL) {
                printf(", string: |%s|", s->curr.as.string);
            }
            printf("\n");
        }
        return sub_nodes;
    }

    report_query_error(s->l->src, s->l->cursor - s->l->curr_word_size + 1, "Either a tag or a '(' character were expected. Supplied: \"%s\" (type: %s)", s->curr.as.string, token_kind_to_cstr(s->curr.kind));
    abort();
}

void __dump_ast(Node_t *node, size_t level)
{
    switch(node->kind) {
        case NODE_PRIORITY: {
            for (size_t i = 0; i < level; ++i) {
                printf(" ");
            }
            printf("STRING: \"Priority\"\n");
            break;
        }
        case NODE_INT: {
            for (size_t i = 0; i < level; ++i) {
                printf(" ");
            }
            printf("INT: %ld\n", node->as.integer);
            break;
        }
        case NODE_TAG: {
            for (size_t i = 0; i < level; ++i) {
                printf(" ");
            }
            printf("TAG: %s\n", node->as.tag_name);
            break;
        }
        case NODE_NOT: {
            assert(node->lhs != NULL && "A not-node's LHS should not be NULL");
            for (size_t i = 0; i < level; ++i) {
                printf(" ");
            }
            printf("NEGATED: \n");
            __dump_ast(node->lhs, level + 2);
            break;
        }
        case NODE_AND: {
            assert(node->lhs != NULL && "An and-node's LHS should not be NULL");
            assert(node->rhs != NULL && "An and-node's RHS should not be NULL");
            for (size_t i = 0; i < level; ++i) {
                printf(" ");
            }
            printf("BOOLEAN: AND\n");
            for (size_t i = 0; i < level + 2; ++i) {
                printf(" ");
            }
            printf("LHS: \n");
            __dump_ast(node->lhs, level+4);
            for (size_t i = 0; i < level + 2; ++i) {
                printf(" ");
            }
            printf("RHS: \n");
            __dump_ast(node->rhs, level+4);
            break;
        }
        case NODE_OR: {
            assert(node->lhs != NULL && "An or-node's LHS should not be NULL");
            assert(node->rhs != NULL && "An or-node's RHS should not be NULL");
            for (size_t i = 0; i < level; ++i) {
                printf(" ");
            }
            printf("BOOLEAN: OR\n");
            for (size_t i = 0; i < level + 2; ++i) {
                printf(" ");
            }
            printf("LHS: \n");
            __dump_ast(node->lhs, level+4);
            for (size_t i = 0; i < level + 2; ++i) {
                printf(" ");
            }
            printf("RHS: \n");
            __dump_ast(node->rhs, level+4);
            break;
        }
        case NODE_LT: {
            assert(node->lhs != NULL && "An compare-node's LHS should not be NULL");
            assert(node->rhs != NULL && "An compare-node's RHS should not be NULL");
            for (size_t i = 0; i < level; ++i) {
                printf(" ");
            }
            printf("COMPARE: Less Than\n");
            for (size_t i = 0; i < level + 2; ++i) {
                printf(" ");
            }
            printf("KEYWORD: \n");
            __dump_ast(node->lhs, level+4);

            for (size_t i = 0; i < level + 2; ++i) {
                printf(" ");
            }
            printf("NUMBER: \n");
            __dump_ast(node->rhs, level+4);
            break;
        }
        case NODE_LTE: {
            assert(node->lhs != NULL && "An compare-node's LHS should not be NULL");
            assert(node->rhs != NULL && "An compare-node's RHS should not be NULL");
            for (size_t i = 0; i < level; ++i) {
                printf(" ");
            }
            printf("COMPARE: Less Than Or Equal\n");
            for (size_t i = 0; i < level + 2; ++i) {
                printf(" ");
            }
            printf("KEYWORD: \n");
            __dump_ast(node->lhs, level+4);

            for (size_t i = 0; i < level + 2; ++i) {
                printf(" ");
            }
            printf("NUMBER: \n");
            __dump_ast(node->rhs, level+4);
            break;
        }
        case NODE_GT: {
            assert(node->lhs != NULL && "An compare-node's LHS should not be NULL");
            assert(node->rhs != NULL && "An compare-node's RHS should not be NULL");
            for (size_t i = 0; i < level; ++i) {
                printf(" ");
            }
            printf("COMPARE: Greater Than\n");
            for (size_t i = 0; i < level + 2; ++i) {
                printf(" ");
            }
            printf("KEYWORD: \n");
            __dump_ast(node->lhs, level+4);

            for (size_t i = 0; i < level + 2; ++i) {
                printf(" ");
            }
            printf("NUMBER: \n");
            __dump_ast(node->rhs, level+4);
            break;
        }
        case NODE_GTE: {
            assert(node->lhs != NULL && "An compare-node's LHS should not be NULL");
            assert(node->rhs != NULL && "An compare-node's RHS should not be NULL");
            for (size_t i = 0; i < level; ++i) {
                printf(" ");
            }
            printf("COMPARE: Greater Than Or Equal\n");
            for (size_t i = 0; i < level + 2; ++i) {
                printf(" ");
            }
            printf("KEYWORD: \n");
            __dump_ast(node->lhs, level+4);

            for (size_t i = 0; i < level + 2; ++i) {
                printf(" ");
            }
            printf("NUMBER: \n");
            __dump_ast(node->rhs, level+4);
            break;
        }
        default:
            UNREACHABLE("Node_Type");
    }
}

void clean_ast(Node_t *node)
{
    // For not-node handling, their rhs will be NULL
    if (!node) return;

    if (node->kind == NODE_TAG) {
        free(node->as.tag_name);
        node->as.tag_name = NULL;
        free(node);
    } else if (node->kind == NODE_INT) {
        free(node);
    } else {
        clean_ast(node->lhs);
        clean_ast(node->rhs);
        free(node);
    }
}

