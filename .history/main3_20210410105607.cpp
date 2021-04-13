/*
    按照c/m对服务器进行分类，将每个虚拟机映射到对应c/m的服务器
*/

#include<iostream>
#include<string>
#include<unordered_map>
#include<vector>
#include<sstream>
#include<set>
#include<cassert>
#include<algorithm>
#include<time.h>
#include<cstring>
#include<map>
using namespace std;

// #define TEST

typedef pair<int, int>PII;
typedef pair<string, int>PSI;

const int MAX_CLASS_NUM = 100;
int classNum = 50;
double section[MAX_CLASS_NUM], delta;

const char* AB[] = {"A", "B"};
const string inputFilePath = "training-data/training-1.txt"; // training-2 example
const string outputFilePath = "output.txt";

struct Request{
    char op;
    string model;
    int id;
};

struct Position{
    int serverClass, serverIdx, serverNode; 
};

struct ServerInfo{
    int core, mem;
    int cost_hardware, cost_power;
    double ratio;
};

typedef pair<string, ServerInfo> PSS;

struct VirtualMachineInfo{
    int core, mem;
    int type;
    double ratio;
    int serverClass;
};

class ServerSetWithClass{
public:
    vector<PSS>_serverDict;
    ServerSetWithClass(){
        _serverDict.clear();
    }
    string preferedModel(){
        int n = this->_serverDict.size();
        // return this->_serverDict[n/2].first;
        return "hostJJA26";
    }
    void sortServerByRatio(){
        sort(this->_serverDict.begin(), this->_serverDict.end(), [](PSS& a, PSS& b){
            return a.second.ratio < b.second.ratio;
        });
    }
};

unordered_map<string, ServerInfo>serverDict;
// map<string, ServerInfo>serverDict;
vector<ServerSetWithClass>serverDictWithClass;
unordered_map<string, VirtualMachineInfo>vmDict;
unordered_map<int, Position>vmToServer; // 虚拟机编号--->(服务器类型，服务器编号，节点编号)

struct Tuple{
    int serverId, node, vmId;
};

void buildOutputStream(const vector<PSI>&book, const vector<int>&migration, const vector<Tuple>addInfo){
    cout << "(purchase, " << book.size() << ")\n";
    for(auto& kv : book){
        cout << "(" << kv.first << ", " << kv.second << ")\n";
    }
    cout  << "(migration, 0)\n";
    for(auto& e : migration){
        cout << e << endl;
    }
    for(auto& e : addInfo){
        int id = e.serverId, node = e.node;
        if(node != 2) cout << "(" << id << ", " << AB[node] << ")\n" ;
        else cout << "(" << id << ")\n";
    }
}

class Server{
public:
    int core[2]; // 0-A 1-B
    int mem[2]; // 0-A 1-B
    int powerCost;
    int id;
    unordered_map<int, VirtualMachineInfo>vms;
    Server(int _core, int _mem, int powerCost){
        core[0] = core[1] = _core/2;
        mem[0] = mem[1] = _mem/2;
        this->powerCost = powerCost;
        vms.clear();
    }
    Server(const ServerInfo& serverInfo){
        core[0] = core[1] = serverInfo.core/2;
        mem[0] = mem[1] = serverInfo.mem/2;
        this->powerCost = serverInfo.cost_power;
    }
    /*
        不能容纳，返回-1；
        否则：
            单结点虚拟机：返回节点信息。0表示A节点，1表示B节点
            双节点虚拟机：返回2
    */
    int addVm(string model, int id){
        VirtualMachineInfo vm = vmDict[model];
        if(vm.type == 0){ // 单结点虚拟机
            for(int node = 0; node < 2; node++){
                if(core[node] >= vm.core && mem[node] >= vm.mem){
                    core[node] -= vm.core;
                    mem[node] -= vm.mem;
                    vms[id] = vm;
                    return node;
                }
            }
        }else{ // 双节点虚拟机
            for(int node = 0; node < 2; node++){
                if(core[node] < vm.core/2 || mem[node] < vm.mem/2) return -1;
            }
            core[0] -= vm.core/2; core[1] -= vm.core/2;
            mem[0] -= vm.mem/2; mem[1] -= vm.mem/2;
            vms[id] = vm;
            return 2;
        }
        return -1;
    }
    /*
        node: 0-A节点，1-B节点，2-AB节点
    */
    void delVm(int id, int node){
        VirtualMachineInfo vm = vms[id];
        if(node == 0 || node == 1){
            core[node] += vm.core;
            mem[node] += vm.mem;
        }else{
            core[0] += vm.core/2; core[1] += vm.core/2;
            mem[0] += vm.mem/2; mem[1] += vm.mem/2;
        }
        vms.erase(id);
    }

};


class ServerResource{
public:
    vector<Server>serversWithClass[MAX_CLASS_NUM];
    int addedServersWithC/* 
0.8 1958920890
1.0 1954344193
1.2 1954309840
1.3 1953515974
1.4 1953488536
1.5 1950927664
1.6 1952911169
1.7 1953544667
1.8 1953738027
*/
#include<iostream>
#include<string>
#include<vector>
#include<unordered_map>
#include<sstream>
#include<unordered_set>
#include<cassert>
#include<cstring>
#include<algorithm>
#include<ctime>
#include <chrono>
using namespace std;

#define TEST
#define ONE_KIND
const int INF = 0x3f3f3f3f;
const char* AB[] = {"A", "B"};
const string inputFilePaths[3] = {" ","training-data/training-1.txt", "training-data/training-2.txt"};
int testedFile = 1;
string inputFilePath = inputFilePaths[testedFile];
const string outputFilePath = "output.txt";
// const double ratio1 = 1.5, ratio2 = 1.2;
const double myRatio = 1.5;
string serverModel = "hostDPJA1"; // hostATVAZ hostDPJA1 host7SRCB
// int purchaseVersion = 1; // 0-只买一种 1-买最便宜
int migrationVersion = 2; // 0-AB无选择地插入，1-AB有选择地插入 2-改成continue & 做两次
int deployVersion = 0; // 0-打分 1-排序


typedef pair<int, int>PII;
typedef pair<string, int>PSI;
struct Tuple{
    int serverId, node, vmId;
};

struct ServerInfo{
    int core, mem;
    int cost_hardware, cost_power;
    string model;
};
struct VirtualMachineInfo{
    int core, mem, type;
};

struct Position{
    int idx, node;
};

struct MigrateInfo{
    int vm_id,server_id, node;
};

unordered_map<string, ServerInfo>serverDict;
vector<ServerInfo>sortedServerList;
unordered_map<string, VirtualMachineInfo>vmDict;
// vector<string, VirtualMachineInfo>sortedVmList;

  
unordered_map<int, Position>vmToServer;

unordered_map<string, string>vmToMinPowerCostServer;

struct Request{
    char op;
    string model;
    int id;
};

class Server{
public:
    int core[2], mem[2]; // left
    int powerCost;
    int id;
    int capacity_core, capacity_mem;
    unordered_map<int, VirtualMachineInfo>vms;

