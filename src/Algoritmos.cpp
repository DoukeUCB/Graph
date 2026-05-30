#include "Algoritmos.h"
#include "UnionFind.h"
#include <queue>
#include <algorithm>
#include <iostream>
#include <limits>
#include <unordered_set>
#include <functional>

static const double INFINITO = std::numeric_limits<double>::infinity();

std::vector<double> Algoritmos::dijkstra(
    const Grafo& grafo, int nodoOrigen,
    bool porTiempo, bool usarNoDirigido,
    std::vector<int>& nodoPrevio)
{
    int totalNodos = grafo.obtenerCantidadNodos();

    std::vector<double> distancia(totalNodos, INFINITO);
    nodoPrevio.assign(totalNodos, -1);
    std::vector<bool> procesado(totalNodos, false);

    using ParDistanciaNodo = std::pair<double, int>;
    std::priority_queue<ParDistanciaNodo,
                        std::vector<ParDistanciaNodo>,
                        std::greater<ParDistanciaNodo>> colaPrioridad;

    distancia[nodoOrigen] = 0.0;
    colaPrioridad.push(std::make_pair(0.0, nodoOrigen));

    while (!colaPrioridad.empty()) {
        ParDistanciaNodo tope = colaPrioridad.top();
        colaPrioridad.pop();
        double distanciaActual = tope.first;
        int nodoActual = tope.second;

        if (procesado[nodoActual]) continue;
        procesado[nodoActual] = true;

        const auto& vecinos = usarNoDirigido
            ? grafo.obtenerVecinosNoDirigidos(nodoActual)
            : grafo.obtenerVecinosDirigidos(nodoActual);

        for (const auto& vecino : vecinos) {
            if (procesado[vecino.nodoDestino]) continue;

            double peso = porTiempo ? vecino.tiempoSegundos : vecino.distanciaMetros;
            double nuevaDistancia = distanciaActual + peso;

            if (nuevaDistancia < distancia[vecino.nodoDestino]) {
                distancia[vecino.nodoDestino] = nuevaDistancia;
                nodoPrevio[vecino.nodoDestino] = nodoActual;
                colaPrioridad.push(std::make_pair(nuevaDistancia, vecino.nodoDestino));
            }
        }
    }

    return distancia;
}

std::vector<int> Algoritmos::reconstruirCamino(
    int nodoOrigen, int nodoDestino,
    const std::vector<int>& nodoPrevio)
{
    std::vector<int> camino;

    if (nodoPrevio[nodoDestino] == -1 && nodoDestino != nodoOrigen) {
        return camino;
    }

    for (int nodo = nodoDestino; nodo != -1; nodo = nodoPrevio[nodo]) {
        camino.push_back(nodo);
    }

    std::reverse(camino.begin(), camino.end());
    return camino;
}

void Algoritmos::calcularMetricasCamino(
    const Grafo& grafo, const std::vector<int>& camino,
    double& distanciaTotal, double& tiempoTotal)
{
    distanciaTotal = 0.0;
    tiempoTotal    = 0.0;

    for (size_t i = 0; i + 1 < camino.size(); i++) {
        int actual   = camino[i];
        int siguiente = camino[i + 1];

        const auto& vecinos = grafo.obtenerVecinosDirigidos(actual);
        double mejorDist = INFINITO;
        double mejorTiempo = INFINITO;

        for (const auto& vecino : vecinos) {
            if (vecino.nodoDestino == siguiente) {
                if (vecino.distanciaMetros < mejorDist) {
                    mejorDist   = vecino.distanciaMetros;
                    mejorTiempo = vecino.tiempoSegundos;
                }
            }
        }

        if (mejorDist < INFINITO) {
            distanciaTotal += mejorDist;
            tiempoTotal    += mejorTiempo;
        }
    }
}

