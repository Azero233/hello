#include <iostream>
#include <vector>
#include <cstring>
using namespace std;

int         dp[32][4][2][2]; // pos, state, tight, matched
vector<int> binDigits;          // 从高位到低位，存放当前数的二进制形式

// 状态转移：当前 pos（对应binDigits，注意pos == 0时对应的是最高位），state：前缀匹配状态，tight：是否受限
// matched 表示是否已经匹配到 "101"，matched == true 表示之前已经遇到过 "101"，后面随便构造都合法
int dfs(int pos, int state, bool tight, bool matched)
{
    if (pos == binDigits.size())
    {
        return matched ? 1 : 0; // 只有匹配过 "101" 才是合法数
    }

    int& res = dp[pos][state][tight][matched];
    if (res != -1) return res;  // 初始状态为-1，这里表面当前状态已经搜索过了，直接返回搜索结果
    res = 0;                    // 未搜索过，初始化为0

    int maxDigit = tight ? binDigits[pos] : 1;  // 例如对于1010001，当前面的高位取了101时，第四位只能取0构成1010，不能取1011（1011xxxx > 1010001）

    for (int d = 0; d <= maxDigit; ++d) {
        int newState = state;
        // 状态转移逻辑
        if (state == 0) newState = (d == 1) ? 1 : 0;
        else if (state == 1) newState = (d == 0) ? 2 : 1;
        else if (state == 2) newState = (d == 1) ? 3 : 0;
        else if (state == 3) newState = 3; // 一直保持在 3 状态（已匹配）

        bool newMatched = matched || (newState == 3);
        // tight && (d == maxDigit) 确定后续dfs构造当前数时是否受限，lead && (d == 0) 判断后续构造时是否还在处理前导0
        res += dfs(pos + 1, newState, tight && (d == maxDigit), newMatched);
    }
    return res;
}

// 统计 [0, x] 中满足条件的数的个数
int countWith101(int x)
{
    binDigits.clear();
    while (x > 0)
    {
        // 将当前数字以二进制形式构造
        binDigits.push_back(x % 2);
        x /= 2;
    }
    reverse(binDigits.begin(), binDigits.end());
    memset(dp, -1, sizeof(dp));
    //return dfs(0, 0, true, true, false);
    return dfs(0, 0, true, false);
}

int countInRange(int L, int R)
{
    return countWith101(R) - countWith101(L - 1);
}

// 下面的函数是暴力判断，用于参照检查功能是否正确
//bool hasNoConsecutive101(int n)
//{
//    // 将输入转换为无符号整数以避免负数右移问题
//    unsigned int x = static_cast<unsigned int>(n);
//
//    // 当x小于5（二进制101）时，不可能存在连续的101
//    while (x >= 5) {
//        // 检查最低三位是否等于101（即5）
//        if ((x & 7) == 5) { // 7的二进制是111，用于取最低三位
//            return false;
//        }
//        x >>= 1; // 右移一位继续检查
//    }
//    return true;
//}
//
//int count101(int L, int R)
//{
//    int ret = 0;
//    for (int x = L; x <= R; ++x)
//    {
//        if (hasNoConsecutive101(x))
//        {
//            ++ret;
//        }
//    }
//    return ret;
//}

int main()
{
    int L, R;
    cout << "请输入区间 L 和 R（例如 10 1000000000）: ";
    cin >> L >> R;

    if (L > R) {
        cerr << "错误：L 不能大于 R！" << endl;
        return 1;
    }

    int result = R - L + 1 - countInRange(L, R);
    cout << "区间 [" << L << ", " << R << "] 中不含 \"101\" 的数的个数为: " << result << endl;

    //int result2 = count101(L, R);
    //cout << "区间 [" << L << ", " << R << "] 中不含 \"101\" 的数的个数为: " << result2 << endl;

    return 0;
}