    Server(const ServerInfo& serverInfo){
        capacity_core = serverInfo.core;
        capacity_mem = serverInfo.mem;
        core[0] = core[1] = serverInfo.core/2;
        mem[0] = mem[1] = serverInfo.mem/2;
        this->powerCost = serverInfo.cost_power;
        vms.clear();
    }
    void printInfo(){
        cout << "A: " << core[0] << ' ' << mem[0] << endl;
        cout << "B: " << core[1] << ' ' << mem[1] << endl;
        cout << "capacity_core: " << capacity_core << " capacity_mem: " << capacity_mem << endl; 
    }
    double getTotalLeft(){
        return (this->core[0] + this->core[1])*myRatio +
                (this->mem[0] + this->mem[1]);
    }
    double getTotalUsed(){
        return this->capacity_core + this->capacity_mem - this->getTotalLeft();
    }
    /*
        传入vm
        返回：1. 对于单结点虚拟机: A、B节点中cost较小的那个cost,同时返回true. res = (node, cost)
              2. 对于双节点虚拟机：返回true. res = (2, cost)
              无法插入返回false
    */
    bool getCost2(VirtualMachineInfo& vm, PII& res){
        if(vm.type == 0){ // 单结点虚拟机
            int minCost = INF, minNode = -1;
            for(int node = 0; node < 2; node++){
                if(core[node] >= vm.core && mem[node] >= vm.mem){
                    int cost = abs((core[node] - vm.core)*myRatio - (mem[node] - vm.mem));
                    if(cost < minCost){
                        minCost = cost;
                        minNode = node;
                    }
                }
            }
            if(minNode == -1) return false;
            res = {minNode, minCost};
            return true;
        }else{ // 双节点虚拟机
            for(int node = 0; node < 2; node++){
                if(core[node] < vm.core/2 || mem[node] < vm.mem/2) return false;
            }
            int cost = abs((core[0] - vm.core/2 + core[1] - vm.core/2)*myRatio - 
                            (mem[0] - vm.mem/2 + mem[1] - vm.mem/2));
            res = {2, cost};
            return true;
        }
        return false;
    }
    bool getCost(VirtualMachineInfo& vm, PII& res){
        if(vm.type == 0){ // 单结点虚拟机
            int minCost = INF, minNode = -1;
            for(int node = 0; node < 2; node++){
                if(core[node] >= vm.core && mem[node] >= vm.mem){
                    int cost = this->getTotalLeft();
                    if(cost < minCost){
                        minCost = cost;
                        minNode = node;
                    }
                }
            }
            if(minNode == -1) return false;
            res = {minNode, minCost};
            return true;
        }else{ // 双节点虚拟机
            for(int node = 0; node < 2; node++){
                if(core[node] < vm.core/2 || mem[node] < vm.mem/2) return false;
            }
            int cost = this->getTotalLeft();
            res = {2, cost};
            return true;
        }
        return false;
    }
    // A B节点无区别插
    int addVm(VirtualMachineInfo& vm, int id){
        if(vm.type == 0){ // 单结点虚拟机
            for(int node = 0; node < 2; node++){
                if(core[node] >= vm.core && mem[node] >= vm.mem){
                    core[node] -= vm.core;
                    mem[node] -= vm.mem;
                    vms[id] = vm;
                    return node;
                }
            }
        }else{ // 双节点虚拟机
            for(int node = 0; node < 2; node++){
                if(core[node] < vm.core/2 || mem[node] < vm.mem/2) return -1;
            }
            core[0] -= vm.core/2; core[1] -= vm.core/2;
            mem[0] -= vm.mem/2; mem[1] -= vm.mem/2;
            vms[id] = vm;
            return 2;
        }
        return -1;
    }
    // A B节点有选择地插
    int addVm2(VirtualMachineInfo& vm, int id){
        PII res;
        int ok = this->getCost(vm, res);
        if(!ok) return -1;
        this->addVm(vm, id, res.first);
        return res.first;
    }
    int addVm(string model, int id){
        VirtualMachineInfo vm = vmDict[model];
        if(vm.type == 0){ // 单结点虚拟机
            for(int node = 0; node < 2; node++){
                if(core[node] >= vm.core && mem[node] >= vm.mem){
                    core[node] -= vm.core;
                    mem[node] -= vm.mem;
                    vms[id] = vm;
                    return node;
                }
            }
        }else{ // 双节点虚拟机
            for(int node = 0; node < 2; node++){
                if(core[node] < vm.core/2 || mem[node] < vm.mem/2) return -1;
            }
            core[0] -= vm.core/2; core[1] -= vm.core/2;
            mem[0] -= vm.mem/2; mem[1] -= vm.mem/2;
            vms[id] = vm;
            return 2;
        }
        return -1;
    }
    void addVm(string model, int id, int node){
        VirtualMachineInfo vm = vmDict[model];
        if(vm.type == 0){ // 单结点虚拟机
            core[node] -= vm.core;
            mem[node] -= vm.mem;
            vms[id] = vm;
        }else{ // 双节点虚拟机
            core[0] -= vm.core/2; core[1] -= vm.core/2;
            mem[0] -= vm.mem/2; mem[1] -= vm.mem/2;
            vms[id] = vm;
        }
    }
    void addVm(VirtualMachineInfo& vm, int id, int node){
        if(vm.type == 0){ // 单结点虚拟机
            core[node] -= vm.core;
            mem[node] -= vm.mem;
            vms[id] = vm;
        }else{ // 双节点虚拟机
            core[0] -= vm.core/2; core[1] -= vm.core/2;
            mem[0] -= vm.mem/2; mem[1] -= vm.mem/2;
            vms[id] = vm;
        }
    }
    /*
        node: 0-A节点，1-B节点，2-AB节点
    */
    void delVm(int id, int node){
        VirtualMachineInfo vm = vms[id];
        if(node == 0 || node == 1){
            core[node] += vm.core;
            mem[node] += vm.mem;
        }else{
            core[0] += vm.core/2; core[1] += vm.core/2;
            mem[0] += vm.mem/2; mem[1] += vm.mem/2;
        }
        vms.erase(id);
    }

};

void buildOutputStream(const vector<PSI>&book, const vector<MigrateInfo>&migrationInfo, const vector<Tuple>addInfo, vector<int>&idxForAddInfo){ //
    cout << "(purchase, " << book.size() << ")\n";
    for(auto& kv : book){
        cout << "(" << kv.first << ", " << kv.second << ")\n";
    }

    cout << "(migration, " << migrationInfo.size() << ")" << endl;
    for(auto& e : migrationInfo){
        if(e.node == 2){
            cout << "(" << e.vm_id << ", " << e.server_id << ")\n";
        }else{
            cout << "(" << e.vm_id << ", " << e.server_id << ", " << AB[e.node] << ")\n";
        }
    }

    int n = idxForAddInfo.size();
    int A[n];
    for(int i = 0; i < n; i++){
        A[idxForAddInfo[i]] = i;
    }
    for(int i = 0; i < n; i++){
        const Tuple& e = addInfo[A[i]];
        int id = e.serverId, node = e.node;
        if(node != 2) cout << "(" << id << ", " << AB[node] << ")\n" ;
        else cout << "(" << id << ")\n";
    }
    fflush(stdout);
}

class ServerResource{
public:
    vector<Server>servers;
    long long COST_HARDWARE, COST_POWER;
    unordered_set<int>emptyServers, notEmptyServers;

