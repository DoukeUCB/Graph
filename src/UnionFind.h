#ifndef UNION_FIND_H
#define UNION_FIND_H

#include <vector>

/**
 * Clase UnionFind (Conjunto Disjunto):
 * Permite agrupar elementos en conjuntos y verificar si dos elementos
 * pertenecen al mismo conjunto de manera eficiente.
 *
 * Optimizaciones:
 *   - Compresión de camino en encontrar(): hace que todos los nodos
 *     apunten directamente a la raíz, acelerando futuras consultas.
 *   - Unión por rango en unir(): el árbol más bajo se une bajo el más alto,
 *     manteniendo el árbol balanceado.
 *
 * Complejidad amortizada: O(α(n)) por operación, donde α es la función
 * inversa de Ackermann (prácticamente O(1)).
 */
class UnionFind {
private:
    std::vector<int> padre;
    std::vector<int> rango;

public:
    /**
     * Constructor: inicializa n elementos, cada uno es su propio conjunto.
     */
    UnionFind(int n) : padre(n), rango(n, 0) {
        for (int i = 0; i < n; i++) {
            padre[i] = i;
        }
    }

    /**
     * Encuentra la raíz (representante) del conjunto al que pertenece x.
     * Aplica compresión de camino: cada nodo visitado apunta directamente a la raíz.
     */
    int encontrar(int x) {
        if (padre[x] != x) {
            padre[x] = encontrar(padre[x]); // Compresión de camino
        }
        return padre[x];
    }

    /**
     * Une los conjuntos de x e y.
     * Retorna true si estaban en conjuntos diferentes (unión exitosa).
     * Retorna false si ya estaban en el mismo conjunto.
     */
    bool unir(int x, int y) {
        int raizX = encontrar(x);
        int raizY = encontrar(y);

        if (raizX == raizY) return false; // Ya están en el mismo conjunto

        // Unión por rango: el árbol más bajo se une bajo el más alto
        if (rango[raizX] < rango[raizY]) {
            std::swap(raizX, raizY);
        }
        padre[raizY] = raizX;

        if (rango[raizX] == rango[raizY]) {
            rango[raizX]++;
        }
        return true;
    }

    /**
     * Verifica si x e y están en el mismo conjunto.
     */
    bool estanConectados(int x, int y) {
        return encontrar(x) == encontrar(y);
    }
};

#endif // UNION_FIND_H
