#ifndef GRAFO_H
#define GRAFO_H

#include <vector>
#include <string>
#include "Nodo.h"

/**
 * Estructura Vecino: entrada ligera en la lista de adyacencia.
 * Almacena solo lo necesario para los algoritmos: nodo destino,
 * distancia en metros y tiempo en segundos.
 *
 * Se usa una estructura separada (no la clase Arista completa)
 * para ahorrar memoria, ya que la lista de adyacencia se duplica
 * por cada arista bidireccional.
 */
struct Vecino {
    int nodoDestino;
    double distanciaMetros;
    double tiempoSegundos;

    Vecino(int destino, double distancia, double tiempo)
        : nodoDestino(destino), distanciaMetros(distancia), tiempoSegundos(tiempo) {}
};

/**
 * Estructura AristaSinDireccion: arista sin dirección para el MST (Kruskal).
 * Solo almacena los dos extremos y la distancia.
 */
struct AristaSinDireccion {
    int nodoA;
    int nodoB;
    double distanciaMetros;
};

/**
 * Clase Grafo: estructura principal que modela la red vial.
 *
 * Representación interna: Lista de adyacencia (vector de vectores).
 * - Cada nodo tiene un índice de 0 a N-1.
 * - Cada entrada en la lista de adyacencia contiene (nodoDestino, distancia, tiempo).
 *
 * ¿Por qué lista de adyacencia y no matriz?
 *   Con 887K nodos, una matriz de adyacencia necesitaría 887K × 887K = ~787 GB.
 *   La lista de adyacencia solo almacena las aristas existentes (~1.2M), usando ~50 MB.
 *
 * El grafo mantiene dos versiones de la lista de adyacencia:
 *   - Dirigida: respeta el sentido de las calles (para Dijkstra).
 *   - No dirigida: trata todas las calles como bidireccionales
 *                  (para componentes conexas y diámetro).
 */
class Grafo {
private:
    int cantidadNodos;
    std::vector<Nodo> nodos;
    std::vector<std::vector<Vecino>> adyacenciaDirigida;
    std::vector<std::vector<Vecino>> adyacenciaNoDirigida;
    std::vector<AristaSinDireccion> listaAristasSinDireccion; // Para Kruskal

public:
    Grafo();

    /**
     * Construye el grafo a partir de los archivos CSV.
     * Lee nodos y aristas, y llena ambas listas de adyacencia.
     */
    void construir(const std::string& rutaNodos, const std::string& rutaAristas);

    // --- Getters ---
    int obtenerCantidadNodos() const;
    const Nodo& obtenerNodo(int id) const;
    const std::vector<Vecino>& obtenerVecinosDirigidos(int nodoId) const;
    const std::vector<Vecino>& obtenerVecinosNoDirigidos(int nodoId) const;
    const std::vector<AristaSinDireccion>& obtenerAristasSinDireccion() const;
};

#endif // GRAFO_H
