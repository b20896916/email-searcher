#pragma GCC optimize("O3", "unroll-loops")
#pragma GCC target("avx", "avx2", "fma")
#include "api.h"
#include <stdio.h>

// The testdata only contains the first 100 mails (mail1 ~ mail100)
// and 2000 queries for you to debug.

typedef long long int lld;
typedef struct mymail{
    int token_num;
    int from;
    int to;
    lld* token;
} mymail;

typedef struct DisjointSet{
    int p;
    int size;
} disjoint_set;

typedef struct SetWithoutInitializing{
    int size;
    lld* content;
    int* space;
} hilset;

#define N_NAME_HASH 19815
#define N_TOKEN_HASH 1000000000000037
#define N_TOKEN_MAIL 3500
int n_mails, n_queries;
mail *mails;
query *queries;
mymail mymails[10000]; // mails after preprocessing
disjoint_set ds[N_NAME_HASH]; // disjoint set
bool set[N_NAME_HASH];
int n_cc, max_cc; // number of connected component, largest size of connected component
hilset hashset;

int cmpfunc(const void* a, const void* b); // for qsort, type: int
bool isalnum(char c); // return true iff c in /([A-Za-z0-9]+)/g
inline int transform(char c); // map '0'-'9' to 0-9, 'a'-'z' and 'A'-'Z' to 10~35
int token_hash(char* content, lld* hashvalue); // hash content to hash_array, save token_num to len, need modifying to reduce complexity
int name_hash(char name[32]); // hash name to 0-19814
void makeset(int i);
inline void static init(int i);
int find_set(int i); // path compression
void dslink(int ra, int rb); // union by size
void hilinit(); // initializing
void hilreset(); // malloc a space for content (can be given to mymail) and set size to 0
void hiladd(lld x); // add x to a

int main(){
    hilinit();
    
    api.init(&n_mails, &n_queries, &mails, &queries);

    for (int i = 0; i < n_mails; i++){
        hilreset();
        int idx = mails[i].id, shift = 0;
        mymails[idx].from = name_hash(mails[i].from);
        mymails[idx].to = name_hash(mails[i].to);
        while (mails[i].subject[shift]){
            if (isalnum(mails[i].subject[shift])){
                lld hashvalue;
                shift += token_hash(mails[i].subject+shift, &hashvalue);
                hiladd(hashvalue);
            }
            shift++;
        }
        while (mails[i].content[shift]){
            if (isalnum(mails[i].content[shift])){
                lld hashvalue;
                shift += token_hash(mails[i].content+shift, &hashvalue);
                hiladd(hashvalue);
            }
            shift++;
        }
        mymails[idx].token = hashset.content;
        mymails[idx].token_num = hashset.size;
        qsort(mymails[idx].token, mymails[idx].token_num, sizeof(lld), cmpfunc);
    }

    for (int i = 0; i < n_queries; i++) {
        if (queries[i].type == expression_match){
            // expression match
            // api.answer(queries[i].id, NULL, 0);
        }
        else if (queries[i].type == find_similar){
            // find similar
            // api.answer(queries[i].id, NULL, 0);
        }
        else{
            // group analyse
            for (int j = 0; j < N_NAME_HASH; j++) set[j] = 0;
            n_cc = 0; max_cc = 0;

            for (int k = 0; k < queries[i].data.group_analyse_data.len; k++){
                dslink(mymails[queries[i].data.group_analyse_data.mids[k]].from, mymails[queries[i].data.group_analyse_data.mids[k]].to);
            }
            int ans[2] = {n_cc, max_cc};
            api.answer(queries[i].id, ans, 2);
        }
    }
    return 0;
}

int cmpfunc(const void* a, const void* b){
    return (*(lld*) a - *(lld*) b);
}

bool isalnum(char c){
    if ((c >= '0') && (c <= '9')) return true;
    if ((c >= 'a') && (c <= 'z')) return true;
    if ((c >= 'A') && (c <= 'Z')) return true;
    return false;
}

inline int transform(char c){
    if ((c >= '0') && (c <= '9')) return c-'0';
    if ((c >= 'a') && (c <= 'z')) return c-'a'+10;
    if ((c >= 'A') && (c <= 'Z')) return c-'A'+10;
}

int token_hash(char* content, lld* hashvalue){
    static char S[9][4] = {"20", "c0", "170", "kz", "nz", "6z", "140", "k0", "g0"};
    int j;
    for (int i = 0; i < 9; i++){
        bool same = 1;
        for (j = 0; j < 4; j++){
            if ( !( S[i][j] || isalnum(content[j]) ) ) break;
            if (content[j] != S[i][j]){
                same = 0;
                break;
            }
        }
        if (same){
            *hashvalue = i;
            return j;
        }
    }
    j = 0;
    lld sum = 0;
    while (isalnum(content[j])){
        sum *= 37;
        sum += transform(content[j]);
        sum %= N_TOKEN_HASH;
        j++;
    }
    *hashvalue = sum;
    return j;
}

int name_hash(char name[32]){
    int r = 0, i = 0;
    while (name[i]){
        r <<= 7;
        r |= name[i];
        r %= N_NAME_HASH;
        i++;
    }
    return r;
}

void makeset(int i){
    // TODO: Initialize a set
    ds[i].p = i;
    ds[i].size = 1;
    n_cc++;
}

inline void static init(int i){
    if (!set[i]) {
        makeset(i);
        set[i] = 1;
    }
}

int find_set(int i){
    init(i);
    // TODO: Implement your find algorithm here
    // if (ds[i].p != i) ds[i].p = find_set(ds[i].p);
    // return ds[i].p;
    // sorry Brian, I have to make it uglier but faster
    return (ds[i].p == i) ? (i) : (ds[i].p = find_set(ds[i].p));
}

void dslink(int ra, int rb){
    // TODO: Implement your union algorithm here
    int a = find_set(ra), b = find_set(rb);
    if (a != b){
        if (ds[a].size < ds[b].size){
            ds[a].p = b;
            ds[b].size += ds[a].size;
            if (ds[b].size > max_cc) max_cc = ds[b].size;
        }
        else{
            ds[b].p = a;
            ds[a].size += ds[b].size;
            if (ds[a].size > max_cc) max_cc = ds[a].size;
        }
        n_cc--;
    }
}

inline void hilinit(){
    hashset.space = (int*) malloc(sizeof(int) * N_TOKEN_HASH);
}

void hilreset(){
    hashset.content = (lld*) malloc(sizeof(lld) * N_TOKEN_MAIL);
    hashset.size = 0;
}

void hiladd(lld x){
    if (hashset.space[x] < hashset.size && hashset.content[hashset.space[x]] == x) return;

    hashset.content[hashset.size] = x;
    hashset.space[x] = hashset.size++;
}