#include "Grafo.h"
#include "LectorCSV.h"
#include <iostream>
#include <algorithm>

Grafo::Grafo() : cantidadNodos(0) {}

void Grafo::construir(const std::string& rutaNodos, const std::string& rutaAristas) {
    // ============================================================
    // PASO 1: Leer nodos del CSV
    // ============================================================
    std::cout << "  Leyendo nodos..." << std::flush;
    std::vector<Nodo> nodosLeidos = LectorCSV::leerNodos(rutaNodos);

    // Determinar el ID máximo para dimensionar los vectores
    int idMaximo = 0;
    for (const auto& nodo : nodosLeidos) {
        idMaximo = std::max(idMaximo, nodo.obtenerId());
    }
    cantidadNodos = idMaximo + 1;

    // Inicializar el vector de nodos (indexado por ID)
    nodos.resize(cantidadNodos);
    for (const auto& nodo : nodosLeidos) {
        nodos[nodo.obtenerId()] = nodo;
    }
    std::cout << " " << nodosLeidos.size() << " nodos leidos." << std::endl;

    // ============================================================
    // PASO 2: Leer aristas del CSV
    // ============================================================
    std::cout << "  Leyendo aristas..." << std::flush;
    std::vector<Arista> aristasLeidas = LectorCSV::leerAristas(rutaAristas);
    std::cout << " " << aristasLeidas.size() << " aristas leidas." << std::endl;

    // ============================================================
    // PASO 3: Construir listas de adyacencia
    // ============================================================
    std::cout << "  Construyendo listas de adyacencia..." << std::flush;

    adyacenciaDirigida.resize(cantidadNodos);
    adyacenciaNoDirigida.resize(cantidadNodos);

    int aristasAgregadas = 0;

    for (const auto& arista : aristasLeidas) {
        int origen  = arista.obtenerOrigen();
        int destino = arista.obtenerDestino();

        // Validar que los IDs estén dentro del rango
        if (origen < 0 || origen >= cantidadNodos ||
            destino < 0 || destino >= cantidadNodos) {
            continue; // Saltar aristas con IDs inválidos
        }

        double distancia = arista.obtenerDistancia();
        double tiempo    = arista.calcularTiempoSegundos();

        // --- Grafo DIRIGIDO ---
        // Siempre agregar origen → destino
        adyacenciaDirigida[origen].emplace_back(destino, distancia, tiempo);

        // Si NO es unidireccional (oneway=0), agregar también destino → origen
        if (!arista.esUnidireccional()) {
            adyacenciaDirigida[destino].emplace_back(origen, distancia, tiempo);
        }

        // --- Grafo NO DIRIGIDO ---
        // Siempre agregar en ambas direcciones
        adyacenciaNoDirigida[origen].emplace_back(destino, distancia, tiempo);
        adyacenciaNoDirigida[destino].emplace_back(origen, distancia, tiempo);

        // --- Lista de aristas sin dirección (para Kruskal) ---
        listaAristasSinDireccion.push_back({origen, destino, distancia});

        aristasAgregadas++;
    }

    std::cout << " Listo. " << aristasAgregadas << " aristas procesadas." << std::endl;
    std::cout << "  Grafo construido: " << cantidadNodos << " nodos, "
              << aristasAgregadas << " aristas." << std::endl;
}

// --- Implementación de getters ---

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
