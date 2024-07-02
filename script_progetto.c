#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* AGGIUNGERE DESCRIZIONE PER UNDO E REDO */

#define N 1024

/* array dinamico che contiene gli indici di riga di line_mem */
int *text;

/* lista che contiene ogni comando fatto dal programma */
struct cmdlist{
    char cmd; /* comando eseguito */
    int textstart; /* indice della linea di inizio del comando */
    int textend; /* indice della linea di fine del comando */
    int numd;
    int numc;
    int len;
    int *memadd; /* array dinamico che segna l'indice delle righe che vengono aggiunte con la change */
    int *memdel; /* array dinamico che segna cio che viene eliminato sia con la change che con la delete */
};

/* struttura dati per il contenimento del testo dato in input (1 nodo = 1 riga di testo). rappresenta la fonte su cui si basano text e cmdlist per ottenere le linee interessate.*/
char **line_mem;

/* pila (array dinamico) per contenere i comandi da svolgere con undo */
struct cmdlist *undostack;

/* variabili globali del programma */
int stringlen = 0;
int textlen = 0;
int memlen = 0;
int undolen = 0;
int undopos = -1;
int undostart = -1;

char *string_input();
void change (int start, int end);
void add (int index, char *string, int op, int numd);
void delete (int start, int end);
void print (int start, int end);
void uradd (char command, int num);
void listadd (char command, int start, int end, int numc, int numd, int textlen);
void generalur (int num);
void undo (int num);
void undochange (int index);
void undodelete (int index);
void redo (int num);
void redochange (int index);
void redodelete (int index);
void flush ();

int main () {
    int b, c, i, j, k, pot;
    char *d;
    int start, end;
    char command;
    int count;

    count = 0;
    text = calloc(1000000, sizeof(int));

    /* do-while per il parse del comando dato in input e chiamata della funzione corrispondente */
    do{
        d = string_input();
        if (*(d + 0) == 'q') {
            command = *(d + 0);
        }else {
            i = 0;
            c = *(d + i);
            while (c >= '0' && c <= '9') {
                i ++;
                c = *(d + i);
            }
            pot = 1;
            start = 0;
            for (j = (i - 1); j >= 0; j --) {
                b = *(d + j) - '0';
                start = start + (b * pot);
                pot = pot * 10;
            }
            if (c == ',') {
                k = i + 1;
                c = *(d + k);
                while (c >= '0' && c <= '9') {
                    k ++;
                    c = *(d + k);
                }
                command = c;
                pot = 1;
                end = 0;
                for (j = (k - 1); j > i;  j --) {
                    b = *(d + j) - '0';
                    end = end + (b * pot);
                    pot = pot * 10;
                }
            }else{
                command = c;
            }
        }

        count ++;
        /* printf("%d %c\n", count, command); */

        switch (command) {
            case 'c':   change(start, end);
                        break;
            case 'd':   delete(start, end);
                        break;
            case 'p':   print(start, end);
                        break;
            case 'u':   uradd(command, start);
                        break;
            case 'r':   uradd(command, start);
                        break;
            case 'q':   break;
            default:    break;
        }
    }while(command != 'q');

    return 0;
}

/* funzione che riceve l'input in modo dinamico generando una stringa tagliata correttamente */
char *string_input(){
    char *string, c;
    char temp[1024];
    int stringlen;

    if(fgets(temp, N, stdin)){};

    stringlen = strlen(temp);

    string = malloc((stringlen + 1) * sizeof(char));
    /* string = calloc((stringlen + 1), sizeof(char)); */
    strncpy(string, temp, stringlen);
    *(string + stringlen) = '\0';
    
    return string;
}

