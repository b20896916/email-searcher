#include "api.h"
#pragma GCC optimize("O3", "unroll-loops")
#pragma GCC target("avx", "avx2", "fma")
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct _trie{
    struct _trie *p[36];
    int end;
} trie;

void insert(trie *o, char *s, int n, int k); // insert a string with length n and beginning at s to a trie
int trie_search(trie *o, char *s, int n); // search whether a string with length n and beginning at s in a trie

void insert(trie *o, char *s, int n, int k) {
    if (o == NULL) {
        o=malloc(sizeof(trie));
        for (int i=0; i<36; i++) {
            o->p[i] = NULL;
        }
        o->end = -1;
    }
    if (n == 0) {
        o->end = k;
        return;
    }
    insert(o->p[transform(*s)], s+1, n-1);
}

int trie_search(trie *o, char *s, int n) {
    if (n == 0) {
        return o->end;
    }
    if (o == NULL) {
        return -1;
    }
    return trie_search(o->p[transform(*s)], s+1, n-1);
}
