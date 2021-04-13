#include<iostream>
#include<vector>
using namespace std;

class Solution {
public:

    int trap(vector<int>& height) {
        // 找到所有极大值点
        int n = height.size(); 
        vector<int>maxPoints1;
        int i = 0;
        while(i+1 < n){
            // up
            while(i+1 < n && height[i+1] > height[i]) {
                i++;
            }
            maxPoints1.push_back(i);
            // down
            while(i+1 < n && height[i+1]  <= height[i]){
                i++;
            }
        }
        for(auto p : maxPoints1){
            cout << p << endl;
        }
        // vector<int>maxPoints2;
        // i = 0, n = maxPoints1.size();
        // while(i+1 < n){
        //     // up
        //     while(i+1 < n && height[maxPoints1[i+1]] > height[maxPoints1[i]]) {
        //         i++;
        //     }
        //     maxPoints2.push_back(i);
        //     // down
        //     while(i+1 < n && height[maxPoints1[i+1]]  <= height[maxPoints1[i]]){
        //         i++;
        //     }
        // }
        // for(auto p : maxPoints2){
        //     cout << p << endl;
        // }
        cout << endl;
        int ans = 0;
        n = maxPoints1.size();
        for(int i = 0; i < n-1;){
            int j = i+1;
            while(j < n && height[maxPoints1[j]] < height[maxPoints1[i]]) j++;
            cout << j << endl;
            if(j == n) j = i+1;
            int left = maxPoints1[i], right = maxPoints1[j];
            int H = min(height[left], height[right]);
            int sum = 0;
            for(int k = left+1; k < right; k++){
                sum += max(0, H - height[k]);
            }
            ans += sum;
            i = j;
        }
        return ans;
    }
};


int main(){
    Solution s;
    vector<int>h = {2,8,5,5,6,1,7,4,5};
    s.trap(s);
    return 0;
}