#ifndef LECTOR_CSV_H
#define LECTOR_CSV_H

#include <vector>
#include <string>
#include "Nodo.h"
#include "Arista.h"

class LectorCSV {
public:
    static std::vector<Nodo> leerNodos(const std::string& rutaArchivo);

    static std::vector<Arista> leerAristas(const std::string& rutaArchivo);
};

#endif
