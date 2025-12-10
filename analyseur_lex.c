#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum {
    PROGRAM = 1,
    BEGIN, 
    END, 
    VAR, 
    INTEGER, 
    CHAR, 
    IF, 
    THEN, 
    ELSE, 
    WHILE, 
    DO,
    READ, 
    READLN, 
    WRITE, 
    WRITELN, 
    ID, 
    NB, 
    OPREL, 
    OPADD, 
    OPMUL,
    PV, 
    DP, 
    V, 
    PERIODE, 
    LPAR, 
    RPAR, 
    AFF, 
    COMM_OUV, 
    COMM_FER, 
    EOF_TOKEN, 
    ERROR
} TokenType;

typedef struct {
    TokenType type;
    char lexeme[100];
} Token;

/* keywords */
const char *keywords[] = {
    "program", "begin", "end", "var", "integer", "char", "if", "then", "else",
    "while", "do", "read", "readln", "write", "writeln"
};
const TokenType keywordTypes[] = {
    PROGRAM, BEGIN, END, VAR, INTEGER, CHAR, IF, THEN, ELSE,
    WHILE, DO, READ, READLN, WRITE, WRITELN
};
const int KEYWORDS_COUNT = 15;

int isKeyword(const char *word, TokenType *type) {
    for (int i = 0; i < KEYWORDS_COUNT; ++i) {
        if (strcmp(word, keywords[i]) == 0) {
            *type = keywordTypes[i];
            return 1;
        }
    }
    return 0;
}

