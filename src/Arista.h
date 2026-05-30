#ifndef ARISTA_H
#define ARISTA_H

#include <string>

class Arista {
private:
    int nodoOrigen;
    int nodoDestino;
    double distanciaMetros;
    std::string tipoVia;
    int velocidadMaxKmh;
    bool unidireccional;

public:
    Arista()
        : nodoOrigen(-1), nodoDestino(-1), distanciaMetros(0.0),
          tipoVia(""), velocidadMaxKmh(30), unidireccional(false) {}

    Arista(int origen, int destino, double distancia,
           const std::string& tipo, int velocidad, bool esUnidireccional)
        : nodoOrigen(origen), nodoDestino(destino), distanciaMetros(distancia),
          tipoVia(tipo), velocidadMaxKmh(velocidad), unidireccional(esUnidireccional) {}

    int obtenerOrigen() const { return nodoOrigen; }
    int obtenerDestino() const { return nodoDestino; }
    double obtenerDistancia() const { return distanciaMetros; }
    std::string obtenerTipoVia() const { return tipoVia; }
    int obtenerVelocidad() const { return velocidadMaxKmh; }
    bool esUnidireccional() const { return unidireccional; }

    double calcularTiempoSegundos() const {
        int velocidad = (velocidadMaxKmh > 0) ? velocidadMaxKmh : 30;
        return distanciaMetros * 3.6 / velocidad;
    }
};

#endif
