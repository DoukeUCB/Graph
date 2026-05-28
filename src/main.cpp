#include <iostream>
#include <iomanip>
#include <chrono>
#include <string>
#include <random>
#include "Grafo.h"
#include "Algoritmos.h"

/**
 * Función auxiliar para medir tiempo de ejecución.
 * Usa chrono::high_resolution_clock para precisión en milisegundos.
 */
template<typename Funcion>
double medirTiempo(Funcion funcion) {
    auto inicio = std::chrono::high_resolution_clock::now();
    funcion();
    auto fin = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duracion = fin - inicio;
    return duracion.count();
}

/**
 * Formatea segundos a un string legible (ej: "2 min 34.5 seg")
 */
std::string formatearTiempo(double segundos) {
    if (segundos < 60) {
        return std::to_string(segundos) + " seg";
    }
    int minutos = (int)(segundos / 60);
    double segs  = segundos - minutos * 60;
    return std::to_string(minutos) + " min " +
           std::to_string(segs).substr(0, 5) + " seg";
}

// ====================================================================
// Variable global para la componente gigante (compartida entre objetivos)
// ====================================================================
static Algoritmos::ResultadoComponentes resultadoComponentes;
static bool componentesCalculadas = false;

/**
 * Asegura que las componentes conexas estén calculadas.
 * Se calcula una sola vez y se reutiliza.
 */
void asegurarComponentes(const Grafo& grafo) {
    if (!componentesCalculadas) {
        std::cout << "\n  [Calculando componentes conexas primero...]\n";
        double tiempo = medirTiempo([&]() {
            resultadoComponentes = Algoritmos::encontrarComponentesConexas(grafo);
        });
        componentesCalculadas = true;
        std::cout << "  [Componentes calculadas en " << std::fixed
                  << std::setprecision(2) << tiempo << " seg]\n";
    }
}

// ====================================================================
// FUNCIONES PARA CADA OBJETIVO
// ====================================================================

void ejecutarObjetivo1(const Grafo& grafo) {
    std::cout << "\n========================================" << std::endl;
    std::cout << " OBJETIVO 1: ALCANCE VEHICULAR (5 km)" << std::endl;
    std::cout << "========================================\n" << std::endl;

    int nodoOrigen = 0;
    double distanciaMaxima = 5000.0; // 5 km en metros

    std::cout << "  Nodo origen: " << nodoOrigen << std::endl;
    std::cout << "  Distancia maxima: " << distanciaMaxima / 1000.0 << " km\n" << std::endl;

    int alcance = 0;
    double tiempoEjecucion = medirTiempo([&]() {
        alcance = Algoritmos::calcularAlcanceVehicular(grafo, nodoOrigen, distanciaMaxima);
    });

    std::cout << "  RESULTADO:" << std::endl;
    std::cout << "    Nodos alcanzables en 5 km: " << alcance << std::endl;
    std::cout << "    Tiempo de ejecucion: " << std::fixed
              << std::setprecision(3) << tiempoEjecucion << " seg" << std::endl;
}

void ejecutarObjetivo2(const Grafo& grafo) {
    std::cout << "\n========================================" << std::endl;
    std::cout << " OBJETIVO 2: ISLAS VIALES" << std::endl;
    std::cout << "========================================\n" << std::endl;

    double tiempoEjecucion = 0;

    if (!componentesCalculadas) {
        tiempoEjecucion = medirTiempo([&]() {
            resultadoComponentes = Algoritmos::encontrarComponentesConexas(grafo);
        });
        componentesCalculadas = true;
    }

    std::cout << "  RESULTADO:" << std::endl;
    std::cout << "    Numero total de islas (componentes): "
              << resultadoComponentes.cantidadComponentes << std::endl;
    std::cout << "    Tamano de la componente gigante: "
              << resultadoComponentes.tamanoComponenteGigante << " nodos" << std::endl;
    std::cout << "    Porcentaje de nodos en la gigante: "
              << std::fixed << std::setprecision(2)
              << (100.0 * resultadoComponentes.tamanoComponenteGigante /
                  grafo.obtenerCantidadNodos()) << "%" << std::endl;
    if (tiempoEjecucion > 0) {
        std::cout << "    Tiempo de ejecucion: " << std::fixed
                  << std::setprecision(3) << tiempoEjecucion << " seg" << std::endl;
    }
}

