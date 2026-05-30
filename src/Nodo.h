#ifndef NODO_H
#define NODO_H

class Nodo {
private:
    int identificador;
    double latitud;
    double longitud;

public:
    Nodo() : identificador(-1), latitud(0.0), longitud(0.0) {}

    Nodo(int id, double lat, double lon)
        : identificador(id), latitud(lat), longitud(lon) {}

    int obtenerId() const { return identificador; }
    double obtenerLatitud() const { return latitud; }
    double obtenerLongitud() const { return longitud; }
};

#endif
