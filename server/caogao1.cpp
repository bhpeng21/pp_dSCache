#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

int func(const string& s, const int& k)
{
    vector<vector<int>> dp(26, vector<int>(s.length() + 1, 0));
    
    for (char c : s)
    {
        int idx = c - 'a';
        int maxLen = 1;
        for (int i = max(0, idx - k); i < idx; i ++)
        {
//             maxLen = max(maxLen, dp[i] + 1);
            for (int j = 1; j < s.length(); j ++)
            {
                maxLen = max(maxLen, dp[i][j] + 1);
            }
        }
        dp[idx][maxLen] = max(dp[idx][maxLen], maxLen);
//         res = max(res, dp[idx][maxLen]);
    }
    int res = 0;
    for (const auto& row : dp)
    {
        res = max(res, *max_element(row.begin(), row.end()));
    }
    return res;
}

int main()
{
    string s = "aaaaa";
    int k = 1;
    // cin >> s >> k;

    cout << func(s, k) << endl;

    return 0;
}