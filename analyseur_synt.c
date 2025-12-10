#include <stdio.h>
#include <stdlib.h>
#include <string.h>


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
    PERIOD,    
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

Token currentToken;


#define SYMTAB_SIZE 200

typedef struct {
    char name[100];
    TokenType type; 
    int declared;   
    int address;    
} Symbol;

static Symbol symtab[SYMTAB_SIZE];
static int symtab_count = 0;
static int next_address = 0;

static char last_declared[100][100];
static int last_declared_count = 0;
static TokenType current_decl_type = ERROR;

#define CODE_SIZE 1000
typedef struct {
    char instruction[100];
} ligne_code;

static ligne_code code[CODE_SIZE];
static int code_index = 0;
static int label_counter = 0;

/* generer code instruction */
void generer(const char *instruction) {
    if (code_index >= CODE_SIZE) {
        fprintf(stderr, "Code memory overflow\n");
        exit(1);
    }
    strcpy(code[code_index].instruction, instruction);
    code_index++;
}


int nouvelle_etiquette() {
    return label_counter++;
}

void afficher_code() {
    printf("\n--- Code pour automate a pile  ---\n");
    for (int i = 0; i < code_index; i++) {
        printf("%3d: %s\n", i, code[i].instruction);
    }
}

void write_symtab_to_file(const char *filename) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Error opening file %s for writing\n", filename);
        return;
    }
    
    for (int i = 0; i < symtab_count; ++i) {
        if (symtab[i].type == INTEGER)
            fprintf(file, "%3d: %-12s  type=INTEGER  declared=%d  address=%d\n", 
                    i, symtab[i].name, symtab[i].declared, symtab[i].address);
        else if (symtab[i].type == CHAR)
            fprintf(file, "%3d: %-12s  type=CHAR     declared=%d  address=%d\n", 
                    i, symtab[i].name, symtab[i].declared, symtab[i].address);
    }
    fprintf(file, "\n");
    fclose(file);
}

void write_code_to_file(const char *filename) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Error opening file %s for writing\n", filename);
        return;
    }

    for (int i = 0; i < code_index; i++) {
        fprintf(file, "%3d: %s\n", i, code[i].instruction);
    }
    fclose(file);
}

int symtab_get_index(const char *name) {
    for (int i = 0; i < symtab_count; ++i) {
        if (strcmp(symtab[i].name, name) == 0) return i;
    }
    return -1;
}

int symtab_add(const char *name) {
    int idx = symtab_get_index(name);
    if (idx != -1) return idx;
    if (symtab_count >= SYMTAB_SIZE) {
        fprintf(stderr, "Symbol table overflow\n");
        exit(1);
    }
    strcpy(symtab[symtab_count].name, name);
    symtab[symtab_count].type = ERROR;
    symtab[symtab_count].declared = 0;
    symtab[symtab_count].address = -1;
    return symtab_count++;
}

void symtab_set_type_by_name(const char *name, TokenType type) {
    int idx = symtab_get_index(name);
    if (idx == -1) idx = symtab_add(name);
    symtab[idx].type = type;
    symtab[idx].declared = 1;
    symtab[idx].address = next_address++;
}

void symtab_print(void) {
    printf("\n--- Symbol table (%d entries) ---\n", symtab_count);
    for (int i = 0; i < symtab_count; ++i) {
        if(symtab[i].type == INTEGER)
            printf("%3d: %-12s  type=INTEGER  declared=%d  address=%d\n", 
                   i, symtab[i].name, symtab[i].declared, symtab[i].address);
        else if(symtab[i].type == CHAR)
            printf("%3d: %-12s  type=CHAR     declared=%d  address=%d\n", 
                   i, symtab[i].name, symtab[i].declared, symtab[i].address);
    }
}

