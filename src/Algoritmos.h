#ifndef ALGORITMOS_H
#define ALGORITMOS_H

#include <vector>
#include <utility>
#include "Grafo.h"

/**
 * Clase Algoritmos: contiene todos los algoritmos de grafos como métodos estáticos.
 *
 * Cada método corresponde a un objetivo del proyecto:
 *   1. Alcance vehicular  → Dijkstra con corte
 *   2. Islas viales        → BFS para componentes débilmente conexas
 *   3. Diámetro vial       → Heurística double-sweep (Dijkstra iterado)
 *   4. MST                 → Kruskal con Union-Find
 *   5. Comparación rutas   → Dijkstra por distancia vs. por tiempo
 */
class Algoritmos {
public:

    // ================================================================
    // OBJETIVO 1: Alcance vehicular
    // ================================================================
    /**
     * Dado un nodo origen, calcula cuántos nodos son alcanzables
     * dentro de maxDistanciaMetros por distancia de calle.
     *
     * Algoritmo: Dijkstra con cola de prioridad, cortando exploración
     * cuando la distancia excede el límite.
     */
    static int calcularAlcanceVehicular(
        const Grafo& grafo, int nodoOrigen, double maxDistanciaMetros);

    // ================================================================
    // OBJETIVO 2: Islas viales (componentes débilmente conexas)
    // ================================================================
    struct ResultadoComponentes {
        int cantidadComponentes;           // Total de "islas" viales
        int tamanoComponenteGigante;       // Nodos en la componente más grande
        std::vector<int> nodosComponenteGigante; // IDs de nodos en la gigante
    };

    /**
     * Identifica componentes débilmente conexas usando BFS
     * sobre el grafo no dirigido.
     */
    static ResultadoComponentes encontrarComponentesConexas(const Grafo& grafo);

    // ================================================================
    // OBJETIVO 3: Diámetro vial
    // ================================================================
    struct ResultadoDiametro {
        int nodoA;               // Primer extremo del diámetro
        int nodoB;               // Segundo extremo del diámetro
        double distanciaMetros;  // Distancia mínima entre ellos
    };

    /**
     * Estima el diámetro de la componente gigante usando la
     * heurística "double sweep" (Dijkstra iterado).
     *
     * Heurística: desde un nodo, buscar el más lejano; desde ese,
     * buscar el más lejano de nuevo. Repetir 2-3 veces.
     */
    static ResultadoDiametro calcularDiametroVial(
        const Grafo& grafo, const std::vector<int>& componenteGigante);

    // ================================================================
    // OBJETIVO 4: Red de emergencia mínima (MST)
    // ================================================================
    /**
     * Construye el árbol de cobertura mínima (MST) de la componente
     * gigante usando el algoritmo de Kruskal.
     *
     * Retorna la distancia total del MST en metros.
     */
    static double construirMST(
        const Grafo& grafo, const std::vector<int>& componenteGigante);

    // ================================================================
    // OBJETIVO 5: Ruta por tipo de horario
    // ================================================================
    struct ResultadoRuta {
        std::vector<int> camino;     // Secuencia de nodos del camino
        double distanciaTotalMetros; // Distancia total en metros
        double tiempoTotalSegundos;  // Tiempo total en segundos
    };

    /**
     * Compara el camino más corto por distancia vs. por tiempo
     * entre un par de nodos. Ejecuta dos Dijkstra con pesos diferentes.
     *
     * Retorna: (rutaPorDistancia, rutaPorTiempo)
     */
    static std::pair<ResultadoRuta, ResultadoRuta> compararRutas(
        const Grafo& grafo, int nodoOrigen, int nodoDestino);

private:
    // --- Métodos auxiliares ---

    /**
     * Dijkstra genérico. Calcula distancia mínima desde el nodo origen.
     *
     * @param porTiempo   Si true, usa tiempo como peso; si false, usa distancia.
     * @param usarNoDirigido Si true, usa la adyacencia no dirigida.
     * @param nodoPrevio  (salida) Vector de nodo previo para reconstrucción de camino.
     * @return Vector de distancias/tiempos mínimas a cada nodo.
     */
    static std::vector<double> dijkstra(
        const Grafo& grafo, int nodoOrigen,
        bool porTiempo, bool usarNoDirigido,
        std::vector<int>& nodoPrevio);

    /**
     * Reconstruye el camino desde origen hasta destino usando nodoPrevio.
     */
    static std::vector<int> reconstruirCamino(
        int nodoOrigen, int nodoDestino,
        const std::vector<int>& nodoPrevio);

    /**
     * Calcula distancia y tiempo total de un camino dado.
     */
    static void calcularMetricasCamino(
        const Grafo& grafo, const std::vector<int>& camino,
        double& distanciaTotal, double& tiempoTotal);
};

#endif // ALGORITMOS_H