    vector<PSI>purchaseInfo; // (server_model, vm_id)
    vector<MigrateInfo>migrationInfo;
    vector<Tuple>addInfo;
    vector<int>idxForAddInfo;
    vector<PSI>addRequests;
    int totalVm = 0;

    ServerResource(){
        servers.clear();
        COST_HARDWARE = COST_POWER = 0;
        this->totalVm = 0;
    }
    int getEmptyNumber(){
        int emptyNumer = 0;
        for(auto& server : this->servers)
            if(server.vms.empty()) emptyNumer ++;
        return emptyNumer;
    }
    void printServers(){
        for(auto& server: this->servers){
            server.printInfo();
            cout << endl;
        }
    }
    void handleRequestsInTheFirstKDays(vector<Request>requestsInTheFirstKDays[], int K){
        for(int i = 0; i < K; i++)
            this->handleRequestsInOneDay(requestsInTheFirstKDays[i]);
    }
    void handleRequestsInOneDay(vector<Request>& requests){
        for(auto& s : emptyServers){
            assert(servers[s].vms.empty());
        }
        for(auto& s : notEmptyServers){
            assert(!servers[s].vms.empty());
        }
        this->addInfo.clear();
        this->purchaseInfo.clear();
        this->idxForAddInfo.clear();
        this->addRequests.clear();
        this->migrationInfo.clear();

#ifdef TEST
        cout << "before migration: " << this->getEmptyNumber() << endl;
#endif
        this->migrateByVersion(migrationVersion);
#ifdef TEST
        cout << "after migration: " << this->getEmptyNumber() << endl;
        cout << this->servers.size() << endl;
        this->printServers(); cout << endl;
        cout << "############################################################\n";
#endif
        int n = requests.size();
        int cnt_add = 0;
        for(int i = 0; i < n;){
            if(requests[i].op == 'a'){
                int j = i;
                while(j < n && requests[j].op == 'a'){
                    addRequests.push_back({requests[j].model, requests[j].id});
                    j++;
                }
                for(int k = 0; k < (j-i); k++) idxForAddInfo.push_back(cnt_add + k);
                
                // 给add请求按照所要求的资源从大到小排序
                sort(this->idxForAddInfo.begin() + cnt_add, this->idxForAddInfo.begin() + cnt_add + j-i, [&](int x, int y){
                    return myRatio*vmDict[addRequests[x].first].core + vmDict[addRequests[x].first].mem
                        >  myRatio*vmDict[addRequests[y].first].core + vmDict[addRequests[y].first].mem;
                });
                for(int k = cnt_add; k < cnt_add + j-i; k++) {
                    PSI& addRequest = addRequests[idxForAddInfo[k]];
                    this->deployByVersion(addRequest.first, addRequest.second, deployVersion);
                    this->totalVm++;
                }

                cnt_add += j - i;
                i = j;
            } else {
                this->handleDeleteRequest(requests[i].id);
                this->totalVm--;
                i++;
            }
        }
        n = this->purchaseInfo.size();
        vector<PSI>book;
        // 编号重排，构造输出信息
        if(n > 0){
            int res[n]; memset(res, -1, sizeof(res));
            int cnt = 0;
            for(int i = 0; i < n; i++){
                if(res[i] != -1) continue;
                res[i] = cnt++;
                int j = i+1, num = 1;
                for(; j < n; j++){
                    if(this->purchaseInfo[j].first == this->purchaseInfo[i].first){
                        res[j] = cnt++;
                        num++;
                    }
                }
                book.push_back({purchaseInfo[i].first, num});
            }
            // 新购买的服务器，需要修改其id
            for(int i = 0; i < n; i++){
                auto& pi = purchaseInfo[i];
                const Position& pos = vmToServer[pi.second];
                this->servers[pos.idx].id =this->servers.size() - n + res[i];
            }
            // 部署信息
            for(auto& e : addInfo){
                const Position& pos = vmToServer[e.vmId];
                e.serverId = this->servers[pos.idx].id;
            }
        }
        buildOutputStream(book, this->migrationInfo, addInfo, this->idxForAddInfo); //, this->idxForAddInfo
#ifdef TEST
        this->calPowerCost();
#endif
    }

