#ifndef ALGORITMOS_H
#define ALGORITMOS_H

#include <vector>
#include <utility>
#include "Grafo.h"

class Algoritmos {
public:

    static int calcularAlcanceVehicular(
        const Grafo& grafo, int nodoOrigen, double maxDistanciaMetros);

    struct ResultadoComponentes {
        int cantidadComponentes;
        int tamanoComponenteGigante;
        std::vector<int> nodosComponenteGigante;
    };

    static ResultadoComponentes encontrarComponentesConexas(const Grafo& grafo);

    struct ResultadoDiametro {
        int nodoA;
        int nodoB;
        double distanciaMetros;
    };

    static ResultadoDiametro calcularDiametroVial(
        const Grafo& grafo, const std::vector<int>& componenteGigante);

    static double construirMST(
        const Grafo& grafo, const std::vector<int>& componenteGigante);

    struct ResultadoRuta {
        std::vector<int> camino;
        double distanciaTotalMetros;
        double tiempoTotalSegundos;
    };

    static std::pair<ResultadoRuta, ResultadoRuta> compararRutas(
        const Grafo& grafo, int nodoOrigen, int nodoDestino);

private:
    static std::vector<double> dijkstra(
        const Grafo& grafo, int nodoOrigen,
        bool porTiempo, bool usarNoDirigido,
        std::vector<int>& nodoPrevio);

    static std::vector<int> reconstruirCamino(
        int nodoOrigen, int nodoDestino,
        const std::vector<int>& nodoPrevio);

    static void calcularMetricasCamino(
        const Grafo& grafo, const std::vector<int>& camino,
        double& distanciaTotal, double& tiempoTotal);
};

#endif