void ejecutarObjetivo3(const Grafo& grafo) {
    std::cout << "\n========================================" << std::endl;
    std::cout << " OBJETIVO 3: DIAMETRO VIAL" << std::endl;
    std::cout << "========================================\n" << std::endl;

    asegurarComponentes(grafo);

    std::cout << "  Calculando diametro con heuristica double-sweep...\n" << std::endl;

    Algoritmos::ResultadoDiametro resultado;
    double tiempoEjecucion = medirTiempo([&]() {
        resultado = Algoritmos::calcularDiametroVial(
            grafo, resultadoComponentes.nodosComponenteGigante);
    });

    std::cout << "\n  RESULTADO:" << std::endl;
    std::cout << "    Par de intersecciones: nodo " << resultado.nodoA
              << " <-> nodo " << resultado.nodoB << std::endl;
    std::cout << "    Distancia minima entre ellos: "
              << std::fixed << std::setprecision(2)
              << resultado.distanciaMetros << " metros ("
              << resultado.distanciaMetros / 1000.0 << " km)" << std::endl;
    std::cout << "    Tiempo de ejecucion: " << std::fixed
              << std::setprecision(3) << tiempoEjecucion << " seg" << std::endl;
}

void ejecutarObjetivo4(const Grafo& grafo) {
    std::cout << "\n========================================" << std::endl;
    std::cout << " OBJETIVO 4: RED DE EMERGENCIA (MST)" << std::endl;
    std::cout << "========================================\n" << std::endl;

    asegurarComponentes(grafo);

    double distanciaMST = 0;
    double tiempoEjecucion = medirTiempo([&]() {
        distanciaMST = Algoritmos::construirMST(
            grafo, resultadoComponentes.nodosComponenteGigante);
    });

    std::cout << "\n  RESULTADO:" << std::endl;
    std::cout << "    Distancia total del MST: "
              << std::fixed << std::setprecision(2)
              << distanciaMST / 1000.0 << " km" << std::endl;
    std::cout << "    Aristas en el MST: "
              << resultadoComponentes.tamanoComponenteGigante - 1 << std::endl;
    std::cout << "    Tiempo de ejecucion: " << std::fixed
              << std::setprecision(3) << tiempoEjecucion << " seg" << std::endl;
}

void ejecutarObjetivo5(const Grafo& grafo) {
    std::cout << "\n========================================" << std::endl;
    std::cout << " OBJETIVO 5: RUTA DISTANCIA VS. TIEMPO" << std::endl;
    std::cout << "========================================\n" << std::endl;

    asegurarComponentes(grafo);

    // Seleccionar dos nodos aleatorios de la componente gigante
    const auto& nodosGigante = resultadoComponentes.nodosComponenteGigante;
    std::mt19937 generador(42); // Semilla fija para reproducibilidad
    std::uniform_int_distribution<int> distribucion(0, (int)nodosGigante.size() - 1);

    int nodoOrigen  = nodosGigante[distribucion(generador)];
    int nodoDestino = nodosGigante[distribucion(generador)];

    // Asegurar que sean diferentes
    while (nodoDestino == nodoOrigen) {
        nodoDestino = nodosGigante[distribucion(generador)];
    }

    std::cout << "  Nodo origen:  " << nodoOrigen << std::endl;
    std::cout << "  Nodo destino: " << nodoDestino << "\n" << std::endl;

    std::pair<Algoritmos::ResultadoRuta, Algoritmos::ResultadoRuta> rutas;
    double tiempoEjecucion = medirTiempo([&]() {
        rutas = Algoritmos::compararRutas(grafo, nodoOrigen, nodoDestino);
    });

    Algoritmos::ResultadoRuta& rutaDistancia = rutas.first;
    Algoritmos::ResultadoRuta& rutaTiempo = rutas.second;

    std::cout << "\n  RESULTADO:\n" << std::endl;

    // --- Ruta optimizada por distancia ---
    std::cout << "  --- Ruta mas CORTA (optimizada por distancia) ---" << std::endl;
    if (rutaDistancia.camino.empty()) {
        std::cout << "    No se encontro camino." << std::endl;
    } else {
        std::cout << "    Distancia total: " << std::fixed << std::setprecision(2)
                  << rutaDistancia.distanciaTotalMetros / 1000.0 << " km" << std::endl;
        std::cout << "    Tiempo estimado: "
                  << formatearTiempo(rutaDistancia.tiempoTotalSegundos) << std::endl;
        std::cout << "    Nodos en el camino: " << rutaDistancia.camino.size() << std::endl;
    }

    std::cout << std::endl;

    // --- Ruta optimizada por tiempo ---
    std::cout << "  --- Ruta mas RAPIDA (optimizada por tiempo) ---" << std::endl;
    if (rutaTiempo.camino.empty()) {
        std::cout << "    No se encontro camino." << std::endl;
    } else {
        std::cout << "    Distancia total: " << std::fixed << std::setprecision(2)
                  << rutaTiempo.distanciaTotalMetros / 1000.0 << " km" << std::endl;
        std::cout << "    Tiempo estimado: "
                  << formatearTiempo(rutaTiempo.tiempoTotalSegundos) << std::endl;
        std::cout << "    Nodos en el camino: " << rutaTiempo.camino.size() << std::endl;
    }

    // --- Análisis comparativo ---
    if (!rutaDistancia.camino.empty() && !rutaTiempo.camino.empty()) {
        std::cout << "\n  --- ANALISIS COMPARATIVO ---" << std::endl;

        double difDistancia = rutaTiempo.distanciaTotalMetros -
                              rutaDistancia.distanciaTotalMetros;
        double difTiempo = rutaDistancia.tiempoTotalSegundos -
                           rutaTiempo.tiempoTotalSegundos;

        std::cout << "    La ruta rapida recorre "
                  << std::fixed << std::setprecision(2)
                  << std::abs(difDistancia) / 1000.0 << " km "
                  << (difDistancia >= 0 ? "MAS" : "MENOS")
                  << " que la ruta corta." << std::endl;

        std::cout << "    La ruta corta tarda "
                  << std::fixed << std::setprecision(1)
                  << std::abs(difTiempo) << " seg "
                  << (difTiempo >= 0 ? "MAS" : "MENOS")
                  << " que la ruta rapida." << std::endl;

        if (difDistancia > 0 && difTiempo > 0) {
            std::cout << "\n    Conclusion: La ruta rapida es mas larga pero mas veloz." << std::endl;
            std::cout << "    Esto ocurre porque prefiere vias rapidas (autopistas, troncales)" << std::endl;
            std::cout << "    aunque sean mas largas en distancia." << std::endl;
        } else if (std::abs(difDistancia) < 0.01 && std::abs(difTiempo) < 0.01) {
            std::cout << "\n    Conclusion: Ambas rutas son identicas." << std::endl;
            std::cout << "    Esto ocurre cuando no hay alternativas con vias mas rapidas." << std::endl;
        }
    }

    std::cout << "\n    Tiempo de ejecucion total: " << std::fixed
              << std::setprecision(3) << tiempoEjecucion << " seg" << std::endl;
}