    string purchaseAsRequired(string vmModel){
        VirtualMachineInfo& vm = vmDict[vmModel];
        // string preferedModel = purchaseVersion == 0 ?  serverModel : vmToMinPowerCostServer[vmModel];
        // string preferedModel = vmToMinPowerCostServer[vmModel];
#ifdef ONE_KIND
        string preferedModel = serverModel;
#else
        string preferedModel = vmToMinPowerCostServer[vmModel];
#endif
        ServerInfo server = serverDict[preferedModel];
        this->servers.push_back(server);
        COST_HARDWARE += server.cost_hardware;
        return preferedModel;
    }
    
    void migrateByVersion(int version){
        switch (version){
            case 0:
                this->migrate0();
                break;
            case 1:
                this->migrate1();
                break;
            case 2:
                this->migrate2();
                break;
            default:
                break;
        }
    }
    void deployByVersion(string model, int id, int version){
        switch (version){
            case 0:
                this->deploy0(model, id);
                break;
            default:
                break;
        }
    }
    
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
    // A B无选择地插入，即调用addVm
    void migrate0(){ 
        int n = this->servers.size();
        int idx[n]; for(int i = 0; i < n; i++) idx[i] = i;
        sort(idx, idx + n, [&](int x, int y){
            // return this->servers[x].getTotalLeft() > this->servers[y].getTotalLeft();
            
            return this->servers[x].getTotalUsed() < this->servers[y].getTotalUsed();    
                // return this->servers[x].vms.size() < this->servers[y].vms.size();
        });
        int total = this->totalVm;
        int limit = 3*total/100, curMigrationNum = 0;
        for(int i = 0; i < n-1; i++){
            Server& server = this->servers[idx[i]];
            vector<PII>deleted; // (vm_id, node)
            for(auto& vm : server.vms){
                if(curMigrationNum+1 > limit) break;
                int node = -1;
                int toServer = n-1;
                while(toServer > i && (node = this->servers[idx[toServer]].addVm(vm.second, vm.first)) == -1){
                    toServer--;
                }
                if(node == -1){ // 无处可插
                    break ;
                }else{
                    // server.delVm(vm.first, node, false); // 不能在这里删除！！！！！
                    this->migrationInfo.push_back({vm.first, this->servers[idx[toServer]].id, node});
                    deleted.push_back({vm.first, vmToServer[vm.first].node}); // 收集delete信息，待本服务器这个vm循环退出再一起移除。
                    vmToServer[vm.first] = {idx[toServer], node};
                    curMigrationNum++;
                }
            }
            for(auto& pii : deleted){
                server.delVm(pii.first, pii.second);
            }
            if(curMigrationNum+1 > limit) break;
        }
        return;
    }
    // A B有选择地插入，即调用addVm2
    void migrate1(){
        int n = this->servers.size();
        int idx[n]; for(int i = 0; i < n; i++) idx[i] = i;
        sort(idx, idx + n, [&](int x, int y){
            return this->servers[x].getTotalLeft() > this->servers[y].getTotalLeft();
            
            // return this->servers[x].getTotalUsed() < this->servers[y].getTotalUsed();    
                // return this->servers[x].vms.size() < this->servers[y].vms.size();
        });
        int total = this->totalVm;
        int limit = 3*total/100, curMigrationNum = 0;
        for(int i = 0; i < n-1; i++){
            Server& server = this->servers[idx[i]];
            vector<PII>deleted; // (vm_id, node)
            for(auto& vm : server.vms){
                if(curMigrationNum+1 > limit) break;
                int node = -1;
                int toServer = n-1;
                while(toServer > i && (node = this->servers[idx[toServer]].addVm2(vm.second, vm.first)) == -1){
                    toServer--;
                }
                if(node == -1){ // 无处可插
                    break ;
                }else{
                    // server.delVm(vm.first, node, false); // 不能在这里删除！！！！！
                    this->migrationInfo.push_back({vm.first, this->servers[idx[toServer]].id, node});
                    deleted.push_back({vm.first, vmToServer[vm.first].node}); // 收集delete信息，待本服务器这个vm循环退出再一起移除。
                    vmToServer[vm.first] = {idx[toServer], node};
                    curMigrationNum++;
                }
            }
            for(auto& pii : deleted){
                server.delVm(pii.first, pii.second);
            }
            if(curMigrationNum+1 > limit) break;
        }
        return;
    }

