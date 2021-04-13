#include<iostream>
using namespace std;

class Position{
public:
    int x;
    int y = 999;
};

int main(){
    Position p;
    cout << p.x << ' ' << p.y << endl;

    return 0;
}