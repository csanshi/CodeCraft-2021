#include<iostream>
#include<limits>
#include<vector>
using namespace std;

template<class T>class HashTable{
public:
    static const int MOD = 100000;
    vector<pair<int, T>>*book;
    HashTable(){
        this->book = new vector<pair<int, T>>[MOD];
    }
    void insert(int key, T value){
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

    const T operator[](int key){
        return this->getValue(key);
    }

    void erase(int key){
        int hash = getHash(key);
        for(auto it = this->book[hash].begin(); it != this->book[hash].end(); it++){
            if(it->first == key){
                book[hash].erase(it);
                return;
            }
        }
    }

};

int main(){
    HashTable<int>book;
    int v = 100;
    book.insert(3, v);
    cout << book[3];
    book.insert(2, v);
    cout << book[2];
    book.erase(2);
    cout << book[2];

    return 0;
}