#include<iostream>
#include<string>
#include<vector>
#include<unordered_map>
#include<unordered_set>
#include<sstream>
#include<cassert>
#include<cstring>
#include<algorithm>
#include<ctime>
#include <chrono>
using namespace std;

#define TEST
int testedFile = 2;


// #define ONE_KIND_ALL_DAYS // 所有天固定一种
#define ONE_KIND_PER_K_DAYS // 每个K天固定一种
// #define USE_vmToMinPowerCostServer // 买最便宜
// #define USE_vmToMinCostServer // 买启发代价最小，效果不好，需要调参

/* 
    迁移策略：0-continue
             1-continue + break 
             2-连续4次失败退出
             3-累计20次失败退出
*/
int migrationVersion = 3;
int cnt_fail_thr = 50;
double thr = 0.2; 

/*
    1. 改进：增加迁移次数
    2. 可以调的参数：myRatio、找c/m最接近的那个服务器时的阈值、getCost3中的alpha、迁移时早停参数
    3. 潜在的问题：bestFit和bestFit2为啥又区别？前者略慢一点，但成本略小一点
*/

const int INF = 0x3f3f3f3f;
const char* AB[] = {"A", "B"};
const string inputFilePaths[3] = {" ","training-data/training-1.txt", "training-data/training-2.txt"};
string inputFilePath = inputFilePaths[testedFile];
const string outputFilePath = "output.txt";
double myRatio = 1.0;
string serverModel; // hostATVAZ hostDPJA1 host7SRCB

int deployVersion = 0; // 

const int maxT = 1000;
int T, K; 

typedef pair<int, int>PII;
typedef pair<string, int>PSI;
struct Tuple{
    int serverId, node, vmId;
};

struct ServerInfo{
    int core, mem;
    int cost_hardware, cost_power;
    string model;
    double cm;
};
struct VirtualMachineInfo{
    int core, mem, type;
    double cm;
};

struct Position{
    int idx, node;
};

struct MigrateInfo{
    int vm_id,server_id, node;
};

unordered_map<string, ServerInfo>serverDict;
unordered_map<string, ServerInfo>bigServerDict; ////////////////////////////////// 可能不好！！！！！！！！
vector<ServerInfo>sortedServerList; // 按照电费排序的服务器列表
unordered_map<string, VirtualMachineInfo>vmDict;

unordered_map<int, Position>vmToServer;

