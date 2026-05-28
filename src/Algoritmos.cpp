#include "Algoritmos.h"
#include "UnionFind.h"
#include <queue>
#include <algorithm>
#include <iostream>
#include <limits>
#include <unordered_set>
#include <functional>
#include <cmath>

// Valor que representa "infinito" en distancias
static const double INFINITO = std::numeric_limits<double>::infinity();

// ====================================================================
// DIJKSTRA GENÉRICO (método privado auxiliar)
// ====================================================================
/**
 * Implementación de Dijkstra con cola de prioridad (min-heap).
 *
 * Estructura de datos: priority_queue con pair<double, int>.
 *   - El primer elemento es la distancia/tiempo acumulado (para que el
 *     heap ordene por distancia mínima).
 *   - El segundo elemento es el nodo.
 *   - Usamos greater<> para convertir el max-heap por defecto en min-heap.
 *
 * Técnica "lazy deletion": en lugar de actualizar la prioridad de un nodo
 * en la cola (operación costosa), simplemente insertamos la nueva distancia.
 * Cuando sacamos un nodo, verificamos si ya fue procesado; si sí, lo ignoramos.
 *
 * Complejidad: O((V + E) * log V)
 *   - Cada nodo se saca del heap como máximo 1 vez: V operaciones pop → O(V log V)
 *   - Cada arista puede generar 1 inserción al heap: E operaciones push → O(E log V)
 *   - Total: O((V + E) * log V)
 */
std::vector<double> Algoritmos::dijkstra(
    const Grafo& grafo, int nodoOrigen,
    bool porTiempo, bool usarNoDirigido,
    std::vector<int>& nodoPrevio)
{
    int totalNodos = grafo.obtenerCantidadNodos();

    // Inicializar distancias en infinito y previos en -1
    std::vector<double> distancia(totalNodos, INFINITO);
    nodoPrevio.assign(totalNodos, -1);
    std::vector<bool> procesado(totalNodos, false);

    // Cola de prioridad: (distanciaAcumulada, nodoId) — min-heap
    using ParDistanciaNodo = std::pair<double, int>;
    std::priority_queue<ParDistanciaNodo,
                        std::vector<ParDistanciaNodo>,
                        std::greater<ParDistanciaNodo>> colaPrioridad;

    // El nodo origen tiene distancia 0
    distancia[nodoOrigen] = 0.0;
    colaPrioridad.push(std::make_pair(0.0, nodoOrigen));

    while (!colaPrioridad.empty()) {
        ParDistanciaNodo tope = colaPrioridad.top();
        colaPrioridad.pop();
        double distanciaActual = tope.first;
        int nodoActual = tope.second;

        // Lazy deletion: si ya procesamos este nodo, ignorar
        if (procesado[nodoActual]) continue;
        procesado[nodoActual] = true;

        // Explorar vecinos
        const auto& vecinos = usarNoDirigido
            ? grafo.obtenerVecinosNoDirigidos(nodoActual)
            : grafo.obtenerVecinosDirigidos(nodoActual);

        for (const auto& vecino : vecinos) {
            if (procesado[vecino.nodoDestino]) continue;

            // Seleccionar el peso según el criterio
            double peso = porTiempo ? vecino.tiempoSegundos : vecino.distanciaMetros;
            double nuevaDistancia = distanciaActual + peso;

            // ¿Encontramos un camino más corto?
            if (nuevaDistancia < distancia[vecino.nodoDestino]) {
                distancia[vecino.nodoDestino] = nuevaDistancia;
                nodoPrevio[vecino.nodoDestino] = nodoActual;
                colaPrioridad.push(std::make_pair(nuevaDistancia, vecino.nodoDestino));
            }
        }
    }

    return distancia;
}

// ====================================================================
// RECONSTRUIR CAMINO (método privado auxiliar)
// ====================================================================
/**
 * Recorre el vector de previos desde destino hasta origen
 * para reconstruir la secuencia de nodos del camino más corto.
 */
