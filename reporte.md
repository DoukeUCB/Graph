---
title: Rutas Optimas en Red Vial Urbana (Bolivia)
author: Douglas Mendez
date: 2026-05-28
---

# Portada
**Proyecto:** Rutas Optimas en Red Vial Urbana  
**Curso:** Analisis de Algoritmos 
**Estudiante:** Douglas Mendez  
**Fecha:** 2026-05-28

# Introduccion
Este trabajo modela una red vial real como grafo dirigido y aplica algoritmos clasicos de caminos mas cortos y conectividad. El objetivo es obtener metricas utiles (alcance, componentes, diametro y MST) y comparar rutas por distancia y por tiempo. Se prioriza una solucion simple y entendible, aun si no es la mas optima para el dataset completo.

# Dataset
**Fuente:** OpenStreetMap / Geofabrik (Bolivia).  
**Archivos:** `data/nodes.csv` y `data/edges.csv`.  

**Nodos:** intersecciones (node_id) con coordenadas (lat, lon).  
**Aristas:** segmentos de calle con campos principales:

- `from_id`, `to_id` (direccion de la via)
- `distance_m` (peso por distancia)
- `fclass` (tipo de via)
- `oneway` (si es de un solo sentido)
- `maxspeed` (km/h, puede estar vacio)

**Peso por tiempo:**  
Si `maxspeed` existe, se usa; si no, se asume segun `fclass`.  
Tiempo (seg) = `distance_m / (speed_kmh * 1000/3600)`.

**Velocidades por defecto:**
| fclass       | km/h |
| ----------- | ---- |
| motorway    | 100  |
| trunk       | 80   |
| primary     | 60   |
| secondary   | 60   |
| tertiary    | 50   |
| residential | 30   |
| path        | 20   |

# Objetivos
1. Alcance vehicular: contar nodos alcanzables en maximo 5 km por calle desde un origen.
2. Islas viales: componentes debilmente conexas, tamano de la componente principal y numero total de islas.
3. Diametro vial: par de intersecciones con mayor distancia minima dentro de la componente gigante.
4. Red de emergencia minima: MST sobre la componente gigante y distancia total (km).
5. Ruta por tipo de horario: comparar camino mas corto por distancia vs por tiempo.

# Formulacion Algoritmica
**Modelo de grafo:**  
G = (V, E) dirigido. Cada arista tiene:
- `w_d(e) = distance_m`
- `w_t(e) = time_s`

Si `oneway = 0`, se agrega arista en ambos sentidos.

**Objetivo 1 (alcance 5 km):**  
Se ejecuta Dijkstra desde el origen usando `distance_m`, pero se detienen expansiones cuando la distancia supera 5000 m. Se cuentan nodos con distancia <= 5000.

**Objetivo 2 (islas viales):**  
Se ignora la direccion (grafo no dirigido) y se usa DSU (Union-Find) para contar componentes. Se reporta cantidad y tamano de la mayor.

**Objetivo 3 (diametro):**  
Exacto: Dijkstra desde cada nodo de la componente gigante y tomar el maximo de distancias minimas (costoso).  
Pragmatico: usar doble Dijkstra (aproximado) cuando la componente es grande. En el codigo se activa exacto si `|GC| <= 20000`, si no se usa aproximacion para mantener tiempos decentes.

**Objetivo 4 (MST):**  
Se convierte a grafo no dirigido (se puede usar el minimo peso entre dos nodos). Se aplica Kruskal con `distance_m`.

**Objetivo 5 (distancia vs tiempo):**  
Se ejecuta Dijkstra dos veces entre origen y destino:  
1. con `distance_m`  
2. con `time_s`  
Se comparan longitud (m) y tiempo (s), y se analiza diferencia de ruta.

**Comparacion de enfoques:**  
Dijkstra vs Bellman-Ford: ambos sirven para caminos mas cortos, pero Dijkstra es mas rapido cuando no hay pesos negativos (este caso). Bellman-Ford solo se justifica si existieran pesos negativos.

# Complejidad
| Tarea | Algoritmo | Complejidad |
| ----- | --------- | ----------- |
| Alcance 5 km | Dijkstra con corte | O((V+E) log V) en el peor caso |
| Islas viales | DSU | O(V+E) |
| Diametro (exacto) | Dijkstra por nodo | O(V * (E log V)) |
| Diametro (aprox) | k Dijkstra | O(k * E log V) |
| MST | Kruskal | O(E log E) |
| Rutas d vs t | 2x Dijkstra | O(2 * E log V) |

# Implementacion (C++)
**Estructura de carpetas:**

```
Graph/
  data/         (nodes.csv, edges.csv)
  src/          (main.cpp, RoadNetwork.h/.cpp, DisjointSetUnion.h)
  build/        (graph)
  reporte.md
```

**Diseno POO (clases principales):**
- **RoadNetwork** (`RoadNetwork.h/.cpp`): carga el CSV, guarda el grafo dirigido/no dirigido y ejecuta los analisis.  
- **DisjointSetUnion** (`DisjointSetUnion.h`): encapsula Union-Find para componentes y MST.  
- **Estructuras de arista** (`DirectedEdge`, `UndirectedEdge`, `UndirectedEdgeRecord`).

