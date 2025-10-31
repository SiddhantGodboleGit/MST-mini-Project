#include <iostream>
#include <vector>
#include <algorithm>
#include <climits>

using namespace std;

//best performing  -- Gemini 1/2/2024
// Function to calculate the highest possible score
int maxStoneJumpScore(const vector<int>& stones) {
    int n = stones.size();
    if (n <= 1) return 0;
    
    vector<long long> dp(n, LLONG_MIN);
    dp[0] = 0;

    for (int i = 1; i < n; ++i) {
        // Optimization: start from the most recent reachable stone
        for (int j = i - 1; j >= 0; --j) {
            if (dp[j] != LLONG_MIN) {
                int jumpLength = i - j;
                long long score = dp[j] + (long long)jumpLength * stones[i];
                dp[i] = max(dp[i], score);
            }
        }
    }

    return dp[n - 1] == LLONG_MIN ? 0 : (int)dp[n - 1];
}

int main() {
    vector<int> stones = {0, 2, 3, 5, 4 , 5 , 10 , 2 , 1};
    cout << "Maximum Score: " << maxStoneJumpScore(stones) << endl;
    
    return 0;
}