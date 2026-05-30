#ifndef UNION_FIND_H
#define UNION_FIND_H

#include <vector>

class UnionFind {
private:
    std::vector<int> padre;
    std::vector<int> rango;

public:
    UnionFind(int n) : padre(n), rango(n, 0) {
        for (int i = 0; i < n; i++) {
            padre[i] = i;
        }
    }

    int encontrar(int x) {
        if (padre[x] != x) {
            padre[x] = encontrar(padre[x]);
        }
        return padre[x];
    }

    bool unir(int x, int y) {
        int raizX = encontrar(x);
        int raizY = encontrar(y);

        if (raizX == raizY) return false;

        if (rango[raizX] < rango[raizY]) {
            std::swap(raizX, raizY);
        }
        padre[raizY] = raizX;

        if (rango[raizX] == rango[raizY]) {
            rango[raizX]++;
        }
        return true;
    }

    bool estanConectados(int x, int y) {
        return encontrar(x) == encontrar(y);
    }
};

#endif