std::vector<int> Algoritmos::reconstruirCamino(
    int nodoOrigen, int nodoDestino,
    const std::vector<int>& nodoPrevio)
{
    std::vector<int> camino;

    // Si no hay camino (previo del destino es -1 y no es el origen)
    if (nodoPrevio[nodoDestino] == -1 && nodoDestino != nodoOrigen) {
        return camino; // Camino vacío = no alcanzable
    }

    // Recorrer desde destino hasta origen siguiendo previos
    for (int nodo = nodoDestino; nodo != -1; nodo = nodoPrevio[nodo]) {
        camino.push_back(nodo);
    }

    // Invertir para obtener origen → destino
    std::reverse(camino.begin(), camino.end());
    return camino;
}

// ====================================================================
// CALCULAR MÉTRICAS DE CAMINO (método privado auxiliar)
// ====================================================================
/**
 * Dado un camino (secuencia de nodos), calcula la distancia total
 * y el tiempo total sumando las aristas del grafo dirigido.
 */
void Algoritmos::calcularMetricasCamino(
    const Grafo& grafo, const std::vector<int>& camino,
    double& distanciaTotal, double& tiempoTotal)
{
    distanciaTotal = 0.0;
    tiempoTotal    = 0.0;

    for (size_t i = 0; i + 1 < camino.size(); i++) {
        int actual   = camino[i];
        int siguiente = camino[i + 1];

        // Buscar la arista actual → siguiente en la adyacencia dirigida
        const auto& vecinos = grafo.obtenerVecinosDirigidos(actual);
        double mejorDist = INFINITO;
        double mejorTiempo = INFINITO;

        for (const auto& vecino : vecinos) {
            if (vecino.nodoDestino == siguiente) {
                if (vecino.distanciaMetros < mejorDist) {
                    mejorDist   = vecino.distanciaMetros;
                    mejorTiempo = vecino.tiempoSegundos;
                }
            }
        }

        if (mejorDist < INFINITO) {
            distanciaTotal += mejorDist;
            tiempoTotal    += mejorTiempo;
        }
    }
}

// ====================================================================
// OBJETIVO 1: ALCANCE VEHICULAR
// ====================================================================
/**
 * Dijkstra con corte: explora el grafo desde el nodo origen y cuenta
 * cuántos nodos están a una distancia de calle ≤ maxDistanciaMetros.
 *
 * Es un Dijkstra normal pero dejamos de expandir nodos cuya distancia
 * acumulada supera el límite. Esto hace que el algoritmo sea mucho
 * más rápido que un Dijkstra completo, porque solo explora un
 * subconjunto local del grafo.
 */
int Algoritmos::calcularAlcanceVehicular(
    const Grafo& grafo, int nodoOrigen, double maxDistanciaMetros)
{
    int totalNodos = grafo.obtenerCantidadNodos();
    std::vector<double> distancia(totalNodos, INFINITO);
    std::vector<bool> procesado(totalNodos, false);

    using ParDistanciaNodo = std::pair<double, int>;
    std::priority_queue<ParDistanciaNodo,
                        std::vector<ParDistanciaNodo>,
                        std::greater<ParDistanciaNodo>> colaPrioridad;

    distancia[nodoOrigen] = 0.0;
    colaPrioridad.push(std::make_pair(0.0, nodoOrigen));

    int nodosAlcanzables = 0;

    while (!colaPrioridad.empty()) {
        ParDistanciaNodo tope = colaPrioridad.top();
        colaPrioridad.pop();
        double distanciaActual = tope.first;
        int nodoActual = tope.second;

        if (procesado[nodoActual]) continue;

        // Si la distancia supera el límite, ya no hay nodos más cercanos
        // (propiedad del min-heap: todos los siguientes tendrán distancia ≥)
        if (distanciaActual > maxDistanciaMetros) break;

        procesado[nodoActual] = true;
        nodosAlcanzables++;

        // Explorar vecinos (grafo dirigido, peso = distancia)
        for (const auto& vecino : grafo.obtenerVecinosDirigidos(nodoActual)) {
            if (procesado[vecino.nodoDestino]) continue;

            double nuevaDist = distanciaActual + vecino.distanciaMetros;

            if (nuevaDist < distancia[vecino.nodoDestino]) {
                distancia[vecino.nodoDestino] = nuevaDist;
                colaPrioridad.push(std::make_pair(nuevaDist, vecino.nodoDestino));
            }
        }
    }

    return nodosAlcanzables;
}

