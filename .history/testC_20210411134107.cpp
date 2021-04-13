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
    // unordered_map<int, int, modified_hash> umap;
    unordered_map<int, int> umap;
  
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

void f2(){

    unordered_map<string, string>m; m["abcabc"] = "anb";
    string book[3]; book[0] = "anb";
    int n = 1e5;
    clock_t start = clock();
    while(n--){
        cout << m["abcabc"] << endl;
    }
    clock_t  finish = clock();
    cout << double(finish-start) / CLOCKS_PER_SEC << endl;
}

void f(){

    string book[3]; book[0] = "anb";
    int n = 1e5;
    clock_t start = clock();
    while(n--){
        cout << book[0] << endl;
    }
    clock_t  finish = clock();
    cout << double(finish-start) / CLOCKS_PER_SEC << endl;
}

int main()
{   
    cout << (double)3/2 << endl;
}

// 累计失败次数大于20退出
    void migrate3(){
        int n = this->servers.size();
        int total = this->totalVm;
        int limit = 3*total/100, curMigrationNum = 0;
        auto migrate = [&](){
            int m = this->emptyServers.size(); // 空的服务器个数，迁移的时候不考虑空服务器
#ifdef TEST
        assert(emptyServers.size() + notEmptyServers.size() == n);
#endif
            int idx[n]; for(int i = 0; i < n; i++) idx[i] = i;
            sort(idx, idx + n, [&](int x, int y){ //////////////////////////// 或需要修改
                return this->servers[x].getTotalLeft() > this->servers[y].getTotalLeft();
                
                // return this->servers[x].getTotalUsed() < this->servers[y].getTotalUsed();    
                    // return this->servers[x].vms.size() < this->servers[y].vms.size();
            });
            
            int end = n; // m + (n-m)*2/3
            int cnt_fail = 0;
            int using_chance = false;
            for(int i = m; i < end; i++){
                if(cnt_fail > cnt_fail_thr) break;
                Server& server = this->servers[idx[i]];
                vector<PII>deleted; // (vm_id, node)
                for(auto& vm : server.vms){
                    if(curMigrationNum+1 > limit){
                        if(this->currentDay > T/2){
                            if(!this->used){
                                using_chance = true;
                                end = n;
                            }
                        }
                        
                        if(!using_chance)break;
                    } 
                    int node = -1;
                    int toServer = n-1;
                    while(toServer > i && (node = this->servers[idx[toServer]].addVm2(vm.second, vm.first)) == -1){
                        toServer--;
                    }
                    if(node == -1){ // 无处可插
                        cnt_fail++;
                        continue ;
                        // break;
                    }else{ // 可以迁移
                        // server.delVm(vm.first, node, false); // 不能在这里删除！！！！！
                        this->migrationInfo.push_back({vm.first, this->servers[idx[toServer]].id, node});
                        deleted.push_back({vm.first, vmToServer[vm.first].node}); // 收集delete信息，待本服务器这个vm循环退出再一起移除。
                        vmToServer[vm.first] = {idx[toServer], node};
                        this->notEmptyServers.insert(idx[toServer]);
                        this->emptyServers.erase(idx[toServer]);
                        curMigrationNum++;
                    }
                }
                for(auto& pii : deleted){
                    server.delVm(pii.first, pii.second);
                }
                if(server.vms.empty()){
                    this->emptyServers.insert(idx[i]);
                    this->notEmptyServers.erase(idx[i]);
                }
                if(curMigrationNum+1 > limit && !using_chance) break;
            }
            if(using_chance){
                this->used = true;
            }
        };
        migrate();
        migrate();
        migrate();
        return;
    }
    