**Estructuras de datos por objetivo:**
- **Adjacency list** (vector<vector<...>>): Dijkstra por distancia/tiempo.  
- **DSU** (Union-Find): componentes debiles y MST (Kruskal).  
- **Lista de aristas** (vector<UndirectedEdgeRecord>): ordenar por peso en Kruskal.  
- **Priority queue**: cola minima en Dijkstra.

**Algoritmos usados**
- Alcance 5 km: Dijkstra con corte por distancia.
- Islas: DSU sobre el grafo no dirigido.
- Diametro: exacto si la componente gigante es pequena; si no, aproximado con doble Dijkstra.
- MST: Kruskal en componente gigante.
- Ruta d vs t: dos Dijkstra (por distancia y por tiempo).

**Defensa tecnica (explicacion y por que):**
Si presento esto ante un profesor, explico asi:

- **Modelo y estructura**: la red vial es dispersa (muchos nodos pero pocos vecinos por nodo), por eso uso **listas de adyacencia** y no matriz. Me da memoria O(V+E) y acceso directo a vecinos, que es exactamente lo que necesita Dijkstra.
- **Alcance 5 km**: uso **Dijkstra** con corte en 5000 m. Dijkstra es el algoritmo mas simple y correcto para pesos no negativos. El corte evita expandir nodos que ya superan el limite, haciendo el tiempo decente sin perder exactitud en el conteo.
- **Islas viales**: uso **Union-Find** porque cada arista une dos nodos; con una pasada sobre las aristas, obtengo todas las componentes. Es O(V+E) y muy facil de justificar.
- **Diametro**: el exacto seria Dijkstra desde cada nodo de la componente gigante, lo cual es muy caro en escala nacional. Por eso aplico **doble Dijkstra** (aproximado) y dejo un umbral que activa el exacto solo si la componente es pequena. Mantengo tiempos decentes sin perder la idea del diametro.
- **MST**: uso **Kruskal** porque ya tengo la lista de aristas; ordenar por distancia y unir con DSU es directo, claro y correcto para un arbol de cobertura minima.
- **Ruta distancia vs tiempo**: corro Dijkstra dos veces con distinta funcion de peso. Esto permite comparar rutas y mostrar la diferencia entre optimizar distancia y optimizar tiempo.

**Compilacion y ejecucion (Linux / macOS / WSL):**
```
mkdir -p build
g++ -O2 -std=c++11 src/main.cpp src/RoadNetwork.cpp -o build/graph
./build/graph
```

**Parametros opcionales:**
- `--origin <node_id>`  
- `--dest <node_id>`  
- `--maxkm <km>`  
- `--diameter <auto|exact|approx>`  
- `--diam-threshold <n>`

# Resultados
Ejecucion sobre los CSV completos con el analizador en C++ (`src\main.cpp`).  
Origen usado: **node 0** (en la componente gigante).  
Destino usado: **node 611806** (mas lejano por distancia desde el origen).  
Diametro: modo **approx** (la componente gigante es grande).

- Nodos alcanzables en 5 km: **2525**  
- Componentes debiles: **314807** (islas: **314806**)  
- Tamano de la componente gigante: **72144**  
- Diametro aproximado: **2329279.41 m** (nodos **611806 -> 681710**)  
- Longitud total del MST: **40338.56 km**  
- Ruta por distancia vs tiempo (node 0 -> node 611806):  
  - Distancia minima: **1683390.77 m**  
  - Tiempo en esa ruta: **98037.18 s**  
  - Tiempo minimo: **96640.61 s**  
  - Distancia en la ruta mas rapida: **1687525.56 m**  
  - Diferencia: la ruta mas rapida ahorra **1396.57 s** (~23.28 min)  
    pero agrega **4.13 km** aprox.

**Tiempos de ejecucion (ms, en este equipo):**
- Lectura nodos: **505 ms**
- Lectura aristas y grafo: **939 ms**
- Alcance 5 km: **3 ms**
- Diametro (approx): **31 ms**
- MST: **11 ms**
- Rutas d vs t: **45 ms**

# Conclusiones
La red vial se modela naturalmente como grafo dirigido con pesos no negativos. Dijkstra es suficiente y simple para rutas y alcance. La conectividad se obtiene con DSU en version no dirigida. El diametro exacto es costoso en escala nacional, por lo que se usa aproximacion cuando la componente es grande. Los tiempos observados son decentes (del orden de segundos para lectura y milisegundos para cada objetivo), lo que cumple el requisito de simplicidad sin sacrificar utilidad.

# Referencias
- OpenStreetMap: https://www.openstreetmap.org  
- Geofabrik downloads: https://download.geofabrik.de/south-america/bolivia.html  
- Cormen et al., Introduction to Algorithms (Dijkstra, Kruskal, Bellman-Ford)