int Algoritmos::calcularAlcanceVehicular(
    const Grafo& grafo, int nodoOrigen, double maxDistanciaMetros)
{
    int totalNodos = grafo.obtenerCantidadNodos();
    std::vector<double> distancia(totalNodos, INFINITO);
    std::vector<bool> procesado(totalNodos, false);

    using ParDistanciaNodo = std::pair<double, int>;
    std::priority_queue<ParDistanciaNodo,
                        std::vector<ParDistanciaNodo>,
                        std::greater<ParDistanciaNodo>> colaPrioridad;

    distancia[nodoOrigen] = 0.0;
    colaPrioridad.push(std::make_pair(0.0, nodoOrigen));

    int nodosAlcanzables = 0;

    while (!colaPrioridad.empty()) {
        ParDistanciaNodo tope = colaPrioridad.top();
        colaPrioridad.pop();
        double distanciaActual = tope.first;
        int nodoActual = tope.second;

        if (procesado[nodoActual]) continue;

        if (distanciaActual > maxDistanciaMetros) break;

        procesado[nodoActual] = true;
        nodosAlcanzables++;

        for (const auto& vecino : grafo.obtenerVecinosDirigidos(nodoActual)) {
            if (procesado[vecino.nodoDestino]) continue;

            double nuevaDist = distanciaActual + vecino.distanciaMetros;

            if (nuevaDist < distancia[vecino.nodoDestino]) {
                distancia[vecino.nodoDestino] = nuevaDist;
                colaPrioridad.push(std::make_pair(nuevaDist, vecino.nodoDestino));
            }
        }
    }

    return nodosAlcanzables;
}

Algoritmos::ResultadoComponentes Algoritmos::encontrarComponentesConexas(
    const Grafo& grafo)
{
    int totalNodos = grafo.obtenerCantidadNodos();
    std::vector<int> componenteDeNodo(totalNodos, -1); // -1 = no visitado
    int cantidadComponentes = 0;
    int tamanoGigante = 0;
    int idComponenteGigante = -1;

    for (int nodoInicial = 0; nodoInicial < totalNodos; nodoInicial++) {
        if (componenteDeNodo[nodoInicial] != -1) continue;

        if (grafo.obtenerVecinosNoDirigidos(nodoInicial).empty()) continue;

        std::queue<int> cola;
        cola.push(nodoInicial);
        componenteDeNodo[nodoInicial] = cantidadComponentes;
        int tamanoComponente = 0;

        while (!cola.empty()) {
            int nodoActual = cola.front();
            cola.pop();
            tamanoComponente++;

            for (const auto& vecino : grafo.obtenerVecinosNoDirigidos(nodoActual)) {
                if (componenteDeNodo[vecino.nodoDestino] == -1) {
                    componenteDeNodo[vecino.nodoDestino] = cantidadComponentes;
                    cola.push(vecino.nodoDestino);
                }
            }
        }

        if (tamanoComponente > tamanoGigante) {
            tamanoGigante = tamanoComponente;
            idComponenteGigante = cantidadComponentes;
        }

        cantidadComponentes++;
    }

    std::vector<int> nodosGigante;
    nodosGigante.reserve(tamanoGigante);
    for (int i = 0; i < totalNodos; i++) {
        if (componenteDeNodo[i] == idComponenteGigante) {
            nodosGigante.push_back(i);
        }
    }

    ResultadoComponentes resultado;
    resultado.cantidadComponentes = cantidadComponentes;
    resultado.tamanoComponenteGigante = tamanoGigante;
    resultado.nodosComponenteGigante = nodosGigante;
    return resultado;
}

Algoritmos::ResultadoDiametro Algoritmos::calcularDiametroVial(
    const Grafo& grafo, const std::vector<int>& componenteGigante)
{
    if (componenteGigante.empty()) {
        ResultadoDiametro vacio; vacio.nodoA = -1; vacio.nodoB = -1; vacio.distanciaMetros = 0.0;
        return vacio;
    }

    std::unordered_set<int> enComponente(componenteGigante.begin(),
                                          componenteGigante.end());

    int nodoActual = componenteGigante[0];
    int mejorNodoA = nodoActual;
    int mejorNodoB = nodoActual;
    double mejorDistancia = 0.0;

    const int ITERACIONES = 3;
    std::vector<int> previos;

    for (int iter = 0; iter < ITERACIONES; iter++) {
        std::cout << "    Iteracion " << (iter + 1) << "/" << ITERACIONES
                  << " - Dijkstra desde nodo " << nodoActual << "..." << std::flush;

        std::vector<double> distancias = dijkstra(
            grafo, nodoActual, false, true, previos);

        int nodoMasLejano = nodoActual;
        double distanciaMaxima = 0.0;

        for (int nodo : componenteGigante) {
            if (distancias[nodo] < INFINITO && distancias[nodo] > distanciaMaxima) {
                distanciaMaxima = distancias[nodo];
                nodoMasLejano = nodo;
            }
        }

        std::cout << " nodo mas lejano: " << nodoMasLejano
                  << " (dist: " << distanciaMaxima / 1000.0 << " km)" << std::endl;

        if (distanciaMaxima > mejorDistancia) {
            mejorDistancia = distanciaMaxima;
            mejorNodoA = nodoActual;
            mejorNodoB = nodoMasLejano;
        }

        nodoActual = nodoMasLejano;
    }

    ResultadoDiametro resultado;
    resultado.nodoA = mejorNodoA;
    resultado.nodoB = mejorNodoB;
    resultado.distanciaMetros = mejorDistancia;
    return resultado;
}

