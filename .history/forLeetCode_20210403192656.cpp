#include<iostream>
#include<vector>
#include<stack>
using namespace std;

class Solution {
public:

    int trap(vector<int>& height) {
        int n = height.size();
        stack<int>s;
        int ans = 0;
        for(int i = 0; i < n; i++){
            while(!s.empty() && height[i] > height[s.top()]){
                s.pop();
                int top = s.top();
                if(s.empty()) break;
                int left = s.top();
                int w = i - left - 1;
                int h = min(height[left], height[i]) - height[top];
                ans += w*h;
            }
            s.push(i);
        }
        return ans;
    }
};


int main(){
    Solution s;
    vector<int>h = {2,8,5,5,6,1,7,4,5};
    s.trap(h);
    return 0;
}