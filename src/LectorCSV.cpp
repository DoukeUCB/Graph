#include "LectorCSV.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

/**
 * Implementación de LectorCSV.
 *
 * Usa ifstream + getline para leer línea por línea.
 * Cada línea se parsea con stringstream y getline(',') para separar campos.
 * La primera línea (cabecera) se descarta.
 */

std::vector<Nodo> LectorCSV::leerNodos(const std::string& rutaArchivo) {
    std::ifstream archivo(rutaArchivo);
    if (!archivo.is_open()) {
        throw std::runtime_error("No se pudo abrir el archivo: " + rutaArchivo);
    }

    std::vector<Nodo> nodos;
    std::string linea;

    // Saltar la cabecera (primera línea)
    std::getline(archivo, linea);

    while (std::getline(archivo, linea)) {
        if (linea.empty()) continue;

        std::stringstream flujo(linea);
        std::string campo;

        // Leer: node_id, lat, lon
        std::getline(flujo, campo, ',');
        int identificador = std::stoi(campo);

        std::getline(flujo, campo, ',');
        double latitud = std::stod(campo);

        std::getline(flujo, campo, ',');
        double longitud = std::stod(campo);

        nodos.emplace_back(identificador, latitud, longitud);
    }

    archivo.close();
    return nodos;
}

std::vector<Arista> LectorCSV::leerAristas(const std::string& rutaArchivo) {
    std::ifstream archivo(rutaArchivo);
    if (!archivo.is_open()) {
        throw std::runtime_error("No se pudo abrir el archivo: " + rutaArchivo);
    }

    std::vector<Arista> aristas;
    std::string linea;

    // Saltar la cabecera
    std::getline(archivo, linea);

    while (std::getline(archivo, linea)) {
        if (linea.empty()) continue;

        std::stringstream flujo(linea);
        std::string campo;

        // Leer: osm_id, from_id, to_id, distance_m, fclass, oneway, maxspeed
        std::getline(flujo, campo, ','); // osm_id (no lo usamos)

        std::getline(flujo, campo, ',');
        int origen = std::stoi(campo);

        std::getline(flujo, campo, ',');
        int destino = std::stoi(campo);

        std::getline(flujo, campo, ',');
        double distancia = std::stod(campo);

        std::getline(flujo, campo, ',');
        std::string tipoVia = campo;

        std::getline(flujo, campo, ',');
        bool esUnidireccional = (std::stoi(campo) == 1);

        std::getline(flujo, campo, ',');
        int velocidadMax = std::stoi(campo);

        // Si la velocidad es inválida, usar 30 km/h como default
        if (velocidadMax <= 0) velocidadMax = 30;

        aristas.emplace_back(origen, destino, distancia, tipoVia,
                             velocidadMax, esUnidireccional);
    }

    archivo.close();
    return aristas;
}