Token getNextToken(FILE *file) {
    Token token;
    int code;
    char lexeme[100];
    
    if (fscanf(file, "Token: %s (Code: %d)\n", lexeme, &code) != 2) {
        token.type = EOF_TOKEN;
        strcpy(token.lexeme, "EOF");
    } else {
        token.type = (TokenType)code;
        strcpy(token.lexeme, lexeme);
    }
    
    printf("L'analyseur lit: %s (Type: %d)\n", token.lexeme, token.type);
    return token;
}

void match(TokenType expected, FILE *file) {
    if (currentToken.type == expected) {
        currentToken = getNextToken(file);
    } else {
        fprintf(stderr, "Error: Expected token type %d but got %d ('%s')\n", 
                expected, currentToken.type, currentToken.lexeme);
        exit(1);
    }
}

void P(FILE *file);
void DCL(FILE *file);
void D(FILE *file);
void list_id(FILE *file);
void L(FILE *file);
void type(FILE *file);
void Inst_composée(FILE *file);
void Inst(FILE *file);
void list_Inst(FILE *file);
void L_I(FILE *file);
void I(FILE *file);
void express(FILE *file);
void S(FILE *file);
void Terme(FILE *file);
void T(FILE *file);
void F(FILE *file);
void Facteur(FILE *file);
void Exp_simple(FILE *file);

// P -> program ID ; DCL Inst_composée .
void P(FILE *file) {
    match(PROGRAM, file);
    match(ID, file);
    match(PV, file);
    DCL(file);
    Inst_composée(file);
    match(PERIOD, file);
    generer("Halte");
}

// DCL -> VAR D | epsilon
void DCL(FILE *file) {
    if (currentToken.type == VAR) {
        match(VAR, file);
        D(file);
    }
}

// D -> list_id : type ; D | epsilon
void D(FILE *file) {
    if (currentToken.type == ID) {
        list_id(file);
        match(DP, file);  
        type(file);
        match(PV, file); 
        for (int i = 0; i < last_declared_count; ++i) {
            symtab_set_type_by_name(last_declared[i], current_decl_type);
        }
        last_declared_count = 0;
        current_decl_type = ERROR;
        D(file); 
    }
}

// list_id -> ID L
void list_id(FILE *file) {
    if (currentToken.type == ID) {
        char saved[100];
        strcpy(saved, currentToken.lexeme);
        symtab_add(saved);
        if (last_declared_count < 100) {
            strcpy(last_declared[last_declared_count++], saved);
        }
        match(ID, file);
        L(file);
    } else {
        fprintf(stderr, "Error in list_id(): Expected ID but got %d ('%s')\n", 
                currentToken.type, currentToken.lexeme);
        exit(1);
    }
}

// L -> , ID L | epsilon
void L(FILE *file) {
    if (currentToken.type == V) { 
        match(V, file);
        if (currentToken.type == ID) {
            char saved[100];
            strcpy(saved, currentToken.lexeme);
            symtab_add(saved);
            if (last_declared_count < 100) {
                strcpy(last_declared[last_declared_count++], saved);
            }
            match(ID, file);
            L(file);
        } else {
            fprintf(stderr, "Error in L(): Expected ID after ',' but got %d ('%s')\n", 
                    currentToken.type, currentToken.lexeme);
            exit(1);
        }
    }
}

// type -> integer | char
void type(FILE *file) {
    if (currentToken.type == INTEGER) {
        current_decl_type = INTEGER;
        match(INTEGER, file);
    } else if (currentToken.type == CHAR) {
        current_decl_type = CHAR;
        match(CHAR, file);
    } else {
        fprintf(stderr, "Error: Expected type (integer or char)\n");
        exit(1);
    }
}

// Inst_composée -> begin Inst end
void Inst_composée(FILE *file) {
    match(BEGIN, file);
    Inst(file);
    match(END, file);
}

// Inst -> list_Inst | epsilon
void Inst(FILE *file) {
    if (currentToken.type == ID || currentToken.type == IF || currentToken.type == WHILE ||
        currentToken.type == READ || currentToken.type == WRITE || currentToken.type == READLN ||
        currentToken.type == WRITELN || currentToken.type == BEGIN) {
        list_Inst(file);
    }
}

