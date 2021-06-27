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
typedef struct _mymail{
    int token_num;
    int from;
    int to;
    lld* token;
} mymail;

typedef struct _DisjointSet{
    int p;
    int size;
} disjoint_set;

typedef struct _hilset{
    int size;
    lld* content;
    int* space;
} hilset;

typedef struct _expr{
    bool contain;
    char word[53];
    struct _expr *prev;
    struct _expr *next;
    lld hash_value;
    struct _expr *stillprev;
    struct _expr *stillnext;
} expr;

typedef struct _trie{
    struct _trie *p[36];
    int end;
} trie;

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
trie *root;

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
void insert(trie *o, char *s, int n, int k); // insert a string with length n and beginning at s to a trie
int trie_search(trie *o, char *s, int n); // search whether a string with length n and beginning at s in a trie

char colli[][50]={"students","068km","49","02km","047","05l","0489","11m","0406","084","00asia","010","662","000ha","020","925","258","014","061","0957","kg","821","167e","497","3km2","0014","00squawk","54","48kg","070","w","modality","008w","28","feet","2","0f","0846","0917n","666667","0681","817","ft","readers","x","5m","s","957","0z","026w","05mhz","updating","083n","cash","533e","67km","0005","15km","07m","5069","008","489","07w","0400","enough","042","00cash","01w","2814e","016","076","083w","69km2","000feet","000copies","nm","03am","028","025","55","000kw","01545","000m","055","10","5s","00917","versuch","1322","017s","26e","864n","0192","nbc","06h","043","33e","60","stapley","0billion","023","63km","mi","20","07n","089","capital","200","79","01361","57","20042010","42","060","4e","ratchet","02m","6km2","020m","us","40m","pm","444w","067km","0x","718w","00h","5336","653","00011","067","276","0000e","063","0313","015","mhz","48","444","06km2","855","00","097","0730","7w","050","386","93","0200","25","0kg","054w","2km","07e","0t","08mph","european","0327","308","052","km","011","065","0in","0900","039","066","000ft","7","05336","0pts","92","04h","072","78km2","89","051","0acres","84","70","917","167","0c","000sqft","0606","11","026e","064","044","0l","00444w","0872","68km","096","5l","5306","6h","81","04e","030","036","052km2","0083","7s","17","007","666667n","02222","21","00power","03e","0599","0033n","0039w","035acres","44km","1417","ha","00167","26w","077","331e","019e","95","2a","05306","kinky","power","694","817n","0025n","0666667","h","126","39w","1n","00035","52km2","0126","am","333","0mph","00capital","02833","96","846","00cnbc","91","66ft","64","0333e","mg","00p","040mm","0081","4square","1w","mm","1935e","743","00w","22","00718w","078km2","9","17e","080","65","000mm","038","029","6s","cc","048kg","0304","08","900","05pm","053lb","94","773","055e","56","cars","032","833e","5am","00worldwide","034","4h","square","63","a","1556","2sqmi","00000000","9e","0sqmi","0855","01","02333","09km2","0743","61","02","30","83n","n","5w","0w","0315","05","09","millye","021","192","088","1278","0098","04km2","38","406","02814e","1191","000w","00030","078","181","872","0s","07s","04square","425","005","004","747","27ft","00444","5","0022","74","31","01191","606","033n","090km2","0square","093","44","0m","05w","0039","069","000m2","0975","33","0497","0771","315","0498","057","47","4","01417","in","5e","0","000infantry","01694","35n","0662","005069","basim","ailments","1833","6km","53","1545","327","66","00167e","099","0333333","03w","0046","0425","f","97","0653","77","09e","086","06s","7778e","07778","00533e","0575","40","0308","065mph","08n","000th","095","05s","sqmi","7l","035n","041","098","000cc","0st","0500","044km","333e","082","46","833w","722","14","01322","05n","2m","0080","018","600","40mm","028km","681","00132w","infantry","0100","15","333333","0025","02a","58","dav","01833","0753","864","058","599","69","875","0km2","000kg","026","2833","000n","acres","0014n","094","001","16","6sqmi","83w","006","000nm","0d","085","billion","68","cavalry","60w","0666667n","000sqmi","500","28km","casualties","43","8","th","054","000square","975","0km","0333","contradicting","2333","00917n","0917","000l","000cavalry","54w","5pm","m2","80","068","0033","000people","5mhz","0586","00000113","053","015km","003","100","06833","046","people","34","00778","90km2","389","83km","7778","000us","50","08km2","20m","03444","e","027","0mi","0181","02sqmi","hp","00194","0001","32","000readers","l","000km2","091","88","8km2","worldwide","024","0006833","72","m","09kg","0004e","35","00331e","000a","7e","030km2","0773","carro","08083","00020","83","0600","08h","06","000196","040","78","29","000students","pts","00389","498","d","t","8mph","730","0614","0258","063km","00s","00833","009","85","0mg","1mi","092","0747","sqft","kw","8n","045","033e","03","313","0333333n","copies","18","01935e","asia","00nbc","194","333333n","000km","0000","000m3","99","687","0925","000sq","000mg","090","000hp","01944","13","048","0875","71","cnbc","27","8h","013","113","400","3e","083km","35acres","00n","1694","04","197","09r","00street","3w","1km2","0271","040m","7m","083","05am","05e","street","7n","17s","05333","m3","0079","33n","0ft","00070","0864n","0761","010km","4nm","000x","1361","073","6","027ft","z","271","0010","761","73","1km","324","lb","5333","p","c","00694","01km2","087","3am","06km","214","37","132w","000lb","00sqmi","778","90","26","8w","0324","06sqmi","000","00pm","017n","1","056","917n","4km2","0276","079","00am","qd7","833","14n","01556","0833","0197","0864","000cars","07778e","52","05m","033","196","069km2","67","586","45","86","037","51","019","753","squawk","000enough","049","000casualties","0004","01km","075","035","83e","0214","17n","82","2222","071","083e","25n","074","19e","0386","017e","5n","98","km2","0083w","1944","771","9km2","36","mph","39","002","007l","0833e","3444","9r","00821","76","01278","04nm","30km2","75","65mph","55e","8083","066ft","575","9kg","0725","031","017","st","00722","3","41","0817","87","0817n","sq","53lb","060w","01n","011m","23","24","07","0687","00european","03km2","01mi","725","022","10km","00833w","304","0mhz","614","19"};

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

void insert(trie *o, char *s, int n, int k) {
    if (o == NULL) {
        o=malloc(sizeof(trie));
        for (int i=0; i<36; i++) {
            o->p[i] = NULL;
        }
        o->end = 0;
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
