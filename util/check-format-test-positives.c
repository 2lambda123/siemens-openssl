/*
 * Copyright 2007-2019 The OpenSSL Project Authors. All Rights Reserved.
 * Copyright Nokia 2007-2019
 * Copyright Siemens AG 2015-2019
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

/*
 * This demonstrates/tests cases where check-format.pl should report issues.
 * Some of the reports are due to sanity checks for proper nesting of comment
 * delimiters and parenthesis-like symbols, e.g., on unexpected/unclosed braces.
 */

/*
 * The '@'s after '*' are used for self-tests: they mark lines containing
 * a single flaw that should be reported. Normally it should be reported
 * while handling the given line, but in case of delayed checks there is a
 * following digit indicating the number of reports expected for this line.
 */

/* For each of the following set of lines the tool should complain once */
/*@ tab character: 	 */
/*@ intra-line carriage return character:  */
/*@ non-printable ASCII character:  */
/*@ non-ASCII character: ä */
/*@ whitespace at EOL: */ 
// /*@ end-of-line comment style not allowed for C90 */
/*X */ /*@ no space after comment start, reported unless sloppy-spc */
/* X*/ /*@ no space before comment end , reported unless sloppy-spc */
/*@ comment starting delimiter: /* inside intra-line comment */
 /*@ normal comment indent off by 1, reported unless sloppy-cmt */
 /*@
  *@ above multi-line comment start indent off by 1, reported unless sloppy-cmt; this comment line is too long
   *@ multi-line comment further off by 1 relative to comment start
*/ /*@ multi-line comment end indent off by -2 relative to comment start */
/*@ multi-line comment starting with text on first line
 *@ comment starting delimiter: /* inside multi-line comment
*@ multi-line comment indent off by -1
 *X*@ no spc after leading'*' in multi-line comment, reported unless sloppy-spc
 *@0 more than double space after '.'   in comment, reported unless sloppy-spc
 *@2 multi-line comment ending with text on last line */
*/ /*@ unexpected comment end symbol outside comment */
/*@ comment line is 4 columns tooooooooooooooooo wide, reported unless sloppy-len */
/*@ comment line is 5 columns toooooooooooooooooooooooooooooooooooooooooooooo wide */
#define X   1       /*@0 dbl space false negative due to coincidental match */
 #define Y  2       /*@ indent of preprocessor directive off by 1 (must be 0) */
typedef struct  {   /*@0 double space, reported unless sloppy-spc */
    enum {          /*@2 double space  in comment, reported unless sloppy-spc */
           w = 0   /*@ hanging expr indent off by 1, or 3 for lines after '{' */
             && 1,  /*@ hanging expr indent off by 3, or -1 for leading '&&' */
         x = 1,     /*@ hanging expr indent off by -1 */
          y,z       /*@ no space after ',', reported unless sloppy-spc */
    } e_member ;    /*@ space before ';', reported unless sloppy-spc */
    int v[1;        /*@ unclosed bracket in type declaration */
   union {          /*@ statement/type declaration indent off by -1 */
        struct{} s; /*@ no space before '{', reported unless sloppy-spc */
    }u_member;      /*@ no space after '}', reported unless sloppy-spc */
    } s_type;       /*@ statement/type declaration indent off by 4 */
int* somefunc();    /*@ no space before '*' in type declaration */
void main(int n) {  /*@ opening brace at end of function definition header */
    for (;;n++) {   /*@ no space after ';', reported unless sloppy-spc */
        return;     /*@0 (1-line) single statement in braces */
    }}              /*@2 code after '}' outside expr */
}                   /*@ unexpected closing brace (too many '}') outside expr */
)                   /*@ unexpected closing paren outside expr */
#endif              /*@ unexpected #endif */
int f (int a,       /*@ space after fn before '(', reported unless sloppy-spc */
      int b)        /*@ hanging expr indent off by -1 */
{ int               /*@ code after '{' opening a block */
    xx = 1) +       /*@ unexpected closing parenthesis */
        2] -        /*@ unexpected closing bracket */
        3: *        /*@ unexpected ':' (without preceding '?') within expr */
        4};         /*@ unexpected closing brace within expression */
    char y[] = {    /*@0 unclosed brace within initializer/enum expression */
        1* 1,       /*@ no space or name, '(', ')', '*' after '*' */
         2,         /*@ hanging expr indent (for lines after '{') off by 1 */
        (xx         /*@0 unclosed parenthesis in expression */
         ? y        /*@0 unclosed '? (conditional expression) */
         [0;        /*@4 unclosed bracket in expression */
   s_type s;        /*@ local variable declaration indent off by -1 */
   somefunc(a,      /*@ statement indent off by -1 */
          "aligned" /*@ expr indent off by -2 accepted if sloppy-hang */ "right"
           , b,     /*@ expr indent off by -1 */
           b,       /*@ expr indent again off -1, accepted if sloppy-hang */
        b,  /*@ expr indent off -4 but @ extra indent accepted if sloppy-hang */
   "again aligned" /*@ expr indent off by -9 (left of stmt indent, */ "right",
            123 == /*@ .. so reported also with sloppy-hang; this line is too long */ 456
# define MAC(A) (A) /*@ nesting indent of preprocessor directive off by 1 */
             ? 1    /*@ hanging expr indent off by 1 */
              : 2); /*@ hanging expr indent off by 2, or 1 for leading ':' */
    if(a            /*@ no space after 'if', reported unless sloppy-spc */
       && ! 0       /*@ space after '!', reported unless sloppy-spc */
         || b ==    /*@ hanging expr indent off by 2, or -2 for leading '||' */
       (xx+= 2) +   /*@ no space before '+=', reported unless sloppy-spc */
       a ++ -       /*@ space before postfix '++', reported unless sloppy-spc */
       (a^ 1) +     /*@ no space before '^', reported unless sloppy-spc */
       a %2 /       /*@ no space after '%', reported unless sloppy-spc */
       1 +/* */     /*@ no space before comment, reported unless sloppy-spc */
       /* */+       /*@ no space after comment, reported unless sloppy-spc */
       s. e_member) /*@ space after '.', reported unless sloppy-spc */
         xx = a + b /*@ extra single-statement indent off by 1 */
               + 0; /*@ double extra single-statement indent off by 3 */
    if (1) f(a,     /*@ (non-brace) code after end of 'if' condition */
             b); else /*@ (non-brace) code before 'else' */
        do f(c, c); /*@ (non-brace) code after 'do' */
        while ( 2); /*@ space after '(', reported unless sloppy-spc */
    b; c;           /*@ more than one statement per line */
    do{             /*@ no space before '{', reported unless sloppy-spc */
        f (3,       /*@ space after fn before '(', reported unless sloppy-spc */
           4);      /*@0 false negative: should report single stmt in braces */
    } while (a+ 0); /*@ no space before '+', reported unless sloppy-spc */
    switch (b ) {   /*@ space before ')', reported unless sloppy-spc */
   case 1:          /*@ 'case' special statement indent off by -1 */
    case(2):        /*@ no space after 'case' */
     default: ;     /*@ 'default' special statement indent off by 1 */
}                   /*@ statement indent off by -4 */
  label:            /*@ label special statement indent off by 1 */
    return(         /*@ no space after 'return' with expression */
           x); }    /*@ block-level '}' has code before */
/* Here the tool should stop complaining apart from the below issues at EOF */

#if 0               /*@0 unclosed #if */
struct t {          /*@0 unclosed brace at decl/block level */
    enum {          /*@0 unclosed brace at enum/expression level */
          v = (1    /*@0 unclosed parenthesis */
               etyp /*@0 empty line follows just before EOF: */