// list_Inst -> I L_I
void list_Inst(FILE *file) {
    I(file);
    L_I(file);
}

// L_I -> ; I L_I | epsilon
void L_I(FILE *file) {
    while (currentToken.type == PV) {
        match(PV, file);
        if (currentToken.type == ID || currentToken.type == IF || currentToken.type == WHILE ||
            currentToken.type == READ || currentToken.type == WRITE || currentToken.type == READLN ||
            currentToken.type == WRITELN || currentToken.type == BEGIN) {
            I(file);
        } else {
            break;
        }
    }
}

// I -> ID := Exp_simple | if express then I else I | 
//      while express do I | read(ID) | write(ID) | 
//      readln(ID) | writeln(ID) | Inst_composée
void I(FILE *file) {
    char buffer[200];
    int idx;
    
    switch (currentToken.type) {
        case ID:
            idx = symtab_get_index(currentToken.lexeme);
            if (idx == -1 || symtab[idx].declared == 0) {
                fprintf(stderr, "Error: Undeclared identifier '%s'\n", currentToken.lexeme);
                exit(1);
            }
            sprintf(buffer, "Valeurg %d", symtab[idx].address);
            generer(buffer);
            match(ID, file);
            match(AFF, file);
            Exp_simple(file);
            generer(":=");
            break;
        
        case IF: {
            int etiq_else = nouvelle_etiquette();
            int etiq_fin = nouvelle_etiquette();
            
            match(IF, file);
            express(file);
            sprintf(buffer, "Aller-si-faux Etiq_%d", etiq_else);
            generer(buffer);
            match(THEN, file);
            I(file);
            sprintf(buffer, "Aller à Etiq_%d", etiq_fin);
            generer(buffer);
            sprintf(buffer, "Etiq Etiq_%d", etiq_else);
            generer(buffer);
            if (currentToken.type == ELSE) {
                match(ELSE, file);
                I(file);
            }
            sprintf(buffer, "Etiq Etiq_%d", etiq_fin);
            generer(buffer);
            break;
        }
        
        case WHILE: {
            int etiq_debut = nouvelle_etiquette();
            int etiq_fin = nouvelle_etiquette();
            
            sprintf(buffer, "Etiq Etiq_%d", etiq_debut);
            generer(buffer);
            match(WHILE, file);
            express(file);
            sprintf(buffer, "Aller-si-faux Etiq_%d", etiq_fin);
            generer(buffer);
            match(DO, file);
            I(file);
            sprintf(buffer, "Aller à Etiq_%d", etiq_debut);
            generer(buffer);
            sprintf(buffer, "Etiq Etiq_%d", etiq_fin);
            generer(buffer);
            break;
        }
        
        case READ:
        case READLN:
            match(currentToken.type, file);
            match(LPAR, file);
            idx = symtab_get_index(currentToken.lexeme);
            if (idx == -1 || symtab[idx].declared == 0) {
                fprintf(stderr, "Error: Undeclared identifier '%s'\n", currentToken.lexeme);
                exit(1);
            }
            sprintf(buffer, "Valeurg %d", symtab[idx].address);
            generer(buffer);
            generer("Lire");
            generer(":=");
            match(ID, file);
            match(RPAR, file);
            break;
        
        case WRITE:
        case WRITELN:
            match(currentToken.type, file);
            match(LPAR, file);
            idx = symtab_get_index(currentToken.lexeme);
            if (idx == -1 || symtab[idx].declared == 0) {
                fprintf(stderr, "Error: Undeclared identifier '%s'\n", currentToken.lexeme);
                exit(1);
            }
            sprintf(buffer, "Valeurd %d", symtab[idx].address);
            generer(buffer);
            generer("Ecrire");
            match(ID, file);
            match(RPAR, file);
            break;
        
        case BEGIN:
            Inst_composée(file);
            break;
        
        default:
            fprintf(stderr, "Error in I(): Unexpected token %d ('%s')\n", 
                    currentToken.type, currentToken.lexeme);
            exit(1);
    }
}

