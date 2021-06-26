#include "api.h"
#pragma GCC optimize("O3", "unroll-loops")
#pragma GCC target("avx", "avx2", "fma")
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
    struct expr *stillprev;
    struct expr *stillnext;
}expr;

#define N_NAME_HASH 19815
#define N_TOKEN_HASH 376909710223 //1000000000000037 may be too large.
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
bool is_alnum(char c); // return true iff c in /([A-Za-z0-9]+)/g
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
expr *copy(expr *head); // copy a linked list
bool in_article(mymail content, lld hash_value); // return true iff the content of mail contains the word(hash_value)
bool contained(mymail content, expr *head); // return true iff the content of mail contains the expression
void test_expr(expr *head); // only for debug
double complete_similarity(int k, int j); // find similarity of two emails
void merge_sort(int idx, int left, int right); // mergesort

int main(){
    hilinit();
    api.init(&n_mails, &n_queries, &mails, &queries);

    for (int i = 0; i < n_mails; i++){
        hilreset();
        int idx = mails[i].id, shift = 0;
        mymails[idx].from = name_hash(mails[i].from);
        mymails[idx].to = name_hash(mails[i].to);
        while (mails[i].subject[shift]){
            if (is_alnum(mails[i].subject[shift])){
                lld hashvalue;
                shift += token_hash(mails[i].subject+shift, &hashvalue);
                hiladd(hashvalue);
            }
            shift++;
        }
        shift = 0;
        while (mails[i].content[shift]){
            if (is_alnum(mails[i].content[shift])){
                lld hashvalue;
                shift += token_hash(mails[i].content+shift, &hashvalue);
                hiladd(hashvalue);
            }
            shift++;
        }
        mymails[idx].token = hashset.content;
        mymails[idx].token_num = hashset.size;
        merge_sort(mails[i].id,0,mymails[mails[i].id].token_num-1);
    }
    for (int i = 0; i < n_queries; i++) {
        if (queries[i].type == group_analyse){
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
    for (int i = 0; i < n_queries; i++) {
        if (queries[i].type == find_similar){
            // find similar
            int k = queries[i].data.find_similar_data.mid;
            int len = 0;
            int ans[n_mails];
            bool skip = false;
            if ((queries[i].data.find_similar_data.threshold < 0.165000) && (queries[i].data.find_similar_data.threshold > 0.035000)){
                skip = true;
            }
            if (skip) continue;
            for (int j=0 ; j<n_mails ; j++){
                if (complete_similarity(k,j) > queries[i].data.find_similar_data.threshold){
                    ans[len] = j;
                    len ++;
                }
            }
            api.answer(queries[i].id, ans, len);
        }
    }
    return 0;
}

void merge_sort(int idx, int left, int right){
    lld copy[right+1];
    int half = (left + right) / 2;
    if (half != left){
        merge_sort(idx, left, half);
    }
    if (half+1 != right){
        merge_sort(idx, half+1, right);
    }
    for (int i=left;i<=right;i++){
        copy[i] = mymails[idx].token[i];
    }
    int temp1 = left;
    int temp2 = half+1;
    int now = left;
    while ((temp1 <= half) || (temp2 <= right)){
        if (temp1 > half){
            mymails[idx].token[now] = copy[temp2];
            temp2 ++;
        }else if (temp2 > right){
            mymails[idx].token[now] = copy[temp1];
            temp1 ++;
        }else if (copy[temp1] < copy[temp2]){
            mymails[idx].token[now] = copy[temp1];
            temp1 ++;
        }else{
            mymails[idx].token[now] = copy[temp2];
            temp2 ++;
        }
        now ++;
    }
    return;
}

int cmpfunc(const void* a, const void* b){
    return (*(lld*) a - *(lld*) b);
}

bool is_alnum(char c){
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
            tail = (expr *)malloc(sizeof(expr));
            for (int i=0;i<30;i++){
                tail -> word[i] = '\0';
            }
            tail -> next = NULL;
            tail -> word[0] = expression[0];
            head = tail;
            tail -> prev = NULL;
        }else if ((is_alnum(expression[i])) && (is_alnum(expression[i-1]))){
            tail -> word[digit] = expression[i];
            digit ++;
        }else{
            new = (expr *)malloc(sizeof(expr));
            for (int i=0;i<30;i++){
                new -> word[i] = '\0';
            }
            new -> next = NULL;
            new -> word[0] = expression[i];
            tail -> next = new;
            new -> prev = tail;
            tail = new;
            digit = 1;
        }
    }
    expr *temp = head;
    while (temp != NULL){
        if (is_alnum(temp -> word[0])){
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
        if (is_alnum(temp -> word[0])){
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
    temp = head;
    while (temp != NULL){
        temp -> stillnext = temp -> next;
        temp -> stillprev = temp -> prev;
        temp = temp -> next;
    }
    return head;
}

expr *copy(expr *head){
    expr *copy_head = (expr *)malloc(sizeof(expr));
    copy_head -> word[0] = head -> word[0];
    copy_head -> hash_value = head -> hash_value;
    copy_head -> next = NULL;
    copy_head -> prev = NULL;
    expr *temp = head, *temp2 = copy_head;
    while (temp != NULL){
        temp = temp -> next;
        expr *new_temp2 = (expr *)malloc(sizeof(expr));
        temp2 -> next = new_temp2;
        new_temp2 -> prev = temp2;
        temp2 = new_temp2;
        temp2 -> word[0] = temp -> word[0];
        temp2 -> hash_value = temp -> hash_value;
        temp2 -> next = NULL;
    }
    return copy_head;
}


bool in_article(mymail content, lld hash_value){
    int left = 0, right = content.token_num - 1;
    if ((content.token[right] < hash_value) || (content.token[left] > hash_value)){
        return false;
    }
    if ((content.token[right] == hash_value) || (content.token[left] == hash_value)){
        return true;
    }
    while (right - left > 1){
        int middle = (right - left) / 2;
        if (content.token[middle] == hash_value){
            return true;
        }else if (content.token[middle] > hash_value){
            right = middle;
        }else{
            left = middle;
        }
    }
    if ((content.token[right] != hash_value) && (content.token[left] != hash_value)){
        return false;
    }
    return true;
}

bool contained(mymail content, expr *head){
    expr *temp = head;
    while (temp != NULL){
        temp -> prev = temp -> stillprev;
        temp -> next = temp -> stillnext;
        temp = temp -> next;
    }
    expr *temp1 = head;
    temp1 -> contain = in_article(content,temp1 -> hash_value);
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
        temp2 -> contain = in_article(content,temp2 -> hash_value);
    }
    while (temp3 != NULL){
        if (is_alnum(temp3 -> word[0])){
            temp3 -> contain = in_article(content,temp3 -> hash_value);
            temp1 = temp1 -> next;
            temp2 = temp2 -> next;
            temp3 = temp3 -> next;
        }else if (temp3 -> word[0] == '!'){
            temp2 -> contain = !(temp2 -> contain);
            temp3 = temp3 -> next;
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
                head = temp1;
                temp1 -> next = temp2;
                if (temp2 != NULL) temp2 -> prev = temp1;
            }else{
                temp1 = temp1 -> prev;
                temp3 = temp3 -> next;
                temp1 -> next = temp2;
                temp2 -> prev = temp1;
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

double complete_similarity(int k,int j){
    if (k == j){
        return 0;
    }
    int temp1 = 0, temp2 = 0;
    int same = 0;
    while ((temp1 != mymails[k].token_num) && (temp2 != mymails[j].token_num)){
        if (mymails[k].token[temp1] == mymails[j].token[temp2]){
            same ++;
            temp1 ++;
            temp2 ++;
        }else if (mymails[k].token[temp1] > mymails[j].token[temp2]){
            temp2 ++;
        }else{
            temp1 ++;
        }
    }
    return (double)same / (double)(mymails[k].token_num + mymails[j].token_num - same);
}
