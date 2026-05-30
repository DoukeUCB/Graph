#ifndef GRAFO_H
#define GRAFO_H

#include <vector>
#include <string>
#include "Nodo.h"

struct Vecino {
    int nodoDestino;
    double distanciaMetros;
    double tiempoSegundos;

    Vecino(int destino, double distancia, double tiempo)
        : nodoDestino(destino), distanciaMetros(distancia), tiempoSegundos(tiempo) {}
};

struct AristaSinDireccion {
    int nodoA;
    int nodoB;
    double distanciaMetros;
};

class Grafo {
private:
    int cantidadNodos;
    std::vector<Nodo> nodos;
    std::vector<std::vector<Vecino>> adyacenciaDirigida;
    std::vector<std::vector<Vecino>> adyacenciaNoDirigida;
    std::vector<AristaSinDireccion> listaAristasSinDireccion;

public:
    Grafo();

    void construir(const std::string& rutaNodos, const std::string& rutaAristas);

    int obtenerCantidadNodos() const;
    const Nodo& obtenerNodo(int id) const;
    const std::vector<Vecino>& obtenerVecinosDirigidos(int nodoId) const;
    const std::vector<Vecino>& obtenerVecinosNoDirigidos(int nodoId) const;
    const std::vector<AristaSinDireccion>& obtenerAristasSinDireccion() const;
};

#endif
