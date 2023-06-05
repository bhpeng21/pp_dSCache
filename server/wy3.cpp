#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

int func(int m, int n, int x, const vector<vector<int>>& arrs)
{
    vector<vector<int>> dp(m, vector<int>(n, 0));
    dp[0][0] = arrs[0][0];
    for (int i = 1; i < m; i ++)
    {
        dp[i][0] = dp[i - 1][0] + arrs[i][0];
    }
    for (int j = 1; j < n; j ++)
    {
        dp[0][j] = dp[0][j -1] + arrs[0][j];
    }
    for (int i = 1; i < m; i ++)
    {
        for (int j = 1; j < n; j ++)
        {
            dp[i][j] = min(dp[i - 1][j], dp[i][j - 1]) + arrs[i][j];
        }
    }

    int res = m + n - 2;
    for (int i = 0; i < m; i ++)
    {
        for (int j = 0; j < n; j ++)
        {
            if (dp[i][j] >= x) res = max(res, i + j);
        }
    }
    return res;
}