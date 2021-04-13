#include<iostream>
#include<limits>
#include<vector>
using namespace std;

template<class T>class HashTable{
public:
    static const int MOD = 1000000;
    vector<pair<int, T>>book[MOD];
    void insert(int key, T& value){
        int hash = getHash(key);
        this->book[hash].push_back({key, value});
    }
    int getHash(int key){
        return key % MOD;
    }

    T getValue(int key){
        int hash = getHash(key);
        for(auto& kv : book[hash]){
            if(kv.first == key) return kv.second;
        }
    }

    T operator[](int key){
        return this->getValue(key);
    }
};

int main(){
    HashTable<int>book;
    book.insert(3, 100);
    cout << book[3];
    return 0;
}