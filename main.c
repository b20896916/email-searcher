#pragma GCC optimize("O3", "unroll-loops")
#pragma GCC target("avx", "avx2", "fma")
#include "api.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>

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

typedef struct hilset{
    int size;
    lld* content;
    int* space;
} hilset;

typedef struct expr{
    bool contain;
    char word[53];
    struct expr *prev;
    struct expr *next;
    lld hash_value;
}expr;

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
int token_hash(char* content, lld* hashvalue); // content points to the beginning char of token, save hash in hashvalue, return the length of token
int name_hash(char name[32]); // hash name to 0-19814
void makeset(int i);
inline void static init(int i);
int find_set(int i); // path compression
void dslink(int ra, int rb); // union by size
inline void hilinit(); // initializing
void hilreset(); // malloc a space for content (can be given to mymail) and set size to 0
void hiladd(lld x); // add x to hashset

int priority(expr *temp); // to see the priority of expr ('(' ')' > '!' > '|' '&')
expr *alloc_expr(); // to create a new expr
expr *linked_list_one(char expression[]); // transfrom expression into a linked list
expr *linked_list_two(expr *head); // transform linked list into postfix type (like hw1 calculator)
bool contained(char content[], expr *head); // return true iff the content contains the expression
void test_expr(expr *head); // only for debug

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
            expr *head = linked_list_one(queries[i].data.expression_match_data.expression);
            head = linked_list_two(head);
            int ans[n_mails];
            int len = 0;
            for (int j=0 ; j<n_mails ; j++){
                if (contained(mymails[j],head)){
                    ans[len] = j;
                    len ++;
                }
            }
            api.answer(queries[i].id, ans, len);
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

int priority(expr *temp){
    if (temp == NULL) return 5;
    if ((temp -> word[0] == '(') || (temp -> word[0] == ')')) return 4;
    if (temp -> word[0] == '!') return 2;
    if ((temp -> word[0] == '&') || (temp -> word[0] == '|')) return 1;
    return 3;
}

expr *alloc_expr(){
    expr *temp = (expr *)malloc(sizeof(expr));
    for (int i=0;i<30;i++){
        temp -> word[i] = '\0';
    }
    temp -> next = NULL;
    temp -> contain = false;
    return temp;
}

expr *linked_list_one(char expression[]){
    expr *head, *tail, *new;
    int digit = 1;
    unsigned long int len = strlen(expression);
    for (unsigned long int i=0;i<len;i++){
        if (i == 0){
            tail = alloc_expr();
            tail -> word[0] = expression[i];
            head = tail;
            tail -> prev = NULL;
        }else if ((isalnum(expression[i])) && (isalnum(expression[i-1]))){
            tail -> word[digit] = expression[i];
            digit ++;
        }else{
            new = alloc_expr();
            new -> word[0] = expression[i];
            tail -> next = new;
            new -> prev = tail;
            tail = new;
            digit = 1;
        }
    }
//TO DO: complete hash funtion.
    expr *temp = head;
    while (temp != NULL){
        if (isalnum(temp -> word[0])){
            lld hashvalue;
            int len = token_hash(temp -> word, &hashvalue);
            temp -> hash_value = hashvalue;
        }
        temp = temp -> next;
    }
    return head;
}