const char *tokenTypeToString(TokenType t) {
    switch (t) {
        case PROGRAM: return "PROGRAM";
        case BEGIN: return "BEGIN";
        case END: return "END";
        case VAR: return "VAR";
        case INTEGER: return "INTEGER";
        case CHAR: return "CHAR";
        case IF: return "IF";
        case THEN: return "THEN";
        case ELSE: return "ELSE";
        case WHILE: return "WHILE";
        case DO: return "DO";
        case READ: return "READ";
        case READLN: return "READLN";
        case WRITE: return "WRITE";
        case WRITELN: return "WRITELN";
        case ID: return "ID";
        case NB: return "NB";
        case OPREL: return "OPREL";
        case OPADD: return "OPADD";
        case OPMUL: return "OPMUL";
        case PV: return "PV";
        case DP: return "DP";
        case V: return "V";
        case PERIODE: return "PERIODE";
        case LPAR: return "LPAR";
        case RPAR: return "RPAR";
        case AFF: return "AFF";
        case COMM_OUV: return "COMM_OUV";
        case COMM_FER: return "COMM_FER";
        case EOF_TOKEN: return "EOF";
        case ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

Token getNextToken(FILE *file) {
    Token token;
    token.type = ERROR;
    token.lexeme[0] = '\0';

    int c;
    int pos = 0;

    /* skip whitespace */
    do {
        c = fgetc(file);
    } while (c == ' ' || c == '\t' || c == '\n' || c == '\r');

    if (c == EOF) {
        token.type = EOF_TOKEN;
        strcpy(token.lexeme, "EOF");
        return token;
    }

    /* store first char tentatively */
    token.lexeme[pos++] = (char)c;
    token.lexeme[pos] = '\0';

    /* comment or left parenthesis */
    if (c == '(') {
        int d = fgetc(file);
        if (d == '*') { 
            while (1) {
                int x = fgetc(file);
                if (x == EOF) {
                    token.type = ERROR;
                    strcpy(token.lexeme, "Commentaire non fermé");
                    return token;
                }
                if (x == '*') {
                    int y = fgetc(file);
                    if (y == ')') {
                        return getNextToken(file);
                    } else {
                        if (y != EOF) ungetc(y, file);
                    }
                }
            }
        } else {
            if (d != EOF) ungetc(d, file);
            token.type = LPAR;
            token.lexeme[0] = '(';
            token.lexeme[1] = '\0';
            return token;
        }
    }

    /* relational operators */
    if (c == '<') {
        int d = fgetc(file);
        if (d == '=') { 
            token.type = OPREL; 
            strcpy(token.lexeme, "<="); 
            return token; 
        }
        if (d == '>') { 
            token.type = OPREL; 
            strcpy(token.lexeme, "<>"); 
            return token; 
        }
        if (d != EOF) ungetc(d, file);
        token.type = OPREL; 
        token.lexeme[0] = '<'; 
        token.lexeme[1] = '\0'; 
        return token;
    }
    if (c == '>') {
        int d = fgetc(file);
        if (d == '=') { 
            token.type = OPREL; 
            strcpy(token.lexeme, ">="); 
            return token; 
        }
        if (d != EOF) ungetc(d, file);
        token.type = OPREL; 
        token.lexeme[0] = '>'; 
        token.lexeme[1] = '\0'; 
        return token;
    }
    if (c == '=') {
        token.type = OPREL; 
        token.lexeme[0] = '='; 
        token.lexeme[1] = '\0'; 
        return token;
    }

    /* assignment or colon */
    if (c == ':') {
        int d = fgetc(file);
        if (d == '=') { 
            token.type = AFF; 
            strcpy(token.lexeme, ":="); 
            return token; }
        if (d != EOF) ungetc(d, file);
        token.type = DP; 
        token.lexeme[0] = ':'; 
        token.lexeme[1] = '\0'; 
        return token;
    }

    /* additive operators + - || */
    if (c == '+' || c == '-') {
        token.type = OPADD;
        token.lexeme[0] = (char)c;
        token.lexeme[1] = '\0';
        return token;
    }
    if (c == '|') {
        int d = fgetc(file);
        if (d == '|') { 
            token.type = OPADD; 
            strcpy(token.lexeme, "||"); 
            return token; 
        }

        if (d != EOF) ungetc(d, file);
        token.type = ERROR;
        sprintf(token.lexeme, "Caractère invalide: %c", '|');
        return token;
    }

    /* multiplicative operators */
    if (c == '*' || c == '/' || c == '%') {
        token.type = OPMUL;
        token.lexeme[0] = (char)c;
        token.lexeme[1] = '\0';
        return token;
    }

    /* punctuation */
    if (c == ';') {
        token.type = PV;
        token.lexeme[0] = ';';
        token.lexeme[1] = '\0';
        return token; 
    }
    if (c == ',') { 
        token.type = V;
        token.lexeme[0] = ','; 
        token.lexeme[1] = '\0'; 
        return token; 
    }
    if (c == '.') { 
        token.type = PERIODE;  
        token.lexeme[0] = '.'; 
        token.lexeme[1] = '\0'; 
        return token; 
    }
    if (c == ')') { 
        token.type = RPAR; 
        token.lexeme[0] = ')'; 
        token.lexeme[1] = '\0'; 
        return token; 
    }

    /* identifiers and keywords */
    if (isalpha(c)) {
        pos = 0;
        token.lexeme[pos++] = (char)c;
        while (1) {
            int d = fgetc(file);
            if (d == EOF) break;
            if (!isalnum(d)) { ungetc(d, file); break; }
            if (pos < (int)sizeof(token.lexeme) - 1) token.lexeme[pos++] = (char)d;
        }
        token.lexeme[pos] = '\0';

        /* lowercase copy for keyword check */
        char lower[100];
        for (int i = 0; token.lexeme[i] && i < (int)sizeof(lower)-1; ++i) lower[i] = (char)tolower((unsigned char)token.lexeme[i]);
        lower[strlen(token.lexeme)] = '\0';

        TokenType kwType;
        if (isKeyword(lower, &kwType)) token.type = kwType;
        else token.type = ID;
        return token;
    }

    /* numbers */
    if (isdigit(c)) {
        pos = 0;
        token.lexeme[pos++] = (char)c;
        while (1) {
            int d = fgetc(file);
            if (d == EOF) break;
            if (!isdigit(d)) { ungetc(d, file); break; }
            if (pos < (int)sizeof(token.lexeme) - 1) token.lexeme[pos++] = (char)d;
        }
        token.lexeme[pos] = '\0';
        token.type = NB;
        return token;
    }

    /* unknown char */
    token.type = ERROR;
    sprintf(token.lexeme, "Caractère invalide: %c", (char)c);
    return token;
}

int main(void) {
    FILE *input = fopen("program.txt", "r");
    FILE *output = fopen("tokens.txt", "w");

    if (!input || !output) {
        perror("Erreur ouverture fichier");
        return 1;
    }

    Token t;
    do {
        t = getNextToken(input);
        fprintf(output, "Token: %s (Code: %d)\n",
                t.lexeme, (int)t.type);
    } while (t.type != EOF_TOKEN);

    fclose(input);
    fclose(output);
    return 0;
}
