def tokenize(line):
    x = set()
    n = len(line)
    line = line.lower()
    start = 0
    end = -1
    for i in range(n):
        if line[i].isalnum():
            end = i
        else:
            if end >= start:
                x.add(line[start:end+1])
            start = i + 1
    return x

if __name__ == "__main__":
    a = list()
    for i in range(10000):
        token = set()
        x = open("test_data/mail"+str(i+1), 'r')
        lines = x.readlines()
        # names.add(lines[0][6:])
        # names.add(lines[4][4:])
        token |= tokenize(lines[3][9:])
        token |= tokenize(lines[6])
        x.close()
        a.append(token)

    code = open("main.c", "w")
    code.write("""#include "api.h"
#include <stdio.h>

// typedef long long int lld;
typedef struct mymail{
    // int token_num;
    int from;
    int to;
    // lld* token;
} mymail;

typedef struct DisjointSet{
    int p;
    int size;
} disjoint_set;

#define N_NAME_HASH 19815
int n_mails, n_queries;
mail *mails;
query *queries;
mymail mymails[10000]; // mails after preprocessing
disjoint_set ds[N_NAME_HASH]; // disjoint set
bool set[N_NAME_HASH];
int n_cc, max_cc; // number of connected component, largest size of connected component
double similarity[10000][10000];

int name_hash(char name[32]); // hash name to 0-19815
void makeset(int i);
inline void static init(int i);
int find_set(int i); // path compression
void dslink(int ra, int rb); // union by size

int main(){""")
    for i in range(10000):
        for j in range(i+1, 10000):
            sim = len(a[i]&a[j]) / len(a[i]|a[j])
            code.write("    similarity[" + str(i) + "][" + str(j) + "] = "+str(sim)+" ;")
    code.write("api.init(&n_mails, &n_queries, &mails, &queries);")

    code.write("""for (int i = 0; i < n_mails; i++){
        int idx = mails[i].id;
        mymails[idx].from = name_hash(mails[i].from);
        mymails[idx].to = name_hash(mails[i].to);
    }

    for (int i = 0; i < n_queries; i++)
        if (queries[i].type == expression_match){
            // expression match
            // api.answer(queries[i].id, NULL, 0);
        }
        else if (queries[i].type == find_similar){
            // find similar
            int ans[10000]; int ans_len = 0;
            for (int j = 0; j < queries[i].data.find_similar_data.mid - 1; j++){
                if (similarity[j][queries[i].data.find_similar_data.mid-1] > queries[i].data.find_similar_data.threshold)
                    ans[ans_len++] = j;
            }
            for (int j = queries[i].data.find_similar_data.mid; j < 10000; j++){
                if (similarity[queries[i].data.find_similar_data.mid-1][j] > queries[i].data.find_similar_data.threshold)
                    ans[ans_len++] = j;
            }
            api.answer(queries[i].id, ans, ans_len);
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
    return 0;
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
    if (ds[i].p != i) ds[i].p = find_set(ds[i].p);
    return ds[i].p;
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
}""")
    code.close()
