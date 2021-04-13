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
    int addedServersWithClassInOneDay[MAX_CLASS_NUM];
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