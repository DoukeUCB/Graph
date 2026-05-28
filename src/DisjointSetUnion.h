#pragma once

#include <algorithm>
#include <vector>

class DisjointSetUnion {
public:
    explicit DisjointSetUnion(int n) : parent(n), size(n, 1) {
        for (int i = 0; i < n; ++i) parent[i] = i;
    }

    int find(int x) {
        while (parent[x] != x) {
            parent[x] = parent[parent[x]];
            x = parent[x];
        }
        return x;
    }

    bool unite(int a, int b) {
        int rootA = find(a);
        int rootB = find(b);
        if (rootA == rootB) return false;
        if (size[rootA] < size[rootB]) std::swap(rootA, rootB);
        parent[rootB] = rootA;
        size[rootA] += size[rootB];
        return true;
    }

private:
    std::vector<int> parent;
    std::vector<int> size;
};
