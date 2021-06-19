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

int n_mails, n_queries;
mail *mails;
query *queries;
mymail* mymails = (mymail*) malloc(sizeof(mymail) * 10000);

int cmpfunc(const void* a, const void* b);
bool alpha_numeric(char c);
// return true iff c in /([A-Za-z0-9]+)/g
int transform(char c);
// map '0'-'9' to 0-9, 'a'-'z' and 'A'-'Z' to 10~35
void token_hash (char content[100000], lld hash_array[100000], int *len);
// hash content to hash_array, save token_num to len, need modifying to reduce complexity
int name_hash(char name[32]);
// not implemented

int main(){
    for (int i = 0; i < 10000; i++){
        mymails[i].token = (lld*) malloc(sizeof(lld) * 100000);
        mymails[i].token_num = 0;
    }

    api.init(&n_mails, &n_queries, &mails, &queries);

    for (int i = 0; i < n_mails; i++){
        int idx = mails[i].id;
        mymails[idx].from = name_hash(mails[i].from);
        mymails[idx].to = name_hash(mails[i].to);
        token_hash(mails[i].subject, mymails[idx].token, &mymails[idx].token_num);
        token_hash(mails[i].content, mymails[idx].token, &mymails[idx].token_num);
        qsort(mymails[idx].token, mymails[idx].token_num, sizeof(lld), cmpfunc);
    }

    for(int i = 0; i < n_queries; i++)
        if (queries[i].type == expression_match){
            // expression match
            api.answer(queries[i].id, NULL, 0);
        }
        else if (queries[i].type == find_similar){
            // find similar
            api.answer(queries[i].id, NULL, 0);
        }
        else{
            // group analyse
            api.answer(queries[i].id, NULL, 0);
        }
    return 0;
}

int cmpfunc(const void* a, const void* b){
    return (*(lld*) a - *(lld*) b);
}

bool alpha_numeric(char c){
    if ((c >= '0') && (c <= '9')) return true;
    if ((c >= 'a') && (c <= 'z')) return true;
    if ((c >= 'A') && (c <= 'Z')) return true;
    return false;
}

int transform(char c){
    if ((c >= '0') && (c <= '9')) return c-'0';
    if ((c >= 'a') && (c <= 'z')) return c-'a'+10;
    if ((c >= 'A') && (c <= 'Z')) return c-'A'+10;
}

void token_hash (char content[100000], lld hash_array[100000], int *len){
    lld hash = 0;
    for (int i = 0; i < 100000; i++){
        if (alpha_numeric(content[i])){
            hash *= 36;
            hash = (hash + transform(content[i])) % 2021060687879487;
        }
        else{
            bool repeated = false;
            for (int j = 0; j < *len; j++){
                if (hash == hash_array[j]){
                    repeated = true;
                    break;
                }
            }
            if ((!repeated) && (hash != 0)){
                hash_array[*len] = hash;
                *len = *len + 1;
            }
            hash = 0;
            if (content[i] == '\0') break;
        }
    }
}