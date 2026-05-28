#ifndef LECTOR_CSV_H
#define LECTOR_CSV_H

#include <vector>
#include <string>
#include "Nodo.h"
#include "Arista.h"

/**
 * Clase LectorCSV: encargada de leer los archivos CSV del dataset.
 * Separa la lógica de lectura de archivos de la lógica del grafo
 * (principio de responsabilidad única).
 */
class LectorCSV {
public:
    /**
     * Lee el archivo nodes.csv y retorna un vector de objetos Nodo.
     * Formato esperado: node_id,lat,lon
     */
    static std::vector<Nodo> leerNodos(const std::string& rutaArchivo);

    /**
     * Lee el archivo edges.csv y retorna un vector de objetos Arista.
     * Formato esperado: osm_id,from_id,to_id,distance_m,fclass,oneway,maxspeed
     */
    static std::vector<Arista> leerAristas(const std::string& rutaArchivo);
};

#endif // LECTOR_CSV_H
