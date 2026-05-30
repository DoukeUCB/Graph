#include "Grafo.h"
#include "LectorCSV.h"
#include <iostream>
#include <algorithm>

Grafo::Grafo() : cantidadNodos(0) {}

void Grafo::construir(const std::string& rutaNodos, const std::string& rutaAristas) {
    std::cout << "  Leyendo nodos..." << std::flush;
    std::vector<Nodo> nodosLeidos = LectorCSV::leerNodos(rutaNodos);

    int idMaximo = 0;
    for (const auto& nodo : nodosLeidos) {
        idMaximo = std::max(idMaximo, nodo.obtenerId());
    }
    cantidadNodos = idMaximo + 1;

    nodos.resize(cantidadNodos);
    for (const auto& nodo : nodosLeidos) {
        nodos[nodo.obtenerId()] = nodo;
    }
    std::cout << " " << nodosLeidos.size() << " nodos leidos." << std::endl;

    std::cout << "  Leyendo aristas..." << std::flush;
    std::vector<Arista> aristasLeidas = LectorCSV::leerAristas(rutaAristas);
    std::cout << " " << aristasLeidas.size() << " aristas leidas." << std::endl;

    std::cout << "  Construyendo listas de adyacencia..." << std::flush;

    adyacenciaDirigida.resize(cantidadNodos);
    adyacenciaNoDirigida.resize(cantidadNodos);

    int aristasAgregadas = 0;

    for (const auto& arista : aristasLeidas) {
        int origen  = arista.obtenerOrigen();
        int destino = arista.obtenerDestino();

        if (origen < 0 || origen >= cantidadNodos ||
            destino < 0 || destino >= cantidadNodos) {
            continue;
        }

        double distancia = arista.obtenerDistancia();
        double tiempo    = arista.calcularTiempoSegundos();

        adyacenciaDirigida[origen].emplace_back(destino, distancia, tiempo);

        if (!arista.esUnidireccional()) {
            adyacenciaDirigida[destino].emplace_back(origen, distancia, tiempo);
        }

        adyacenciaNoDirigida[origen].emplace_back(destino, distancia, tiempo);
        adyacenciaNoDirigida[destino].emplace_back(origen, distancia, tiempo);

        listaAristasSinDireccion.push_back({origen, destino, distancia});

        aristasAgregadas++;
    }

    std::cout << " Listo. " << aristasAgregadas << " aristas procesadas." << std::endl;
    std::cout << "  Grafo construido: " << cantidadNodos << " nodos, "
              << aristasAgregadas << " aristas." << std::endl;
}


int Grafo::obtenerCantidadNodos() const {
    return cantidadNodos;
}

const Nodo& Grafo::obtenerNodo(int id) const {
    return nodos[id];
}

const std::vector<Vecino>& Grafo::obtenerVecinosDirigidos(int nodoId) const {
    return adyacenciaDirigida[nodoId];
}

const std::vector<Vecino>& Grafo::obtenerVecinosNoDirigidos(int nodoId) const {
    return adyacenciaNoDirigida[nodoId];
}

const std::vector<AristaSinDireccion>& Grafo::obtenerAristasSinDireccion() const {
    return listaAristasSinDireccion;
}
