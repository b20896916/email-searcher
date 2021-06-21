#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N_TOKEN_HASH 73176001

typedef struct token{
    char* content;
    int len;
} token;

int trans(char c);
unsigned int hash(token* c, int base);
int cmpfunc(const void* a, const void* b) {return (*(unsigned int*) a - *(unsigned int*) b);}


int appeared[N_TOKEN_HASH] = {0};

int main(){
    FILE* tok;
    tok = fopen("tokens", "r");
    token tokens[138078];
    for (int i = 0; i < 138078; i++) tokens[i].content = (char*) malloc(sizeof(char) * 52);
    int collision[1000];
    int col = 0;

    for (int i = 0; i < 138078; i++){
        fscanf(tok, "%s", tokens[i].content);
        tokens[i].len = strlen(tokens[i].content);
        unsigned int h = hash(tokens+i, N_TOKEN_HASH);
        if (appeared[h]){
            collision[col++] = i;
        }
        appeared[h]++;
    }

    printf("%d\n", col);

    int rehash_base = 2;
    while (rehash_base < N_TOKEN_HASH){
        int T[600];
        int tlen = 0;
        for (int i = 0; i < col; i++){
            unsigned int h = hash(tokens+collision[i], rehash_base);// + hash(tokens+collision[i], N_TOKEN_HASH);
            if (!appeared[h]){
                T[tlen++] = h;
            }
            else break;
        }
        if (tlen == 506){
            qsort(T, tlen, sizeof(unsigned int), cmpfunc);
            int collisionagain = 0;
            for (int i = 0; i < tlen-1; i++){
                if (T[i] == T[i+1]){
                    collisionagain = 1;
                    break;
                }
            }
            if (!collisionagain){
                printf("%d\n", rehash_base);
            }
        }
        rehash_base++;
    }

    return 0;
}

unsigned int hash(token* c, int base){
    unsigned int r = 0;
    for (int i = 0; i < c->len; i++){
        r *= 37;
        r += trans(c->content[i]);
        r %= base;
    }
    return r;
}

int trans(char c){
    if ((c >= '0') && (c <= '9')) return c-'0';
    if ((c >= 'a') && (c <= 'z')) return c-'a'+10;
    if ((c >= 'A') && (c <= 'Z')) return c-'A'+10;
}