// ====================================================================
// OBJETIVO 2: ISLAS VIALES (COMPONENTES DÉBILMENTE CONEXAS)
// ====================================================================
/**
 * Usa BFS iterativo sobre el grafo NO dirigido para encontrar
 * las componentes débilmente conexas.
 *
 * ¿Por qué BFS y no DFS?
 *   - BFS es iterativo (usa queue), no hay riesgo de stack overflow
 *     con 887K nodos. DFS recursivo podría desbordar el stack.
 *   - Ambos tienen la misma complejidad: O(V + E).
 *
 * ¿Por qué débilmente conexas?
 *   Dos nodos están débilmente conectados si existe un camino entre
 *   ellos ignorando la dirección de las aristas. Esto modela la
 *   realidad: si hay una calle de A a B, físicamente A y B están
 *   "conectados" aunque la calle sea de un solo sentido.
 */
Algoritmos::ResultadoComponentes Algoritmos::encontrarComponentesConexas(
    const Grafo& grafo)
{
    int totalNodos = grafo.obtenerCantidadNodos();
    std::vector<int> componenteDeNodo(totalNodos, -1); // -1 = no visitado
    int cantidadComponentes = 0;
    int tamanoGigante = 0;
    int idComponenteGigante = -1;

    // Recorrer todos los nodos
    for (int nodoInicial = 0; nodoInicial < totalNodos; nodoInicial++) {
        // Si este nodo ya fue visitado, saltar
        if (componenteDeNodo[nodoInicial] != -1) continue;

        // Verificar que el nodo tiene al menos una arista
        // (los nodos aislados sin aristas no son intersecciones reales)
        if (grafo.obtenerVecinosNoDirigidos(nodoInicial).empty()) continue;

        // BFS desde nodoInicial
        std::queue<int> cola;
        cola.push(nodoInicial);
        componenteDeNodo[nodoInicial] = cantidadComponentes;
        int tamanoComponente = 0;

        while (!cola.empty()) {
            int nodoActual = cola.front();
            cola.pop();
            tamanoComponente++;

            for (const auto& vecino : grafo.obtenerVecinosNoDirigidos(nodoActual)) {
                if (componenteDeNodo[vecino.nodoDestino] == -1) {
                    componenteDeNodo[vecino.nodoDestino] = cantidadComponentes;
                    cola.push(vecino.nodoDestino);
                }
            }
        }

        // ¿Es la componente más grande encontrada hasta ahora?
        if (tamanoComponente > tamanoGigante) {
            tamanoGigante = tamanoComponente;
            idComponenteGigante = cantidadComponentes;
        }

        cantidadComponentes++;
    }

    // Recopilar los nodos de la componente gigante
    std::vector<int> nodosGigante;
    nodosGigante.reserve(tamanoGigante);
    for (int i = 0; i < totalNodos; i++) {
        if (componenteDeNodo[i] == idComponenteGigante) {
            nodosGigante.push_back(i);
        }
    }

    ResultadoComponentes resultado;
    resultado.cantidadComponentes = cantidadComponentes;
    resultado.tamanoComponenteGigante = tamanoGigante;
    resultado.nodosComponenteGigante = nodosGigante;
    return resultado;
}

// ====================================================================
// OBJETIVO 3: DIÁMETRO VIAL
// ====================================================================
/**
 * Estima el diámetro de la componente gigante con la heurística
 * "double sweep" (barrido doble):
 *
 * 1. Tomar un nodo arbitrario u de la componente.
 * 2. Ejecutar Dijkstra desde u → encontrar el nodo más lejano v.
 * 3. Ejecutar Dijkstra desde v → encontrar el nodo más lejano w.
 * 4. Repetir desde w una vez más para refinar.
 * 5. La distancia máxima encontrada es el diámetro estimado.
 *
 * ¿Por qué esta heurística y no el cálculo exacto?
 *   El diámetro exacto requiere Dijkstra desde TODOS los nodos:
 *   O(V × (V+E) log V). Con V=887K, esto tomaría días.
 *   La heurística da resultados exactos o muy cercanos en grafos
 *   viales (que tienen forma "alargada") en solo 3-4 ejecuciones
 *   de Dijkstra.
 *
 * Se usa el grafo NO DIRIGIDO para garantizar que el diámetro
 * esté bien definido (en un grafo dirigido, puede haber pares
 * inalcanzables incluso dentro de la componente débilmente conexa).
 */
