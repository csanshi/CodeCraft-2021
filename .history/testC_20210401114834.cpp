// C++ program to determine worst case
// time complexity of an unordered_map
// using modified hash function
  
#include <bits/stdc++.h>
using namespace std;
using namespace std::chrono;
  
struct modified_hash {
  
    static uint64_t splitmix64(uint64_t x)
    {
        x += 0x9e3779b97f4a7c15;
        x = (x ^ (x >> 30))
            * 0xbf58476d1ce4e5b9;
        x = (x ^ (x >> 27))
            * 0x94d049bb133111eb;
        return x ^ (x >> 31);
    }
  
    int operator()(uint64_t x) const
    {
        static const uint64_t random
            = steady_clock::now()
                  .time_since_epoch()
                  .count();
        return splitmix64(x + random);
    }
};
  
int N = 55000;
int prime1 = 107897;
int prime2 = 126271;
  
// Function to insert in the hashMap
void insert(int prime)
{
    auto start = high_resolution_clock::now();
  
    // Third argument in initialisation
    // of unordered_map ensures that
    // the map uses the hash function
    unordered_map<int, int, modified_hash>
        umap;
  
    // Inserting multiples of prime
    // number as key in the map
    for (int i = 1; i <= N; i++)
        umap[i * prime] = i;
  
    auto stop
        = high_resolution_clock::now();
  
    auto duration
        = duration_cast<milliseconds>(
            stop - start);
  
    cout << "for " << prime << " : "
         << duration.count() / 1000.0
         << " seconds "
         << endl;
}
  
// Driver Code
int main()
{
    // Function call for prime 1
    insert(prime1);
  
    // Function call for prime 2
    insert(prime2);
}