unordered_map<string, string>vmToMinPowerCostServer;
unordered_map<string, string>vmToMinCostServer[maxT];

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
    double getTotalLeft(){
        return (this->core[0] + this->core[1])*myRatio +
                (this->mem[0] + this->mem[1]);
    }
    double getTotalUsed(){
        return this->capacity_core + this->capacity_mem - this->getTotalLeft();
    }

    // 插入后的平衡性 ?????????直接作差似乎有问题??????????????????????????
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
    // 剩下的的资源总量
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
    // 平衡性 + alpha*所剩资源 ????????????? alpha可作为参数 ???????????????????????????????
    bool getCost3(VirtualMachineInfo& vm, PII& res){
        if(vm.type == 0){ // 单结点虚拟机
            int minCost = INF, minNode = -1;
            for(int node = 0; node < 2; node++){
                if(core[node] >= vm.core && mem[node] >= vm.mem){
                    int cost1 = abs((core[node] - vm.core)*myRatio - (mem[node] - vm.mem));
                    int cost2 = this->getTotalLeft();
                    int cost = cost1 + cost2;
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
            int cost1 = abs((core[0] - vm.core/2 + core[1] - vm.core/2)*myRatio - 
                            (mem[0] - vm.mem/2 + mem[1] - vm.mem/2));
            int cost2 = this->getTotalLeft();
            int cost = cost1 + cost2;
            res = {2, cost};
            return true;
        }
        return false;
    }
    
// 未知插入节点************************    
    // A B节点无区别插;
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
    // A B节点有选择地插 ???????????????? 如何做选择，值得再考虑????????????????????????
    int addVm2(VirtualMachineInfo& vm, int id){
        PII res;
        int ok = this->getCost(vm, res);
        if(!ok) return -1;
        this->addVm(vm, id, res.first);
        return res.first;
    }
    
// 已知插入节点*************************
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
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// %%%%%%%%%%%%%%%%%%%% debug用 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    void printInfo(){
        cout << "A: " << core[0] << ' ' << mem[0] << endl;
        cout << "B: " << core[1] << ' ' << mem[1] << endl;
        cout << "capacity_core: " << capacity_core << " capacity_mem: " << capacity_mem << endl; 
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
    unordered_set<int>emptyServers, notEmptyServers;
    long long COST_HARDWARE, COST_POWER;

    vector<vector<Request>>allRequests; int currentDay;
    vector<double>totalCoreOfEachDay, totalMemOfEachDay;
    double totalCoreInKDays, totalMemInKDays;
    vector<PSI>purchaseInfo; // (server_model, vm_id)
    vector<MigrateInfo>migrationInfo;
    vector<Tuple>addInfo; // serverId, node, vmId;
    vector<int>idxForAddInfo;
    vector<PSI>addRequests;
    int totalVm = 0;
    int delNumInLastDay;
    // int totalMigration = 0;

    ServerResource(){
        servers.clear();
        COST_HARDWARE = COST_POWER = 0;
        this->totalVm = 0;
        this->currentDay = 0;
        totalCoreInKDays = totalMemInKDays = 0;
    }
    
    void getRequestsInTheFirstKDays(vector<Request>requestsInTheFirstKDays[]){
        for(int i = 0; i < K; i++){
            allRequests.push_back(requestsInTheFirstKDays[i]);
            double totalCore = 0, totalMem = 0;
            for(auto& req : allRequests.back()){
                if(req.op == 'a'){
                    VirtualMachineInfo& vm = vmDict[req.model];
                    totalCore += vm.core;
                    totalMem += vm.mem; 
                }
            }
            this->totalCoreOfEachDay.push_back(totalCore);
            this->totalMemOfEachDay.push_back(totalMem);
            this->totalCoreInKDays += totalCore;
            this->totalMemInKDays += totalMem;
        }
        this->handleRequestsInOneDay();
    }
    void getRequestInOneDay(vector<Request>&requests){
        allRequests.push_back(requests);
        double totalCore = 0, totalMem = 0;
        for(auto& req : allRequests.back()){
            if(req.op == 'a'){
                VirtualMachineInfo& vm = vmDict[req.model];
                totalCore += vm.core;
                totalMem += vm.mem; 
            }
        }
        this->totalCoreOfEachDay.push_back(totalCore);
        this->totalMemOfEachDay.push_back(totalMem);

        this->totalCoreInKDays += totalCore;
        this->totalMemInKDays += totalMem;
        this->totalCoreInKDays -= this->totalCoreOfEachDay[this->currentDay-1];
        this->totalMemInKDays -= this->totalMemOfEachDay[this->currentDay-1];
        
        this->handleRequestsInOneDay();
    }
    void handleLeftRequests(){
        while(currentDay < T){
            this->totalCoreInKDays -= this->totalCoreOfEachDay[this->currentDay-1];
            this->totalMemInKDays -= this->totalMemOfEachDay[this->currentDay-1];
            this->handleRequestsInOneDay();
        }
    }
    
    // 末尾this->currentDay++;
    void handleRequestsInOneDay(){ 
        // 处理的是第currentDay天的请求，但是能够看到后面K（最后小于K）天。
        // 即能够看到 requestsInKDays[currentDay...-1]
        vector<Request>& requests = this->allRequests[currentDay];
#ifdef ONE_KIND_PER_K_DAYS
        myRatio = this->totalCoreInKDays/this->totalMemInKDays;
        auto getBestServerModel = [&]()->string{
            // 找到比例和myRatio较为接近且相对便宜并且能够容纳所有虚拟机的服务器
            //  //////////////////// 注意：目前实际上是找再阈值之内的，或出错
            ///////////////////////////////////////////////////////////////
            double minCost = INF; string model;
            for(auto& server : bigServerDict){
                if(abs(myRatio-server.second.cm) > ratio_thr) continue;
                double cost = server.second.cost_hardware + (T-currentDay)*server.second.cost_power; //////////////////
                if(cost < minCost){
                    minCost = cost;
                    model = server.first;
                }
            }
            return model;
        };
        serverModel = getBestServerModel();
#endif

#ifdef TEST
        for(auto& s : emptyServers){
            if(!servers[s].vms.empty()){
                assert(servers[s].vms.empty());
            }
        }
        for(auto& s : notEmptyServers){
            assert(!servers[s].vms.empty());
        }
#endif
        this->addInfo.clear();
        this->purchaseInfo.clear();
        this->idxForAddInfo.clear();
        this->addRequests.clear();
        this->migrationInfo.clear();

// #ifdef TEST
//         cout << "before migration: " << this->getEmptyNumber() << endl;
// #endif
        this->migrateByVersion(migrationVersion);
// #ifdef TEST
//         cout << "after migration: " << this->getEmptyNumber() << endl;
//         cout << this->servers.size() << endl;
//         this->printServers(); cout << endl;
//         cout << "############################################################\n";
// #endif
        int n = requests.size();
        int cnt_add = 0;
        this->delNumInLastDay = 0;
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
                this->delNumInLastDay++;
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
                this->servers[pos.idx].id = this->servers.size() - n + res[i];
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
        this->currentDay++;
    }

    string purchaseAsRequired(string vmModel){
        VirtualMachineInfo& vm = vmDict[vmModel];
#ifdef ONE_KIND_PER_K_DAYS
        string preferedModel = serverModel;
#endif

#ifdef ONE_KIND_ALL_DAYS
        string preferedModel = serverModel;
#endif

#ifdef USE_vmToMinPowerCostServer
        string preferedModel = vmToMinPowerCostServer[vmModel];
#endif

#ifdef USE_vmToMinCostServer
        string preferedModel = vmToMinCostServer[this->currentDay][vmModel];
#endif
        ServerInfo server = serverDict[preferedModel];
        this->servers.push_back(server);
#ifdef TEST
        COST_HARDWARE += server.cost_hardware;
#endif
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
            case 3:
                this->migrate3();
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
    // 无早停
    void migrate0(){
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
                if(curMigrationNum+1 > limit) break;
            }
        };
        return;
    }
    // 配合 continue + break
    void migrate1(){
        int n = this->servers.size();
        int total = this->totalVm;
        int limit = 3*total/100, curMigrationNum = 0;
        
        ///////////////////////////////////////////////////////////////////////////////// first
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
            if(curMigrationNum+1 > limit) break;
        }

        /////////////////////////////////////////////////////////////////////////////// second
        m = this->emptyServers.size(); // 空的服务器个数，迁移的时候不考虑空服务器
#ifdef TEST
    assert(emptyServers.size() + notEmptyServers.size() == n);
#endif
        for(int i = 0; i < n; i++) idx[i] = i;
        sort(idx, idx + n, [&](int x, int y){ //////////////////////////// 或需要修改
            return this->servers[x].getTotalLeft() > this->servers[y].getTotalLeft();
            
            // return this->servers[x].getTotalUsed() < this->servers[y].getTotalUsed();    
                // return this->servers[x].vms.size() < this->servers[y].vms.size();
        });
        
        end = m + (n-m)*2/3;
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
                    // continue ;
                    break;
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
            if(curMigrationNum+1 > limit) break;
        }
        return;
    }
    // 连续4次失败退出
    void migrate2(){
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
            bool canMigrate[n]; memset(canMigrate, 0, sizeof(canMigrate));
            for(int i = m; i < end; i++){
                if(i > m+4 && !canMigrate[i] && !canMigrate[i-1] && !canMigrate[i-2] && !canMigrate[i-3]) break;
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
                        // break;
                    }else{ // 可以迁移
                        // server.delVm(vm.first, node, false); // 不能在这里删除！！！！！
                        canMigrate[i] = true;
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
                if(curMigrationNum+1 > limit) break;
            }
        };
        migrate();
        migrate();
        migrate();
        return;
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
            for(int i = m; i < end; i++){
                if(cnt_fail > cnt_fail_thr) break;
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
                if(curMigrationNum+1 > limit) break;
            }
        };
        migrate();
        migrate();
        migrate();
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
            this->emptyServers.erase(idx2);
            this->notEmptyServers.insert(idx2);
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
        // assert(idx == bestFit(model, id, minRes));
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
        if(this->servers[pos.idx].vms.empty()){
            this->emptyServers.insert(pos.idx);
            this->notEmptyServers.erase(pos.idx);
        }
        vmToServer.erase(id); ////////////////////
    }
    
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// %%%%%%%%%%%%%%%%%%%% debug用 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    void calPowerCost(){
        for(auto& server : this->servers){
            if(!server.vms.empty()){
                this->COST_POWER += server.powerCost;
            }
        }
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
};