Algoritmos::ResultadoDiametro Algoritmos::calcularDiametroVial(
    const Grafo& grafo, const std::vector<int>& componenteGigante)
{
    if (componenteGigante.empty()) {
        ResultadoDiametro vacio; vacio.nodoA = -1; vacio.nodoB = -1; vacio.distanciaMetros = 0.0;
        return vacio;
    }

    // Crear un set de la componente gigante para verificación rápida
    std::unordered_set<int> enComponente(componenteGigante.begin(),
                                          componenteGigante.end());

    int nodoActual = componenteGigante[0]; // Empezar con el primer nodo
    int mejorNodoA = nodoActual;
    int mejorNodoB = nodoActual;
    double mejorDistancia = 0.0;

    // Realizar 3 iteraciones del double-sweep
    const int ITERACIONES = 3;
    std::vector<int> previos;

    for (int iter = 0; iter < ITERACIONES; iter++) {
        std::cout << "    Iteracion " << (iter + 1) << "/" << ITERACIONES
                  << " - Dijkstra desde nodo " << nodoActual << "..." << std::flush;

        // Dijkstra desde nodoActual (no dirigido, por distancia)
        std::vector<double> distancias = dijkstra(
            grafo, nodoActual, false, true, previos);

        // Encontrar el nodo más lejano dentro de la componente gigante
        int nodoMasLejano = nodoActual;
        double distanciaMaxima = 0.0;

        for (int nodo : componenteGigante) {
            if (distancias[nodo] < INFINITO && distancias[nodo] > distanciaMaxima) {
                distanciaMaxima = distancias[nodo];
                nodoMasLejano = nodo;
            }
        }

        std::cout << " nodo mas lejano: " << nodoMasLejano
                  << " (dist: " << distanciaMaxima / 1000.0 << " km)" << std::endl;

        // Actualizar el mejor resultado
        if (distanciaMaxima > mejorDistancia) {
            mejorDistancia = distanciaMaxima;
            mejorNodoA = nodoActual;
            mejorNodoB = nodoMasLejano;
        }

        // Siguiente iteración: partir desde el nodo más lejano
        nodoActual = nodoMasLejano;
    }

    ResultadoDiametro resultado;
    resultado.nodoA = mejorNodoA;
    resultado.nodoB = mejorNodoB;
    resultado.distanciaMetros = mejorDistancia;
    return resultado;
}

// ====================================================================
// OBJETIVO 4: RED DE EMERGENCIA MÍNIMA (MST - KRUSKAL)
// ====================================================================
/**
 * Algoritmo de Kruskal para el Árbol de Cobertura Mínima (MST):
 *
 * 1. Filtrar aristas donde ambos extremos pertenecen a la componente gigante.
 * 2. Ordenar esas aristas por distancia (de menor a mayor).
 * 3. Recorrer las aristas en orden:
 *    - Si los dos extremos están en conjuntos distintos (Union-Find),
 *      agregar la arista al MST y unir los conjuntos.
 *    - Si ya están en el mismo conjunto, saltar (evitar ciclos).
 * 4. Terminar cuando el MST tenga (V-1) aristas.
 *
 * ¿Por qué Kruskal y no Prim?
 *   - Kruskal es más intuitivo: "tomar siempre la arista más corta
 *     que no forme ciclo".
 *   - La estructura Union-Find es un concepto fundamental que todo
 *     ingeniero de sistemas debe conocer.
 *   - Funciona naturalmente con la lista de aristas que ya tenemos.
 *
 * Complejidad: O(E log E) por el ordenamiento.
 *   El Union-Find opera en O(α(n)) amortizado por operación.
 */