void change (int start, int end) {
    char *string, *dot;
    int i, numc, op, numd, arg;
    int defstart, defend;
    char command;

    numc = end - start + 1;
    defstart = start - 1;
    defend = end - 1;
    command = 'c';
    op = 0;

    if (undopos != undostart) {
        arg = undopos - undostart;
        generalur(arg);
    }

    if (end >= textlen) {
        numd = textlen - start + 1;
    }else {
        numd = end - start + 1;
    }

    flush();

    if (line_mem == NULL) {
        line_mem = calloc(numc, sizeof(char *));
    }else {
        line_mem = realloc(line_mem, (memlen + numc) * sizeof(char *));
    }

    listadd(command, defstart, defend, numc, numd, textlen);

    if (textlen == 0) {
        /* text = calloc(numc, sizeof(int)); */
        textlen = numc;
    }else {
        if (end > textlen) {
            /* text = realloc(text, end * sizeof(int)); */
            textlen = end;
        }
    }

    for (i = defstart;  i <= defend; i ++) {
        string = string_input();
        add(i, string, op, numd);
        op ++;
    }

    undostack[undolen - 1].len = textlen;

    dot = string_input();
}

void add (int index, char *string, int op, int numd) {
    line_mem[memlen] = malloc(stringlen * sizeof(char));
    line_mem[memlen] = string;

    if (undostack[undolen - 1].memdel != NULL) {
        if (op < numd) {
            undostack[undolen - 1].memdel[op] = text[index];
        }
    }
    text[index] = memlen;
    undostack[undolen - 1].memadd[op] = memlen;

    memlen += 1;
}

void delete (int start, int end) {
    int defstart, defend;
    int i, numd, arg;
    char command;

    defstart = start - 1;
    defend = end - 1;
    command = 'd';

    if (undopos != undostart) {
        arg = undopos - undostart;
        generalur(arg);
    }

    flush();

    if (start == 0) {
        if (end > 0) {
            defstart = 0;
        }else {
            defend = -2;
        }
    }

    if (end >= textlen) {
        if (start > textlen) {
            defstart = -1;
            defend = -2;
        }else {
            defend = textlen - 1;
        }
    }

    numd = defend - defstart + 1;
    listadd(command, defstart, defend, 0, numd, textlen);

    if (defstart >= 0 && defend >= 0) {
        if (end < textlen) {
            for (i = 0; i < textlen; i ++) {
                if (i >= 0 && i < numd) {
                    undostack[undolen - 1].memdel[i] = text[defstart + i];
                }
                if ((defend + 1 + i) < textlen) {
                    text[defstart + i] = text[(defend + 1) + i];
                }
            }
        }else {
            for (i = 0; i < numd; i ++) {
                undostack[undolen - 1].memdel[i] = text[defstart + i];
            }
        }

        textlen = textlen - numd;
        if (textlen == 0) {
            /* free(text); */
        }else {
            /* text = realloc(text, textlen * sizeof(int)); */
        }
    }

    undostack[undolen - 1].len = textlen;
}

/* funzione che printa le righe richieste con controllo del caso 0,*p e del numero di righe printate */
void print (int start, int end) {
    int m, t, i;
    int index, arg;
    int defstart, defend;
    char dot = '.';
    m = end - start + 1;
    t = 0;

    defstart = start - 1;
    defend = end - 1;

    if (undopos != undostart) {
        arg = undopos - undostart;
        generalur(arg);
    }

    if (defend >= textlen) {
        defend = textlen - 1;
    }

    if (defstart == -1) {
        fputc(dot, stdout);
        fputc('\n', stdout);
        defstart = 0;
        t = 1;
    }

    if (defend != -1) {
        for (i = defstart; i <= defend; i++) {
            index = *(text + i);
            fputs(line_mem[index], stdout);
            /* fputc('\n', stdout); */
            t += 1;
        }
    }
    
    if (t != m) {
        for (i = t + 1; i <= m; i ++) {
            fputc(dot, stdout);
            fputc('\n', stdout);
        }
    }
}

void uradd (char command, int num) {
    int defnum;

    if (command == 'u') {
        defnum = -num;
    }else if (command == 'r') {
        defnum = num;
    }

    if (defnum < 0) {
        if ((undopos + defnum) < -1) {
            defnum = - undopos - 1;
        }
    }else if (defnum > 0) {
        if ((undopos + defnum) >= undolen) {
            defnum = undolen - undopos - 1;
        }
    }

    undopos = undopos + defnum;
}

