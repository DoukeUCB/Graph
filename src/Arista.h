#ifndef ARISTA_H
#define ARISTA_H

#include <string>

/**
 * Clase Arista: representa un segmento de calle entre dos intersecciones.
 * Almacena la distancia en metros, el tipo de vía, la velocidad máxima
 * y si es unidireccional.
 */
class Arista {
private:
    int nodoOrigen;
    int nodoDestino;
    double distanciaMetros;
    std::string tipoVia;       // residential, trunk, primary, etc.
    int velocidadMaxKmh;       // velocidad máxima en km/h
    bool unidireccional;       // true si oneway=1

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

    /**
     * Calcula el tiempo de recorrido en segundos.
     * Fórmula: tiempo = distancia_m / (velocidad_kmh / 3.6)
     *        = distancia_m * 3.6 / velocidad_kmh
     */
    double calcularTiempoSegundos() const {
        int velocidad = (velocidadMaxKmh > 0) ? velocidadMaxKmh : 30;
        return distanciaMetros * 3.6 / velocidad;
    }
};

#endif // ARISTA_H