expr *linked_list_two(expr *head){
    expr *tail, *temp = head;
    expr *prisonhead = NULL, *prisontail = NULL;
    while (temp != NULL){
        expr *new = temp -> next;
        if (isalnum(temp -> word[0])){
            if (temp != head){
                tail -> next = temp;
                temp -> prev = tail;
            }else{
                temp -> prev = NULL;
            }
            temp -> next = NULL;
            tail = temp;
            while (priority(prisontail) <= priority(tail)){
                expr *prev = prisontail -> prev;
                tail -> next = prisontail;
                prisontail -> next = NULL;
                prisontail -> prev = tail;
                tail = prisontail;
                if (prev != NULL) prev -> next = NULL;
                prisontail = prev;
            }
            if (prisontail == NULL) prisonhead = NULL;
        }else{
            if (temp == head){
                head = head -> next;
            }
            if (prisonhead == NULL){
                prisonhead = temp;
                prisontail = temp;
                temp -> prev = NULL;
                temp -> next = NULL;
            }else{
                if (temp -> word[0] == ')'){
                    free(temp);
                    while (tail -> word[0] != '('){
                        expr *prev = prisontail -> prev;
                        if (tail != NULL) tail -> next = prisontail;
                        prisontail -> next = NULL;
                        prisontail -> prev = tail;
                        tail = prisontail;
                        if (prev != NULL) prev -> next = NULL;
                        prisontail = prev;
                    }
                    if (prisontail == NULL){
                        prisonhead = NULL;
                    }
                    int x = 0;
                    do{
                        if (x == 0){
                            expr *previous = tail -> prev;
                            free (tail);
                            tail = previous;
                            if (previous != NULL) previous -> next = NULL;
                            if (priority(prisontail) > priority(tail)){
                                break;
                            }
                        }
                        expr *prev = prisontail -> prev;
                        tail -> next = prisontail;
                        prisontail -> next = NULL;
                        prisontail -> prev = tail;
                        tail = prisontail;
                        if (prev != NULL) prev -> next = NULL;
                        prisontail = prev;
                        x ++;
                    } while (priority(prisontail) <= priority(tail));
                    if (prisontail == NULL) prisonhead = NULL;
                }else{
                    prisontail -> next = temp;
                    temp -> prev = prisontail;
                    temp -> next = NULL;
                    prisontail = temp;
                }
            }
        }
        temp = new;
    }
    while (prisontail != NULL){
        expr *prev = prisontail -> prev;
        tail -> next = prisontail;
        prisontail -> prev = tail;
        prisontail -> next = NULL;
        tail = prisontail;
        prisontail = prev;
    }
    return head;
}

bool contained(mymail content, expr *head){
    // TO DO : HERE : true iff temp1(2)(3) -> hash_value can be found in content -> token.
    expr *temp1 = head;
    temp1 -> contain = true; // HERE
    expr *temp2 = temp1 -> next;
    while ((temp2 != NULL) && (temp2 -> word[0] == '!')){
        temp2 = temp2 -> next;
        temp1 -> next = temp2;
        if (temp2 != NULL) temp2 -> prev = temp1;
        temp1 -> contain = !(temp1 -> contain);
    }
    expr *temp3;
    if (temp2 == NULL){
        temp3 = NULL;
    }else{
        temp3 = temp2 -> next;
        temp2 -> contain = true; // HERE
    }
    while (temp3 != NULL){
        if (isalnum(temp3 -> word[0])){
            temp3 -> contain = true; // HERE
            temp1 = temp1 -> next;
            temp2 = temp2 -> next;
            temp3 = temp3 -> next;
        }else if (temp3 -> word[0] == '!'){
            temp2 -> contain = !(temp2 -> contain);
            temp3 = temp3 -> next;
            free (temp2 -> next);
            temp2 -> next = temp3;
            if (temp3 != NULL) temp3 -> prev = temp2;
        }else{
            if (temp3 -> word[0] == '&'){
                temp2 -> contain = (temp1 -> contain) & (temp2 -> contain);
            }else if (temp3 -> word[0] == '|'){
                temp2 -> contain = (temp2 -> contain) | (temp2 -> contain);
            }
            if (temp1 == head){
                temp1 = temp1 -> next;
                temp2 = temp2 -> next -> next;
                if (temp3 -> next != NULL) temp3 = temp3 -> next -> next;
                else temp3 = NULL;
                free(head);
                head = temp1;
                free(temp1 -> next);
                temp1 -> next = temp2;
                if (temp2 != NULL) temp2 -> prev = temp1;
            }else{
                temp1 = temp1 -> prev;
                temp3 = temp3 -> next;
                free(temp1 -> next);
                temp1 -> next = temp2;
                temp2 -> prev = temp1;
                free(temp3 -> prev);
                temp3 -> prev = temp2;
                temp2 -> next = temp3;
            }
        }
    }
    bool answer = temp1 -> contain;
    while (head != NULL){
        expr *temp = head -> next;
        free(head);
        head = temp;
    }
    return answer;
}

void test_expr(expr *head){
    expr *temp = head;
    while (temp != NULL){
        printf ("%s ",temp -> word);
        temp = temp -> next;
    }
    printf ("\n");
}