ServerResource serverResource;

void readServerInfos(string model, string core, string mem, string cost1, string cost2){
    int i_core = stoi(core);
    int i_mem = stoi(mem);
    int i_cost1 = stoi(cost1);
    int i_cost2 = stoi(cost2);
    serverDict[model] = {i_core, i_mem, i_cost1, i_cost2, model, (double)i_core/i_mem};
    sortedServerList.push_back({i_core, i_mem, i_cost1, i_cost2, model});
}
void readVmInfos(string model, string core, string mem, string isDouble){
    int i_core = stoi(core);
    int i_mem = stoi(mem);
    vmDict[model] = {i_core, i_mem, isDouble[0]-'0', (double)i_core/i_mem};
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
#ifdef ONE_KIND_ALL_DAYS
    serverModel = N == 80 ? "hostDPJA1" : "hostVMUHX";
    myRatio = N == 80 ? 1.5 : 0.3;
#endif
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


// 买最便宜的服务器预处理
void buildVmToServerWithMinPowerCost(){
    sort(sortedServerList.begin(), sortedServerList.end(), [](ServerInfo& x, ServerInfo& y){
        return x.cost_power < y.cost_power;
    });
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

// 买代价最小的服务器预处理 (vmModel, 天数) -> 服务器 注意，这里没有考虑比例
void buildVmToServerWithMinCost(){
    auto check = [](VirtualMachineInfo& vm, ServerInfo& server){
        if(vm.type == 0){
            return vm.core <= server.core/2 && vm.mem <= server.mem/2;
        }else{
            return vm.core <= server.core && vm.mem <= server.mem;
        }
    };
    for(auto& vm : vmDict){
        for(int t = 0; t < maxT; t++){
            double minCost = INF;
            string model;
            for(auto& server : serverDict){
                if(!check(vm.second, server.second)) continue;
                double cost = server.second.cost_hardware + (T-t)*server.second.cost_power;
                if(cost < minCost){
                    minCost = cost;
                    model = server.first;
                }
            }
            vmToMinCostServer[t][vm.first] = model;
        }
    }
}

// 得到可以容纳所有类型的虚拟机的服务器。在使用[K天固定一种服务器]的方案时，调用此函数
void getBigServerDict(){
    auto check = [](VirtualMachineInfo& vm, ServerInfo& server){
        if(vm.type == 0){
            return vm.core <= server.core/2 && vm.mem <= server.mem/2;
        }else{
            return vm.core <= server.core && vm.mem <= server.mem;
        }
    };
    for(auto& server : serverDict){

    }
    for(auto& server : serverDict){
        // vmToMinPowerCostServer[vm.first] = serverModel;
        bool ok = true;
        for(auto& vm : vmDict){
            if(!check(vm.second, server.second)){
                ok = false;
                break;
            }
        }
        if(ok){
            bigServerDict.insert(server);
        }
    }
}

void solve(){
    cin >> T >> K; // 最大1000 ，add操作不超过100000
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
    serverResource.getRequestsInTheFirstKDays(requestsInTheFirstKDays);
    for(int t = K; t < T; t++){
        vector<Request>requestInfos;
        readOneDay(requestInfos);
        serverResource.getRequestInOneDay(requestInfos);
    }
    serverResource.handleLeftRequests();
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
#ifdef USE_vmToMinPowerCostServer
    buildVmToServerWithMinPowerCost();
#endif
#ifdef USE_vmToMinCostServer
    buildVmToServerWithMinCost();
#endif
#ifdef ONE_KIND_PER_K_DAYS
    getBigServerDict();
#endif
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
    return 0;
}