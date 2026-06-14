/**
 * axon-lang/compiler/src/lexer/lexer.c
 * AXON Language Lexer
 *
 * Tokenizes AXON source text into a flat token stream.
 * Zero heap allocation per token -- tokens point directly into
 * the source buffer (zero-copy string views).
 *
 * APSE Group -- Engineering Directorate
 * License: Proprietary
 */

#include "lexer.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>

/* ------------------------------------------------------------------ */
/* Keyword table                                                        */
/* ------------------------------------------------------------------ */

static const struct { const char *word; TokenKind kind; } kKeywords[] = {
    { "node",    TK_NODE    },
    { "edge",    TK_EDGE    },
    { "concept", TK_CONCEPT },
    { "message", TK_MESSAGE },
    { "fn",      TK_FN      },
    { "let",     TK_LET     },
    { "const",   TK_CONST   },
    { "use",     TK_USE     },
    { "domain",  TK_DOMAIN  },
    { "links",   TK_LINKS   },
    { "diverge", TK_DIVERGE },
    { "branch",  TK_BRANCH  },
    { "sandbox", TK_SANDBOX },
    { "pub",     TK_PUB     },
    { "priv",    TK_PRIV    },
    { "return",  TK_RETURN  },
    { "if",      TK_IF      },
    { "else",    TK_ELSE    },
    { "loop",    TK_LOOP    },
    { "break",   TK_BREAK   },
    { "match",   TK_MATCH   },
    { "as",      TK_AS      },
    { "true",    TK_TRUE    },
    { "false",   TK_FALSE   },
    { NULL,      TK_EOF     }
};

/* ------------------------------------------------------------------ */
/* Lexer state                                                          */
/* ------------------------------------------------------------------ */

void axon_lexer_init(Lexer *lex, const char *src, size_t len,
                     const char *filename) {
    lex->src      = src;
    lex->len      = len;
    lex->pos      = 0;
    lex->line     = 1;
    lex->col      = 1;
    lex->filename = filename;
}

static inline char peek(const Lexer *lex) {
    if (lex->pos >= lex->len) return '\0';
    return lex->src[lex->pos];
}

static inline char peek2(const Lexer *lex) {
    if (lex->pos + 1 >= lex->len) return '\0';
    return lex->src[lex->pos + 1];
}

static inline char advance(Lexer *lex) {
    char c = lex->src[lex->pos++];
    if (c == '\n') { lex->line++; lex->col = 1; }
    else           { lex->col++; }
    return c;
}

static void skip_whitespace_and_comments(Lexer *lex) {
    while (lex->pos < lex->len) {
        char c = peek(lex);
        /* Whitespace */
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            advance(lex);
        }
        /* Line comment: // ... */
        else if (c == '/' && peek2(lex) == '/') {
            while (lex->pos < lex->len && peek(lex) != '\n')
                advance(lex);
        }
        /* Block comment: /* ... */ */
        else if (c == '/' && peek2(lex) == '*') {
            advance(lex); advance(lex); /* consume /* */
            while (lex->pos + 1 < lex->len) {
                if (peek(lex) == '*' && peek2(lex) == '/') {
                    advance(lex); advance(lex);
                    break;
                }
                advance(lex);
            }
        }
        else break;
    }
}

/* ------------------------------------------------------------------ */
/* Token constructors                                                   */
/* ------------------------------------------------------------------ */

static Token make_token(Lexer *lex, TokenKind kind,
                        size_t start, uint32_t line, uint32_t col) {
    Token t;
    t.kind   = kind;
    t.start  = lex->src + start;
    t.length = (uint32_t)(lex->pos - start);
    t.line   = line;
    t.col    = col;
    return t;
}

static Token make_single(Lexer *lex, TokenKind kind) {
    uint32_t line = lex->line, col = lex->col;
    size_t   start = lex->pos;
    advance(lex);
    return make_token(lex, kind, start, line, col);
}

/* ------------------------------------------------------------------ */
/* Identifier / keyword scan                                            */
/* ------------------------------------------------------------------ */

static Token scan_identifier(Lexer *lex) {
    uint32_t line = lex->line, col = lex->col;
    size_t   start = lex->pos;

    while (lex->pos < lex->len &&
           (isalnum((unsigned char)peek(lex)) || peek(lex) == '_'))
        advance(lex);

    uint32_t len = (uint32_t)(lex->pos - start);

    /* Keyword lookup */
    for (int i = 0; kKeywords[i].word != NULL; i++) {
        if (strlen(kKeywords[i].word) == len &&
            memcmp(kKeywords[i].word, lex->src + start, len) == 0) {
            return make_token(lex, kKeywords[i].kind, start, line, col);
        }
    }

    return make_token(lex, TK_IDENT, start, line, col);
}

/* ------------------------------------------------------------------ */
/* Number scan                                                          */
/* ------------------------------------------------------------------ */