    void migrate2(){
        int n = this->servers.size();
        int m = this->emptyServers.size(); // 空的服务器个数，迁移的时候不考虑空服务器
        int idx[n]; for(int i = m; i < n; i++) idx[i] = i;
        sort(idx+m, idx + n, [&](int x, int y){
            return this->servers[x].getTotalLeft() > this->servers[y].getTotalLeft();
            
            // return this->servers[x].getTotalUsed() < this->servers[y].getTotalUsed();    
                // return this->servers[x].vms.size() < this->servers[y].vms.size();
        });
        int total = this->totalVm;
        int limit = 3*total/100, curMigrationNum = 0;
        // int m = 0; while(m < n && this->servers[idx[m]].vms.empty()) m++;
        int end = m + (n-m)*2/3;
        for(int i = m; i < end; i++){
            Server& server = this->servers[idx[i]];
            vector<PII>deleted; // (vm_id, node)
            for(auto& vm : server.vms){
                if(curMigrationNum+1 > limit) break;
                int node = -1;
                int toServer = n-1;
                while(toServer > i && (node = this->servers[idx[toServer]].addVm2(vm.second, vm.first)) == -1){
                    toServer--;
                }
                if(node == -1){ // 无处可插
                    continue ;
                }else{
                    // server.delVm(vm.first, node, false); // 不能在这里删除！！！！！
                    this->migrationInfo.push_back({vm.first, this->servers[idx[toServer]].id, node});
                    deleted.push_back({vm.first, vmToServer[vm.first].node}); // 收集delete信息，待本服务器这个vm循环退出再一起移除。
                    vmToServer[vm.first] = {idx[toServer], node};
                    this->notEmptyServers.insert(idx[toServer]);
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
            if(curMigrationNum+1 > limit) break;
        }
        m = this->emptyServers.size();
        for(int i = m; i < n; i++) idx[i] = i;
        sort(idx+m, idx + n, [&](int x, int y){
            return this->servers[x].getTotalLeft() > this->servers[y].getTotalLeft();
            
            // return this->servers[x].getTotalUsed() < this->servers[y].getTotalUsed();    
                // return this->servers[x].vms.size() < this->servers[y].vms.size();
        });
        // m = 0; while(m < n && this->servers[idx[m]].vms.empty()) m++;
        end = m + (n-m)/3;

        for(int i = m; i < end; i++){
            Server& server = this->servers[idx[i]];
            vector<PII>deleted; // (vm_id, node)
            for(auto& vm : server.vms){
                if(curMigrationNum+1 > limit) break;
                int node = -1;
                int toServer = n-1;
                while(toServer > i && (node = this->servers[idx[toServer]].addVm2(vm.second, vm.first)) == -1){
                    toServer--;
                }
                if(node == -1){ // 无处可插
                    break ;
                }else{
                    // server.delVm(vm.first, node, false); // 不能在这里删除！！！！！
                    this->migrationInfo.push_back({vm.first, this->servers[idx[toServer]].id, node});
                    deleted.push_back({vm.first, vmToServer[vm.first].node}); // 收集delete信息，待本服务器这个vm循环退出再一起移除。
                    vmToServer[vm.first] = {idx[toServer], node};
                    this->notEmptyServers.insert(idx[toServer]);
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
            if(curMigrationNum+1 > limit) break;
        }
        // for(int i = 0; i < n; i++){
        //     Server& server = this->servers[idx[i]];
        //     vector<PII>deleted; // (vm_id, node)
        //     for(auto& vm : server.vms){
        //         if(curMigrationNum+1 > limit) break;
        //         int node = -1;
        //         int toServer = n-1;
        //         while(toServer > i && (node = this->servers[idx[toServer]].addVm2(vm.second, vm.first)) == -1){
        //             toServer--;
        //         }
        //         if(node == -1){ // 无处可插
        //             continue ;
        //         }else{
        //             // server.delVm(vm.first, node, false); // 不能在这里删除！！！！！
        //             this->migrationInfo.push_back({vm.first, this->servers[idx[toServer]].id, node});
        //             deleted.push_back({vm.first, vmToServer[vm.first].node}); // 收集delete信息，待本服务器这个vm循环退出再一起移除。
        //             vmToServer[vm.first] = {idx[toServer], node};
        //             curMigrationNum++;
        //         }
        //     }
        //     for(auto& pii : deleted){
        //         server.delVm(pii.first, pii.second);
        //     }
        //     if(curMigrationNum+1 > limit) break;
        // }
        
        // for(int i = 0; i < n; i++) idx[i] = i;
        // sort(idx, idx + n, [&](int x, int y){
        //     return this->servers[x].getTotalLeft() > this->servers[y].getTotalLeft();
            
        //     // return this->servers[x].getTotalUsed() < this->servers[y].getTotalUsed();    
        //         // return this->servers[x].vms.size() < this->servers[y].vms.size();
        // });
        // for(int i = 0; i < n/2; i++){
        //     Server& server = this->servers[idx[i]];
        //     vector<PII>deleted; // (vm_id, node)
        //     for(auto& vm : server.vms){
        //         if(curMigrationNum+1 > limit) break;
        //         int node = -1;
        //         int toServer = n-1;
        //         while(toServer > i && (node = this->servers[idx[toServer]].addVm2(vm.second, vm.first)) == -1){
        //             toServer--;
        //         }
        //         if(node == -1){ // 无处可插
        //             continue ;
        //         }else{
        //             // server.delVm(vm.first, node, false); // 不能在这里删除！！！！！
        //             this->migrationInfo.push_back({vm.first, this->servers[idx[toServer]].id, node});
        //             deleted.push_back({vm.first, vmToServer[vm.first].node}); // 收集delete信息，待本服务器这个vm循环退出再一起移除。
        //             vmToServer[vm.first] = {idx[toServer], node};
        //             curMigrationNum++;
        //         }
        //     }
        //     for(auto& pii : deleted){
        //         server.delVm(pii.first, pii.second);
        //     }
        //     if(curMigrationNum+1 > limit) break;
        // }
    
        // int toServer = n-1;
    // for(int i = 0; i < toServer; i++){
    //     Server& server = this->servers[idx[i]];
    //     vector<PII>deleted; // (vm_id, node)
    //     for(auto& vm : server.vms){
    //         if(curMigrationNum+1 > limit) break;
    //         int node;
    //         while(toServer > i && (node =this->servers[idx[toServer]].addVm2(vm.second, vm.first)) == -1){
    //             toServer--;
    //         }
    //         if(node == -1){ // 无处可插
    //             break ;
    //         }else{
    //             // server.delVm(vm.first, node, false); // 不能在这里删除！！！！！
    //             // assert(false);
    //             this->migrationInfo.push_back({vm.first, this->servers[idx[toServer]].id, node});
    //             deleted.push_back({vm.first, vmToServer[vm.first].node}); // 收集delete信息，待本服务器这个vm循环退出再一起移除。
    //             vmToServer[vm.first] = {idx[toServer], node};
    //             curMigrationNum++;
    //         }
    //     }
    //     for(auto& pii : deleted){
    //         server.delVm(pii.first, pii.second);
    //     }
    //     if(curMigrationNum+1 > limit) break;
    // }
        
        return;
    }
    int bestFit(string model, int id, PII& res){
        VirtualMachineInfo vm = vmDict[model];
        int idx1 = -1, idx2 = -1;
        PII minRes1 = {-1, INF}, minRes2 = {-1, INF};
        int n = this->servers.size();
        for(int i = 0; i < n; i++){
            Server& server = this->servers[i];
            PII res;
            int ok = server.getCost(vm, res);
            if(!ok) continue;
            if(!server.vms.empty() && res.second < minRes1.second){
                minRes1 = res;
                idx1 = i;
            }else if(server.vms.empty() && res.second < minRes2.second){
                minRes2 = res;
                idx2 = i;
            }
        }
        if(idx1 != -1) {
            res = minRes1; return idx1;
        }else if(idx2 != -1){
            res = minRes2; return idx2;
        }
        return -1;
    }
    // 使用了新设的 空服务器集合 和 非空服务器集合
    int bestFit2(string model, int id, PII& res){
        VirtualMachineInfo vm = vmDict[model];
        int idx = -1; PII minRes = {-1, INF};
        for(auto& i : this->notEmptyServers){ // 先在非空服务器里找
            Server& server = this->servers[i];
            PII res;
            int ok = server.getCost(vm, res);
            if(!ok) continue;
            if(res.second < minRes.second){
                minRes = res;
                idx = i;
            }
        }
        if(idx != -1) { // 非空服务器里找到了
            res = minRes; 
            return idx;
        }
        // 在空的服务器找
        idx = -1; minRes = {-1, INF};
        for(auto& i : this->emptyServers){
            Server& server = this->servers[i];
            PII res;
            int ok = server.getCost(vm, res);
            if(!ok) continue;
            if(res.second < minRes.second){
                minRes = res;
                idx = i;
            }
        }
        if(idx != -1){ // 空的服务器里找到了
            res = minRes; 
            this->emptyServers.erase(idx);
            this->notEmptyServers.insert(idx);
            return idx;
        }
        return -1; // 现有服务器都找不到
    }
    void deploy0(string model, int id){
        VirtualMachineInfo vm = vmDict[model];
        PII minRes = {-1, INF};
        int idx = bestFit(model, id, minRes);
        if(idx != -1){ // 在已有的服务器中找到了
            this->servers[idx].addVm(model, id, minRes.first);
            this->addInfo.push_back({servers[idx].id, minRes.first, id});
            vmToServer[id] = {idx, minRes.first};
        }else{ // 需要购买
            string serverModel =  this->purchaseAsRequired(model);
            this->notEmptyServers.insert(this->servers.size()-1);
            this->purchaseInfo.push_back({serverModel, id}); // 为编号为id的虚拟机购买了型号为serverModel的服务器
            int node = this->servers.back().
            addVm2(vm, id);
            this->addInfo.push_back({-1, node, id});
            vmToServer[id] = {(int)(this->servers.size()-1), node};
        }
    }    
    
    void handleDeleteRequest(int id){
        const Position& pos = vmToServer[id];
        this->servers[pos.idx].delVm(id, pos.node);
        vmToServer.erase(id); ////////////////////
        if(this->servers[pos.idx].vms.empty()){
            this->emptyServers.insert(pos.idx);
            this->notEmptyServers.erase(pos.idx);
        }
    }
    void calPowerCost(){
        for(auto& server : this->servers){
            if(!server.vms.empty()){
                this->COST_POWER += server.powerCost;
            }
        }
    }
};

ServerResource serverResource;

void readServerInfos(string model, string core, string mem, string cost1, string cost2){
    int i_core = stoi(core);
    int i_mem = stoi(mem);
    int i_cost1 = stoi(cost1);
    int i_cost2 = stoi(cost2);
    serverDict[model] = {i_core, i_mem, i_cost1, i_cost2, model};
    sortedServerList.push_back({i_core, i_mem, i_cost1, i_cost2, model});
}
void readVmInfos(string model, string core, string mem, string isDouble){
    int i_core = stoi(core);
    int i_mem = stoi(mem);
    vmDict[model] = {i_core, i_mem, isDouble[0]-'0'};
}
void buildServerAndVmDict(){
    // 读服务器
    int N; cin >> N; // 最大100
    for(int i = 0; i < N; i++){
        string model, core, mem, cost1, cost2;
        cin >> model >> core >> mem >> cost1 >> cost2;
        readServerInfos(model.substr(1, model.size()-2), 
            core.substr(0, core.size()-1), mem.substr(0, mem.size()-1), 
            cost1.substr(0, cost1.size()-1), cost2.substr(0, cost2.size()-1));
    }

    // 预处理服务器
    sort(sortedServerList.begin(), sortedServerList.end(), [](ServerInfo& x, ServerInfo& y){
        return x.cost_power < y.cost_power;
    });

    // 读虚拟机
    int M; cin >> M; // 最大1000
    for(int i = 0; i < M; i++){
        string model, core, mem, isDouble;
        cin >> model >> core >> mem >> isDouble;
        readVmInfos(model.substr(1, model.size()-2), 
            core.substr(0, core.size()-1), mem.substr(0, mem.size()-1), 
            isDouble.substr(0, isDouble.size()-1));
    }

}


void buildVmToServerWithMinPowerCost(){
    auto check = [](VirtualMachineInfo& vm, ServerInfo& server){
        if(vm.type == 0){
            return vm.core <= server.core/2 && vm.mem <= server.mem/2;
        }else{
            return vm.core <= server.core && vm.mem <= server.mem;
        }
    };
    for(auto& vm : vmDict){
        // vmToMinPowerCostServer[vm.first] = serverModel;
        for(auto& server : sortedServerList){
            if(check(vm.second, server)){
                vmToMinPowerCostServer[vm.first] = server.model;
                break;
            }
        }
    }
}

void solve(){
    int T, K; cin >> T >> K; // 最大1000 ，add操作不超过100000
    auto readOneDay = [](vector<Request>& requests){
        int R; cin >> R;
        while(R--){
            string op, model, id;
            cin >> op;
            if(op[1] == 'a') cin >> model >> id;
            else cin >> id;
            requests.push_back(
                {op[1], model.substr(0, model.size()-1), stoi(id.substr(0, id.size()-1))}
            );
        }
    };
    vector<Request> requestsInTheFirstKDays[K];
    for(int t = 0; t < K; t++){
        readOneDay(requestsInTheFirstKDays[t]);
    }
    serverResource.handleRequestsInTheFirstKDays(requestsInTheFirstKDays, K);
    for(int t = K; t < T; t++){
        vector<Request>requestInfos;
        readOneDay(requestInfos);
        serverResource.handleRequestsInOneDay(requestInfos);
    }
}

int main(){
#ifdef TEST //重定向
    freopen(inputFilePath.c_str(), "rb", stdin);
    freopen(outputFilePath.c_str(), "wb", stdout);
    clock_t start, finish;
    start = clock();
#endif
    ios::sync_with_stdio(false);

    buildServerAndVmDict();

    buildVmToServerWithMinPowerCost();
    solve();

#ifdef TEST
    finish = clock();
    cout << endl;
    cout << "data" << testedFile << ":" << endl;
    cout << "COST_HARDWARE: " << serverResource.COST_HARDWARE << endl;
    cout << "COST_POWER: " << serverResource.COST_POWER << endl;
    cout << "COST: " << serverResource.COST_HARDWARE + serverResource.COST_POWER << endl;
    cout << "Number of vm: " << serverResource.totalVm << endl;
    cout << "Time used: " << (finish-start) / CLOCKS_PER_SEC << "s" << endl;
#endif

    // system("pause");
    return 0;
}lassInOneDay[MAX_CLASS_NUM];
    int serverNumber;
    long long COST_HARDWARE, COST_POWER;
    vector<PSI>purchaseInfo; // (服务器型号，虚拟机id) 
    vector<Tuple>addInfo; // 服务器编号，节点, 虚拟机id

    ServerResource(){
        for(auto& servers : serversWithClass){
            servers.clear();
        }
        COST_HARDWARE = COST_POWER = serverNumber = 0;
    }
    string purchaseServer(int serverClass){
        string preferedModel = serverDictWithClass[serverClass].preferedModel();
        this->serversWithClass[serverClass].push_back(serverDict[preferedModel]);
        COST_HARDWARE += serverDict[preferedModel].cost_hardware;
        serverNumber++;
        return preferedModel;
    }
    vector<int> migrateVm(){
        return {};
    }
    void handleRequestsInOneDay(vector<Request> requestInfos){
        this->migrateVm();
        this->addInfo.clear();
        this->purchaseInfo.clear();
        
        // 处理一天的请求
        for(auto& r : requestInfos){
            char op = r.op;
            string model = r.model;
            int id = r.id;
            if(op == 'a'){ //创建虚拟机
                this->handleAddRequest(model, id);
            }
            else { // 删除虚拟机
                this->handleDelRequest(id);
            }
            for(auto& servers : this->serversWithClass){
                for(auto& server : servers){
                    assert(server.core[0] >= 0 && server.core[1] >= 0 &&
                            server.mem[0] >= 0 && server.mem[1] >= 0);
                }
            }
        }
        // 构造购买信息
        int n = purchaseInfo.size();
        // map<string, int>book;
        vector<PSI>book;
        if(n > 0){
            int res[n]; memset(res, -1, sizeof(res));
            int cnt = 0;
            for(int i = 0; i < n; i++){
                if(res[i] != -1) continue;
                res[i] = cnt++; 
                int j = i+1; int num = 1;
                for(; j < n; j++){
                    if(purchaseInfo[j].first == purchaseInfo[i].first){
                        res[j] = cnt++;  
                        num++;
                    }
                }
                book.push_back({purchaseInfo[i].first, num}); 
            }
            for(int i = 0; i < n; i++){
                auto& pi = purchaseInfo[i];
                Position pos = vmToServer[pi.second];
                this->serversWithClass[pos.serverClass][pos.serverIdx].id = this->serverNumber - n + res[i];
            }
            // for(int i = 0; i < addInfo.size(); i++){
            //     addInfo[i].serverId = this->serversWithClass[vmToServer[AA[i]].serverClass][vmToServer[AA[i]].serverIdx].id;
            // }
            for(auto& e : addInfo){
                Position& pos = vmToServer[e.vmId];
                e.serverId = this->serversWithClass[pos.serverClass][pos.serverIdx].id;
            }
        }
        
        buildOutputStream(book, this->migrateVm(), addInfo);
        this->calPowerCost();
    }
    void handleAddRequest(string model, int id){
        bool succeed = 0;
        int serverClass = vmDict[model].serverClass;
        vector<Server>& servers = this->serversWithClass[serverClass];
        for(int i = 0; i < servers.size(); i++){
            int node = servers[i].addVm(model, id);
            if(node != -1){
                vmToServer[id] = {serverClass, i, node};
                addInfo.push_back({this->serversWithClass[serverClass][i].id, node, id});
                succeed = 1;
                break;
            }
        }
        // 没有任何一个服务器可以容纳
        if(!succeed){
            string serverModel = this->purchaseServer(serverClass);
            this->purchaseInfo.push_back({serverModel, id});
            int node = this->serversWithClass[serverClass].back().
            addVm(model, id);

            addInfo.push_back({-1, node, id});
            vmToServer[id] = {serverClass, (int)(this->serversWithClass[serverClass].size())-1, node};
        }
    }
    void handleDelRequest(int id){
        Position pos = vmToServer[id];
        this->serversWithClass[pos.serverClass][pos.serverIdx].delVm(id, pos.serverNode);
    }
    void calPowerCost(){
        for(auto& servers : this->serversWithClass)
            for(auto& server : servers){
                if(!server.vms.empty()){
                    this->COST_POWER += server.powerCost;
                }
            }
    }
};

ServerResource serverResource;

void buildServerInfos(string model, string core, string mem, string cost1, string cost2){
    int i_core = stoi(core);
    int i_mem = stoi(mem);
    int i_cost1 = stoi(cost1);
    int i_cost2 = stoi(cost2);
    serverDict[model] = {i_core, i_mem, i_cost1, i_cost2, double(i_core)/i_mem};
}

void buildServerDictWithClass(){
    // 剔除服务器
    for(auto it = serverDict.begin(); it != serverDict.end();){
        bool ok = true;
        for(auto& vm : vmDict){
            if(vm.second.type == 1 && it->second.core < vm.second.core || 
                it->second.mem < vm.second.mem){
                    ok = false;
                    break;
                }else if(vm.second.type == 0 && it->second.core/2 < vm.second.core || 
                it->second.mem/2 < vm.second.mem){
                    ok = false;
                    break;
                }
        }
        if(!ok){
            serverDict.erase(it++);
        }else{
            it++;
        }
    }

    double minRatio = INT_MAX, maxRatio = INT_MIN;
    for(auto& kv : serverDict){
        minRatio = min(minRatio, kv.second.ratio);
        maxRatio = max(maxRatio, kv.second.ratio);
    }
    delta = (maxRatio-minRatio)/classNum;
    
    // build section
    double start = minRatio;
    for(int i = 0; i < classNum; i++){
        section[i] = start;
        start += delta;
    }
    
    // cout << minRatio << ' ' << maxRatio << ' ' << delta << endl;
    int ss = serverDict.size();
    serverDictWithClass.clear();
    for(int i = 0; i < classNum; i++){
        ServerSetWithClass sswc;
        for(auto& kv : serverDict){
            double ratio = kv.second.ratio;
            if(ratio >= section[i]-1e-6 && ratio <= section[i]+delta+1e-6){ ///////////////////////
                sswc._serverDict.push_back({kv.first, kv.second});
            }
        }
        serverDictWithClass.push_back(sswc);
    }
    // 构造了serverDictWithClass后，进行排序
    for(auto& e : serverDictWithClass){
        e.sortServerByRatio();
    }

    // for(auto& sswc : serverDictWithClass){
    //     for(auto& s : sswc._serverDict){
    //         cout << s.first << ' ' << s.second.ratio << endl;
    //     }
    //     cout << endl;
    // }
}

void buildVmToClass(){
    for(auto& vm : vmDict){
        if(vm.second.ratio < section[0]+delta){ // too small 
            vm.second.serverClass = 0;
        }else if(vm.second.ratio > section[classNum-1]){ // to big
            vm.second.serverClass = classNum-1;
        }else {
            bool ok = false;
            for(int i = 1; i < classNum-1; i++){
                
                if(vm.second.ratio >= section[i] && vm.second.ratio <= section[i]+delta && 
                    !serverDictWithClass[i]._serverDict.empty()){
                    vm.second.serverClass = i;
                    ok = true;
                    break;
                }
            }
            if(!ok){ // 意味着中间的这个区间没有服务器，此时考虑将它分配给最小类，这里可以优化***************************
                vm.second.serverClass = 0;
            }
        }
    }

}

void buildVmInfos(string model, string core, string mem, string isDouble){
    int i_core = stoi(core);
    int i_mem = stoi(mem);
    VirtualMachineInfo vm = {i_core, i_mem, isDouble[0]-'0', (double)i_core/i_mem};
    vmDict[model] = vm;
}


int main(){
    ios::sync_with_stdio(false);

#ifdef TEST //重定向
    freopen(inputFilePath.c_str(), "rb", stdin);
    freopen(outputFilePath.c_str(), "wb", stdout);
    // freopen(outputFilePath.c_str(), "ab", stdout);
#endif

    // 建立可购服务器表 
    int N; cin >> N; // 最大100
    for(int i = 0; i < N; i++){
        string model, core, mem, cost1, cost2;
        cin >> model >> core >> mem >> cost1 >> cost2;
        buildServerInfos(model.substr(1, model.size()-2), 
            core.substr(0, core.size()-1), mem.substr(0, mem.size()-1), 
            cost1.substr(0, cost1.size()-1), cost2.substr(0, cost2.size()-1));
    }

    // 建立可售虚拟机表
    int M; cin >> M; // 最大1000
    for(int i = 0; i < M; i++){
        string model, core, mem, isDouble;
        cin >> model >> core >> mem >> isDouble;
        buildVmInfos(model.substr(1, model.size()-2), 
            core.substr(0, core.size()-1), mem.substr(0, mem.size()-1), 
            isDouble.substr(0, isDouble.size()-1));
    }

    // 找到资源要求最高的那个虚拟机，剔除无法满足这种要求的服务器。*********或许可以只剔除vm对应的那个类别的服务器列表中资源太少的服务器？
    // 服务器按c/m分类
    buildServerDictWithClass();

    // 建立虚拟机到服务器类别的映射关系
    buildVmToClass();

    int T; cin >> T; // 最大1000 ，add操作不超过100000
    for(int i = 0; i < T; i++){
        int R; cin >> R;
        vector<Request> requestInfos;
        // 构造这天的请求信息
        for(int i = 0; i < R; i++){
            string op, model, id;
            cin >> op;
            if(op[1] == 'a') cin >> model >> id;
            else cin >> id;
            requestInfos.push_back(
                {op[1], model.substr(0, model.size()-1), stoi(id.substr(0, id.size()-1))}
            );
        }
        // 处理这天的请求
        serverResource.handleRequestsInOneDay(requestInfos);

        // cout << "\n\n\n天数：" << i << endl;
        // for(auto& server : serverResource.servers){
        //     cout << "A节点资源：" << server.core[0] << " " << server.mem[0] << " " <<  "B节点资源" << server.core[1] << " " <<  server.mem[1] << endl;
        // }
    }
#ifdef TEST
    cout << "COST_HARDWARE: " << serverResource.COST_HARDWARE << endl;
    cout << "COST_POWER: " << serverResource.COST_POWER << endl;
    cout << "COST: " << serverResource.COST_HARDWARE + serverResource.COST_POWER << endl;
#endif
    return 0;
}