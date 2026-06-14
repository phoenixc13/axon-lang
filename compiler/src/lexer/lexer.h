#ifndef AXON_LEXER_H
#define AXON_LEXER_H

/**
 * axon-lang/compiler/src/lexer/lexer.h
 * AXON Lexer -- Public API
 *
 * APSE Group -- Engineering Directorate
 * License: Proprietary
 */

#include <stddef.h>
#include <stdint.h>

/* ------------------------------------------------------------------ */
/* Token Kinds                                                          */
/* ------------------------------------------------------------------ */

typedef enum {
    /* Graph primitives */
    TK_NODE, TK_EDGE, TK_CONCEPT, TK_MESSAGE,

    /* Keywords */
    TK_FN, TK_LET, TK_CONST, TK_USE,
    TK_DOMAIN, TK_LINKS, TK_DIVERGE, TK_BRANCH,
    TK_SANDBOX, TK_PUB, TK_PRIV, TK_RETURN,
    TK_IF, TK_ELSE, TK_LOOP, TK_BREAK,
    TK_MATCH, TK_AS, TK_TRUE, TK_FALSE,

    /* Literals */
    TK_IDENT,       /* identifier */
    TK_INT,         /* integer literal */
    TK_FLOAT,       /* float literal */
    TK_STRING,      /* string literal */
    TK_ATTRIBUTE,   /* #[...] attribute */

    /* Punctuation */
    TK_LBRACE,      /* { */
    TK_RBRACE,      /* } */
    TK_LPAREN,      /* ( */
    TK_RPAREN,      /* ) */
    TK_LBRACKET,    /* [ */
    TK_RBRACKET,    /* ] */
    TK_COLON,       /* : */
    TK_COLONCOLON,  /* :: */
    TK_SEMICOLON,   /* ; */
    TK_COMMA,       /* , */
    TK_DOT,         /* . */
    TK_EQ,          /* = */
    TK_EQEQ,        /* == */
    TK_NEQ,         /* != */
    TK_LT,          /* < */
    TK_LTE,         /* <= */
    TK_GT,          /* > */
    TK_GTE,         /* >= */
    TK_ARROW,       /* -> */
    TK_PLUS,        /* + */
    TK_MINUS,       /* - */
    TK_STAR,        /* * */
    TK_SLASH,       /* / */
    TK_BANG,        /* ! */

    /* Control */
    TK_EOF,
    TK_ERROR
} TokenKind;

/* ------------------------------------------------------------------ */
/* Token -- zero-copy string view into source buffer                   */
/* ------------------------------------------------------------------ */

typedef struct {
    TokenKind    kind;
    const char  *start;   /* pointer into source buffer (NOT null-terminated) */
    uint32_t     length;  /* byte length of token text */
    uint32_t     line;
    uint32_t     col;
} Token;

/* ------------------------------------------------------------------ */
/* Lexer state                                                          */
/* ------------------------------------------------------------------ */

typedef struct {
    const char  *src;       /* source buffer (not owned) */
    size_t       len;       /* source buffer length in bytes */
    size_t       pos;       /* current scan position */
    uint32_t     line;
    uint32_t     col;
    const char  *filename;  /* for diagnostics */
} Lexer;

/* ------------------------------------------------------------------ */
/* API                                                                  */
/* ------------------------------------------------------------------ */

/**
 * Initialize a Lexer over a source buffer.
 * The buffer must remain valid for the lifetime of the Lexer.
 * No memory is allocated.
 */
void  axon_lexer_init(Lexer *lex, const char *src, size_t len,
                      const char *filename);

/**
 * Return the next Token from the source stream.
 * Tokens point directly into the source buffer (zero-copy).
 * Returns TK_EOF once the stream is exhausted.
 */
Token axon_next_token(Lexer *lex);

/**
 * Helper: returns a static C-string name for a TokenKind.
 * Useful for diagnostics and debug printing.
 */
const char *axon_token_kind_name(TokenKind kind);

#endif /* AXON_LEXER_H */
