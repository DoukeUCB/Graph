#include "LectorCSV.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

std::vector<Nodo> LectorCSV::leerNodos(const std::string& rutaArchivo) {
    std::ifstream archivo(rutaArchivo);
    if (!archivo.is_open()) {
        throw std::runtime_error("No se pudo abrir el archivo: " + rutaArchivo);
    }

    std::vector<Nodo> nodos;
    std::string linea;

    std::getline(archivo, linea);

    while (std::getline(archivo, linea)) {
        if (linea.empty()) continue;

        std::stringstream flujo(linea);
        std::string campo;

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

    std::getline(archivo, linea);

    while (std::getline(archivo, linea)) {
        if (linea.empty()) continue;

        std::stringstream flujo(linea);
        std::string campo;

        std::getline(flujo, campo, ',');

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

        if (velocidadMax <= 0) velocidadMax = 30;

        aristas.emplace_back(origen, destino, distancia, tipoVia,
                             velocidadMax, esUnidireccional);
    }

    archivo.close();
    return aristas;
}