void listadd (char command, int start, int end, int numc, int numd, int textlen) {
    if (undolen == 0) {
        undostack = malloc(1 * sizeof(struct cmdlist));
        undolen = 1;
        undopos = (undolen - 1);
    }else {
        undolen += 1;
        undopos = (undolen - 1);
        undostack = realloc(undostack, undolen * sizeof(struct cmdlist));
    }

    undostart = undopos;

    undostack[undolen - 1].cmd = command;
    undostack[undolen - 1].textstart = start;
    undostack[undolen - 1].textend = end;
    undostack[undolen - 1].numd = numd;
    undostack[undolen - 1].numc = numc;

    if (numc != 0) {
        undostack[undolen - 1].memadd = calloc(numc, sizeof(int));
    }
    if (numd != 0) {
        undostack[undolen - 1].memdel = calloc(numd, sizeof(int));
    }
}

void generalur (int num) {
    int arg;

    if (num < 0) {
        arg = -num;
        undo(arg);
    }else if (num > 0) {
        arg = num;
        redo(arg);
    }
}

void undo (int num) {
    int i;

    if ((undostart - num) == -1) {
        textlen = 0;
    }else if (undolen > undopos) {
        for (i = undostart; i > undopos; i --) {
            if (undostack[i].cmd == 'c') {
                undodelete(i);
            }else if (undostack[i].cmd == 'd') {
                if (undostack[i].textstart != -1 && undostack[i].textend != -2) {
                    undochange(i);
                }
            }
        }
    }
    
    undostart = undopos;
}

void undochange (int index) {
    int len, op, i;

    len = undostack[index].textend - undostack[index].textstart + 1;
    textlen = undostack[index - 1].len;

    for (i = textlen - 1; i > undostack[index].textstart; i --) {
        if ((i - len) >= undostack[index].textstart) {
            text[i] = text[i - len];
        }else {
            text[i] = 0;
        }
    }
    
    op = 0;

    for (i = undostack[index].textstart; i < (undostack[index].textend + 1); i ++) {
        text[i] = undostack[index].memdel[op];
        op ++;
    }
}

void undodelete (int index) {
    int len, i, num;

    len = undostack[index].textend - undostack[index].textstart + 1;
    num = textlen - undostack[index].textstart;

    for (i = 0; i < len; i ++) {
        if (undostack[index].memdel != NULL && i < undostack[index].numd) {
            text[undostack[index].textstart + i] = undostack[index].memdel[i];
        }
    }
    
    textlen = undostack[index - 1].len;
}

void redo (int num) {
    int i;

    undostart ++;

    for (i = undostart; i <= undopos; i ++) {
        if (undostack[i].cmd == 'c') {
            redochange(i);
        }else if (undostack[i].cmd == 'd') {
            if (undostack[i].textstart != -1 && undostack[i].textend != -2) {
                redodelete(i);
            }
        }
    }

    undostart = undopos;
}

void redochange (int index) {
    int i, op;

    op = 0;

    for (i = undostack[index].textstart; i <= undostack[index].textend; i ++) {
        text[i] = undostack[index].memadd[op];
        op ++;
    }

    if (undostack[index].textend >= textlen) {
        textlen = textlen + (undostack[index].textend - (textlen - 1));
    }
}

void redodelete (int index) {
    int i, op;

    op = 0;

    if (undostack[index].textstart >= 0 && undostack[index].textend >= 0) {
        if (undostack[index].textend + 1 < textlen) {
            for (i = 0; i < textlen; i ++) {
                text[undostack[index].textstart + i] = text[(undostack[index].textend + 1) + i];
                op ++;
            }
        }
    }
    
    textlen = textlen - (undostack[index].textend - undostack[index].textstart + 1);
}

void flush () {
    if (undopos == -1) {
        if (undolen != 0) {
            free(undostack);
            undolen = 0;
        }
    }else if (undopos < (undolen - 1)) {
        undostack = realloc(undostack, (undopos + 1) * sizeof(struct cmdlist));
        undolen = undopos + 1;
    }
}