// ====================================================================
// FUNCIÓN PRINCIPAL
// ====================================================================
int main() {
    std::cout << "==================================================" << std::endl;
    std::cout << "  RUTAS OPTIMAS EN RED VIAL URBANA - BOLIVIA" << std::endl;
    std::cout << "  Dataset: OpenStreetMap/Geofabrik" << std::endl;
    std::cout << "==================================================" << std::endl;

    // --- Cargar el grafo ---
    std::cout << "\nCargando dataset...\n" << std::endl;

    Grafo grafo;
    double tiempoCarga = medirTiempo([&]() {
        grafo.construir("data/nodes.csv", "data/edges.csv");
    });

    std::cout << "\n  Tiempo de carga: " << std::fixed << std::setprecision(2)
              << tiempoCarga << " seg\n" << std::endl;

    // --- Menú principal ---
    int opcion = -1;
    while (opcion != 0) {
        std::cout << "\n--------------------------------------------------" << std::endl;
        std::cout << "  MENU PRINCIPAL" << std::endl;
        std::cout << "--------------------------------------------------" << std::endl;
        std::cout << "  1. Alcance vehicular (5 km desde nodo 0)" << std::endl;
        std::cout << "  2. Islas viales (componentes conexas)" << std::endl;
        std::cout << "  3. Diametro vial (componente gigante)" << std::endl;
        std::cout << "  4. Red de emergencia minima (MST)" << std::endl;
        std::cout << "  5. Ruta por tipo (distancia vs. tiempo)" << std::endl;
        std::cout << "  6. Ejecutar TODOS los objetivos" << std::endl;
        std::cout << "  0. Salir" << std::endl;
        std::cout << "--------------------------------------------------" << std::endl;
        std::cout << "  Seleccione una opcion: ";
        std::cin >> opcion;

        switch (opcion) {
            case 1: ejecutarObjetivo1(grafo); break;
            case 2: ejecutarObjetivo2(grafo); break;
            case 3: ejecutarObjetivo3(grafo); break;
            case 4: ejecutarObjetivo4(grafo); break;
            case 5: ejecutarObjetivo5(grafo); break;
            case 6:
                ejecutarObjetivo1(grafo);
                ejecutarObjetivo2(grafo);
                ejecutarObjetivo3(grafo);
                ejecutarObjetivo4(grafo);
                ejecutarObjetivo5(grafo);
                break;
            case 0:
                std::cout << "\n  Saliendo del programa. Hasta pronto!\n" << std::endl;
                break;
            default:
                std::cout << "\n  Opcion invalida. Intente de nuevo." << std::endl;
        }
    }

    return 0;
}