double Algoritmos::construirMST(
    const Grafo& grafo, const std::vector<int>& componenteGigante)
{
    if (componenteGigante.empty()) return 0.0;

    // Set de nodos en la componente gigante para filtrado rápido
    std::unordered_set<int> enComponente(componenteGigante.begin(),
                                          componenteGigante.end());

    // PASO 1: Filtrar aristas de la componente gigante
    std::cout << "    Filtrando aristas de la componente gigante..." << std::flush;
    const auto& todasAristas = grafo.obtenerAristasSinDireccion();
    std::vector<AristaSinDireccion> aristasComponente;

    for (const auto& arista : todasAristas) {
        if (enComponente.count(arista.nodoA) && enComponente.count(arista.nodoB)) {
            aristasComponente.push_back(arista);
        }
    }
    std::cout << " " << aristasComponente.size() << " aristas." << std::endl;

    // PASO 2: Ordenar por distancia (menor a mayor)
    std::cout << "    Ordenando aristas por distancia..." << std::flush;
    std::sort(aristasComponente.begin(), aristasComponente.end(),
        [](const AristaSinDireccion& a, const AristaSinDireccion& b) {
            return a.distanciaMetros < b.distanciaMetros;
        });
    std::cout << " Listo." << std::endl;

    // PASO 3: Kruskal con Union-Find
    std::cout << "    Ejecutando Kruskal..." << std::flush;
    int totalNodos = grafo.obtenerCantidadNodos();
    UnionFind conjuntos(totalNodos);

    double distanciaTotalMST = 0.0;
    int aristasEnMST = 0;
    int aristasNecesarias = (int)componenteGigante.size() - 1;

    for (const auto& arista : aristasComponente) {
        if (aristasEnMST >= aristasNecesarias) break;

        // Si los nodos están en conjuntos distintos, agregar la arista
        if (conjuntos.unir(arista.nodoA, arista.nodoB)) {
            distanciaTotalMST += arista.distanciaMetros;
            aristasEnMST++;
        }
    }

    std::cout << " Listo. " << aristasEnMST << " aristas en el MST." << std::endl;

    return distanciaTotalMST;
}

// ====================================================================
// OBJETIVO 5: RUTA POR TIPO DE HORARIO (DISTANCIA VS. TIEMPO)
// ====================================================================
/**
 * Ejecuta dos Dijkstra entre el mismo par de nodos:
 *   1. Peso = distancia en metros → ruta más corta en distancia
 *   2. Peso = tiempo en segundos  → ruta más rápida
 *
 * Esto demuestra cómo el criterio de optimización cambia la ruta:
 *   - La ruta más corta en distancia puede usar calles lentas (residenciales).
 *   - La ruta más rápida puede ser más larga pero usa vías rápidas (autopistas).
 */
std::pair<Algoritmos::ResultadoRuta, Algoritmos::ResultadoRuta>
Algoritmos::compararRutas(const Grafo& grafo, int nodoOrigen, int nodoDestino)
{
    ResultadoRuta rutaDistancia, rutaTiempo;

    // --- Dijkstra por DISTANCIA ---
    std::cout << "    Dijkstra por distancia..." << std::flush;
    std::vector<int> previosDistancia;
    std::vector<double> distancias = dijkstra(
        grafo, nodoOrigen, false, true, previosDistancia);

    rutaDistancia.camino = reconstruirCamino(nodoOrigen, nodoDestino, previosDistancia);
    calcularMetricasCamino(grafo, rutaDistancia.camino,
                           rutaDistancia.distanciaTotalMetros,
                           rutaDistancia.tiempoTotalSegundos);
    std::cout << " Listo." << std::endl;

    // --- Dijkstra por TIEMPO ---
    std::cout << "    Dijkstra por tiempo..." << std::flush;
    std::vector<int> previosTiempo;
    std::vector<double> tiempos = dijkstra(
        grafo, nodoOrigen, true, true, previosTiempo);

    rutaTiempo.camino = reconstruirCamino(nodoOrigen, nodoDestino, previosTiempo);
    calcularMetricasCamino(grafo, rutaTiempo.camino,
                           rutaTiempo.distanciaTotalMetros,
                           rutaTiempo.tiempoTotalSegundos);
    std::cout << " Listo." << std::endl;

    return std::make_pair(rutaDistancia, rutaTiempo);
}
