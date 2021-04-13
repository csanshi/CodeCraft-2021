#include<iostream>
using namespace std;

class Position{
public:
    int x;
    int y = 2;
};

int main(){
    Position p;
    cout << p.x << ' ' << p.y << endl;

    return 0;
}