#include<iostream>
using namespace std;

class Position{
public:
    int x;
    int y = 2;
    int z;
};

int main(){
    Position p;
    cout << p.x << ' ' << p.y << p.z << endl;

    return 0;
}