double Algoritmos::construirMST(
    const Grafo& grafo, const std::vector<int>& componenteGigante)
{
    if (componenteGigante.empty()) return 0.0;

    std::unordered_set<int> enComponente(componenteGigante.begin(),
                                          componenteGigante.end());

    std::cout << "    Filtrando aristas de la componente gigante..." << std::flush;
    const auto& todasAristas = grafo.obtenerAristasSinDireccion();
    std::vector<AristaSinDireccion> aristasComponente;

    for (const auto& arista : todasAristas) {
        if (enComponente.count(arista.nodoA) && enComponente.count(arista.nodoB)) {
            aristasComponente.push_back(arista);
        }
    }
    std::cout << " " << aristasComponente.size() << " aristas." << std::endl;

    std::cout << "    Ordenando aristas por distancia..." << std::flush;
    std::sort(aristasComponente.begin(), aristasComponente.end(),
        [](const AristaSinDireccion& a, const AristaSinDireccion& b) {
            return a.distanciaMetros < b.distanciaMetros;
        });
    std::cout << " Listo." << std::endl;

    std::cout << "    Ejecutando Kruskal..." << std::flush;
    int totalNodos = grafo.obtenerCantidadNodos();
    UnionFind conjuntos(totalNodos);

    double distanciaTotalMST = 0.0;
    int aristasEnMST = 0;
    int aristasNecesarias = (int)componenteGigante.size() - 1;

    for (const auto& arista : aristasComponente) {
        if (aristasEnMST >= aristasNecesarias) break;

        if (conjuntos.unir(arista.nodoA, arista.nodoB)) {
            distanciaTotalMST += arista.distanciaMetros;
            aristasEnMST++;
        }
    }

    std::cout << " Listo. " << aristasEnMST << " aristas en el MST." << std::endl;

    return distanciaTotalMST;
}

std::pair<Algoritmos::ResultadoRuta, Algoritmos::ResultadoRuta>
Algoritmos::compararRutas(const Grafo& grafo, int nodoOrigen, int nodoDestino)
{
    ResultadoRuta rutaDistancia, rutaTiempo;

    std::cout << "    Dijkstra por distancia..." << std::flush;
    std::vector<int> previosDistancia;
    std::vector<double> distancias = dijkstra(
        grafo, nodoOrigen, false, true, previosDistancia);

    rutaDistancia.camino = reconstruirCamino(nodoOrigen, nodoDestino, previosDistancia);
    calcularMetricasCamino(grafo, rutaDistancia.camino,
                           rutaDistancia.distanciaTotalMetros,
                           rutaDistancia.tiempoTotalSegundos);
    std::cout << " Listo." << std::endl;

    std::cout << "    Dijkstra por tiempo..." << std::flush;
    std::vector<int> previosTiempo;
    std::vector<double> tiempos = dijkstra(
        grafo, nodoOrigen, true, true, previosTiempo);

    rutaTiempo.camino = reconstruirCamino(nodoOrigen, nodoDestino, previosTiempo);
    calcularMetricasCamino(grafo, rutaTiempo.camino,
                           rutaTiempo.distanciaTotalMetros,
                           rutaTiempo.tiempoTotalSegundos);
    std::cout << " Listo." << std::endl;

    return std::make_pair(rutaDistancia, rutaTiempo);
}
