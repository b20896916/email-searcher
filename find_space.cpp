#include<iostream>
#include<fstream>
#include<set>
#include<cstring>
#define int long long
using namespace std;
set<int> x,y;
const int K=37,MOD=1e9+9;
inline int encode(char c){
    if(c>='0'&&c<='9'){
        return c-'0';
    }
    else if(c>='A'&&c<='Z'){
        return c-'A';
    }
    else{
        return c-'a';
    }
}
inline int f(string s){
    int ans=0;
    for(int i=0;i<s.length();i++){
        ans=(ans*37+encode(s[i]))%MOD;
    }
    return ans;
}
string a[1000000];
signed main(){
    //fstream fin(fstream::in);
    int b,c=0,d=0;
    for(;cin>>a[c];c++){
        b=f(a[c]);
        if(x.find(b)==x.end()){
            x.insert(b);
        }
        else{
            y.insert(b);
        }
    }
    b=0;
    for(auto &i:x){
        if(y.find(i)==y.end()){
            if(i-b>801){
                cout<<i<<'\n';
                break;
            }
            cout<<i<<'\n';
            b=i;
        }
    }
    for(int i=0;i<c;i++){
        if(y.count(f(a[i]))){
            cout<<a[i]<<'\n';
            d++;
        }
    }
    cout<<d<<'\n';
}
