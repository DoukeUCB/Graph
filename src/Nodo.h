#ifndef NODO_H
#define NODO_H

/**
 * Clase Nodo: representa una intersección vial.
 * Cada nodo tiene un identificador único y coordenadas UTM (latitud, longitud).
 */
class Nodo {
private:
    int identificador;
    double latitud;   // Coordenada Y (proyección UTM, en metros)
    double longitud;  // Coordenada X (proyección UTM, en metros)

public:
    Nodo() : identificador(-1), latitud(0.0), longitud(0.0) {}

    Nodo(int id, double lat, double lon)
        : identificador(id), latitud(lat), longitud(lon) {}

    int obtenerId() const { return identificador; }
    double obtenerLatitud() const { return latitud; }
    double obtenerLongitud() const { return longitud; }
};

#endif // NODO_H
