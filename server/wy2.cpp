#include <iostream>
#include <unordered_map>
#include <string>

using namespace std;

struct TreeNode{
    int val;
    TreeNode* left;
    TreeNode* right;
    TreeNode(int x) : val(x), left(nullptr), right(nullptr) {}
}

class Solution{
public:
    int count;
    string init(TreeNode* root, unordered_map<string , int>& subtreeCnt)
    {
        if (!root) return "X";
        string done = "(" + init(root->left, subtreeCnt) + ")" + to_string(root->val) + "(" +init(root->right, subtreeCnt) + ")";
        subtreeCnt[done] ++;
        if (subtreeCnt[done] >= 2){
            count ++;
        }
        return done;
    }
    int findDup(TreeNode* root)
    {
        count = 0;
        unordered_map<string, int> subtreeCnt;
        init(root, subtreeCnt);
        return count;
    }
    TreeNode* deserial(const string& s, int& pos)
    {
        if (s[pos] == 'X')
        {
            pos++;
            return nullptr;
        }
        pos ++;
        TreeNode* left = deserial(s, pos);
        pos ++;

        int val = 0;
        while(isdigit(s[pos])){
            val = val * 10 + (s[pos] - '0');
            pos ++;
        }
        pos ++;
        TreeNode* right = deserial(s, pos);
        pos ++;

        TreeNode* node = new TreeNode(val);
        node->left = left;
        node->right = right;
        return node;
    }
    TreeNode* deserial(const string& s)
    {
        int pos = 0;
        return deserial(s, pos);
    }
private:
};