static Token scan_number(Lexer *lex) {
    uint32_t line = lex->line, col = lex->col;
    size_t   start = lex->pos;
    int      is_float = 0;

    /* Optional leading sign handled by parser */
    while (lex->pos < lex->len && isdigit((unsigned char)peek(lex)))
        advance(lex);

    if (peek(lex) == '.' && isdigit((unsigned char)peek2(lex))) {
        is_float = 1;
        advance(lex); /* consume '.' */
        while (lex->pos < lex->len && isdigit((unsigned char)peek(lex)))
            advance(lex);
    }

    /* Scientific notation: e / E */
    if (peek(lex) == 'e' || peek(lex) == 'E') {
        is_float = 1;
        advance(lex);
        if (peek(lex) == '+' || peek(lex) == '-') advance(lex);
        while (lex->pos < lex->len && isdigit((unsigned char)peek(lex)))
            advance(lex);
    }

    return make_token(lex, is_float ? TK_FLOAT : TK_INT, start, line, col);
}

/* ------------------------------------------------------------------ */
/* Attribute scan: #[...] -> TK_ATTRIBUTE                              */
/* ------------------------------------------------------------------ */

static Token scan_attribute(Lexer *lex) {
    uint32_t line = lex->line, col = lex->col;
    size_t   start = lex->pos;
    advance(lex); /* '#' */
    if (peek(lex) == '[') {
        advance(lex);
        while (lex->pos < lex->len && peek(lex) != ']') advance(lex);
        if (peek(lex) == ']') advance(lex);
    }
    return make_token(lex, TK_ATTRIBUTE, start, line, col);
}

/* ------------------------------------------------------------------ */
/* Main next_token                                                      */
/* ------------------------------------------------------------------ */

Token axon_next_token(Lexer *lex) {
    skip_whitespace_and_comments(lex);

    if (lex->pos >= lex->len) {
        Token t = { TK_EOF, NULL, 0, lex->line, lex->col };
        return t;
    }

    uint32_t line = lex->line, col = lex->col;
    char c = peek(lex);

    if (c == '#') return scan_attribute(lex);
    if (isalpha((unsigned char)c) || c == '_') return scan_identifier(lex);
    if (isdigit((unsigned char)c)) return scan_number(lex);

    switch (c) {
        case '{': return make_single(lex, TK_LBRACE);
        case '}': return make_single(lex, TK_RBRACE);
        case '(': return make_single(lex, TK_LPAREN);
        case ')': return make_single(lex, TK_RPAREN);
        case '[': return make_single(lex, TK_LBRACKET);
        case ']': return make_single(lex, TK_RBRACKET);
        case ':': {
            size_t start = lex->pos;
            advance(lex);
            if (peek(lex) == ':') { advance(lex); return make_token(lex, TK_COLONCOLON, start, line, col); }
            return make_token(lex, TK_COLON, start, line, col);
        }
        case ';': return make_single(lex, TK_SEMICOLON);
        case ',': return make_single(lex, TK_COMMA);
        case '.': return make_single(lex, TK_DOT);
        case '=': {
            size_t start = lex->pos;
            advance(lex);
            if (peek(lex) == '=') { advance(lex); return make_token(lex, TK_EQEQ, start, line, col); }
            return make_token(lex, TK_EQ, start, line, col);
        }
        case '-': {
            size_t start = lex->pos;
            advance(lex);
            if (peek(lex) == '>') { advance(lex); return make_token(lex, TK_ARROW, start, line, col); }
            return make_token(lex, TK_MINUS, start, line, col);
        }
        case '+': return make_single(lex, TK_PLUS);
        case '*': return make_single(lex, TK_STAR);
        case '/': return make_single(lex, TK_SLASH);
        case '!': {
            size_t start = lex->pos;
            advance(lex);
            if (peek(lex) == '=') { advance(lex); return make_token(lex, TK_NEQ, start, line, col); }
            return make_token(lex, TK_BANG, start, line, col);
        }
        case '<': {
            size_t start = lex->pos;
            advance(lex);
            if (peek(lex) == '=') { advance(lex); return make_token(lex, TK_LTE, start, line, col); }
            return make_token(lex, TK_LT, start, line, col);
        }
        case '>': {
            size_t start = lex->pos;
            advance(lex);
            if (peek(lex) == '=') { advance(lex); return make_token(lex, TK_GTE, start, line, col); }
            return make_token(lex, TK_GT, start, line, col);
        }
        case '"': {
            /* String literal */
            uint32_t sline = lex->line, scol = lex->col;
            size_t start = lex->pos;
            advance(lex); /* opening " */
            while (lex->pos < lex->len && peek(lex) != '"') {
                if (peek(lex) == '\\') advance(lex); /* escape */
                advance(lex);
            }
            if (peek(lex) == '"') advance(lex); /* closing " */
            return make_token(lex, TK_STRING, start, sline, scol);
        }
        default: {
            /* Unknown character -- emit error token */
            size_t start = lex->pos;
            advance(lex);
            Token t = make_token(lex, TK_ERROR, start, line, col);
            fprintf(stderr, "[axonc] lexer error at %s:%u:%u -- unexpected character '%c'\n",
                    lex->filename, line, col, c);
            return t;
        }
    }
}