// express -> Exp_simple S
void express(FILE *file) {
    Exp_simple(file);
    S(file);
}

// S -> OPREL Exp_simple | epsilon
void S(FILE *file) {
    if (currentToken.type == OPREL) {
        char op[100];
        strcpy(op, currentToken.lexeme);
        match(OPREL, file);
        Exp_simple(file);
        
        if (strcmp(op, ">") == 0) {
            generer("Comparer-si-sup");
        } else if (strcmp(op, "<") == 0) {
            generer("Comparer-si-inf");
        } else if (strcmp(op, "=") == 0 || strcmp(op, "==") == 0) {
            generer("Comparer-si-égal");
        } else if (strcmp(op, ">=") == 0) {
            generer("Comparer-si-inf");
            generer("Empiler 0");
            generer("Comparer-si-égal");
        } else if (strcmp(op, "<=") == 0) {
            generer("Comparer-si-sup");
            generer("Empiler 0");
            generer("Comparer-si-égal");
        } else if (strcmp(op, "<>") == 0 || strcmp(op, "!=") == 0) {
            generer("Comparer-si-égal");
            generer("Empiler 0");
            generer("Comparer-si-égal");
        }
    }
}

// Exp_simple -> Terme T
void Exp_simple(FILE *file) {
    Terme(file);
    T(file);
}

// T -> OPADD Terme T | epsilon
void T(FILE *file) {
    if (currentToken.type == OPADD) {
        char op[100];
        strcpy(op, currentToken.lexeme);
        match(OPADD, file);
        Terme(file);
        
        if (strcmp(op, "+") == 0) {
            generer("+");
        } else if (strcmp(op, "-") == 0) {
            generer("-");
        }
        
        T(file);
    }
}

// Terme -> Facteur F
void Terme(FILE *file) {
    Facteur(file);
    F(file);
}

// F -> OPMUL Facteur F | epsilon
void F(FILE *file) {
    if (currentToken.type == OPMUL) {
        char op[100];
        strcpy(op, currentToken.lexeme);
        match(OPMUL, file);
        Facteur(file);
        
        if (strcmp(op, "*") == 0) {
            generer("*");
        } else if (strcmp(op, "/") == 0) {
            generer("/");
        }
        
        F(file);
    }
}

// Facteur -> ID | NB | (Exp_simple)
void Facteur(FILE *file) {
    char buffer[200];
    
    switch (currentToken.type) {
        case ID: {
            int idx = symtab_get_index(currentToken.lexeme);
            if (idx == -1 || symtab[idx].declared == 0) {
                fprintf(stderr, "Error: Undeclared identifier '%s'\n", currentToken.lexeme);
                exit(1);
            }
            sprintf(buffer, "Valeurd %d", symtab[idx].address);
            generer(buffer);
            match(ID, file);
            break;
        }
        
        case NB:
            sprintf(buffer, "Empiler %s", currentToken.lexeme);
            generer(buffer);
            match(NB, file);
            break;
        
        case LPAR:
            match(LPAR, file);
            Exp_simple(file);
            match(RPAR, file);
            break;
        
        default:
            fprintf(stderr, "Error in Facteur(): Unexpected token %d ('%s')\n", 
                    currentToken.type, currentToken.lexeme);
            exit(1);
    }
}

int main() {
    FILE *tokenFile = fopen("tokens.txt", "r");
    if (tokenFile == NULL) {
        perror("Error opening tokens.txt file");
        return 1;
    }
    
    currentToken = getNextToken(tokenFile);
    P(tokenFile);
    
    symtab_print();
    afficher_code();

    write_symtab_to_file("symbol_table.txt");
    write_code_to_file("pile_code.txt");
    
    fclose(tokenFile);

    return 0;
}