# Rutas Óptimas en Red Vial Urbana — Bolivia

**Universidad:** UCB  
**Materia:** Analisis de Algoritmos  
**Dataset:** OpenStreetMap/Geofabrik — Bolivia Road Network  
**Lenguaje:** C++ (Compilador g++ MinGW)

---

## Portada

| Campo | Detalle |
|-------|---------|
| **Proyecto** | Rutas Óptimas en Red Vial Urbana |
| **Dataset** | OpenStreetMap/Geofabrik — Bolivia |
| **Nodos** | 887,233 intersecciones |
| **Aristas** | 588,902 segmentos de calle |
| **Lenguaje** | C++ con POO |

---

## 1. Introducción

Este proyecto modela la red vial de Bolivia como un **grafo ponderado dirigido** donde las intersecciones son nodos y los segmentos de calle son aristas con peso (distancia en metros y tiempo estimado en segundos). El objetivo es aplicar algoritmos clásicos de teoría de grafos para resolver problemas reales de análisis vial.

### ¿Por qué es importante?

- **Planificación urbana**: conocer qué zonas están conectadas y cuáles están aisladas.
- **Servicios de emergencia**: determinar la infraestructura mínima para cobertura total.
- **Navegación**: encontrar rutas óptimas bajo diferentes criterios.

---

## 2. Dataset

### Fuente
OpenStreetMap/Geofabrik — Bolivia (descarga libre).

### Archivos

| Archivo | Filas | Campos |
|---------|-------|--------|
| `nodes.csv` | 887,233 | `node_id`, `lat` (UTM-Y), `lon` (UTM-X) |
| `edges.csv` | 588,902 | `osm_id`, `from_id`, `to_id`, `distance_m`, `fclass`, `oneway`, `maxspeed` |

### Campos relevantes

- **`distance_m`**: distancia del segmento en metros, derivada de la geometría real de la vía.
- **`fclass`**: tipo de vía (residential, trunk, primary, secondary, motorway, etc.). Hay 28 tipos diferentes.
- **`oneway`**: 0 = bidireccional, 1 = sentido único.
- **`maxspeed`**: velocidad máxima permitida en km/h (rango: 1 a 110 km/h).

### Modelado como grafo

```
Nodo = intersección vial (con coordenadas UTM)
Arista = segmento de calle (con distancia y velocidad)
Peso 1 = distancia en metros
Peso 2 = tiempo en segundos = distancia / (velocidad / 3.6)
```

Si `oneway=0`, la arista se agrega en **ambos sentidos** (A→B y B→A).  
Si `oneway=1`, la arista se agrega solo en **un sentido** (A→B).

## 2.1 Procesamiento y Limpieza de Datos

El procesamiento inicial del dataset se realizó en dos fases principales mediante scripts de Python, priorizando la eficiencia computacional a través de operaciones vectoriales y asegurando la consistencia de los atributos necesarios para el cálculo de pesos.

### Extracción y Vectorización de la Red
Para generar los archivos CSV finales a partir del *shapefile* (`.shp`), se desarrolló un script optimizado (`process.py`) que evita los bucles tradicionales para procesar la información de forma más eficiente:

* **Filtrado de geometrías:** Se descartaron registros con geometrías nulas y se conservaron estrictamente aquellas de tipo `LineString` para asegurar la validez de los segmentos.
* **Eliminación de duplicados:** Se limpió el dataset conservando solo la primera aparición de cada identificador vial (`osm_id`), eliminando segmentos redundantes de la red.
* **Generación de Nodos y Aristas:** Utilizando bibliotecas como `geopandas`, `pandas` y `numpy`, se extrajeron las coordenadas de inicio y fin de cada segmento de forma vectorial. Esto permitió mapear nodos únicos y construir las relaciones de adyacencia (aristas) rápidamente.

### Limpieza e Imputación de Velocidades
Dado que el cálculo de la ruta más rápida depende de la velocidad máxima de cada vía, se identificó la ausencia de este atributo (`maxspeed`) en una gran parte del dataset original. Para solucionarlo, se implementó un segundo pipeline de limpieza (`max_speed_impute.py`):

* **Estandarización inicial:** Se forzó la conversión de la columna `maxspeed` a valores numéricos, transformando cadenas de texto irregulares a nulos y unificando las ausencias de datos a un valor de 0.
* **Imputación basada en la mediana:** Para los segmentos sin velocidad definida, se calculó e imputó la mediana correspondiente a su clasificación de vía (`fclass`), tomando en cuenta exclusivamente los registros válidos (velocidad > 0).

**Resultados de la imputación:**
De los 588,902 registros procesados, el algoritmo logró imputar con éxito la velocidad en 522,677 aristas. No obstante, 1,170 casos quedaron sin resolver debido a la ausencia total de referencias válidas para calcular la mediana en su respectiva clasificación.

| Clasificación (`fclass`) | Registros sin imputar |
|--------------------------|-----------------------|
| `track_grade1`           | 583                   |
| `cycleway`               | 560                   |
| `bridleway`              | 14                    |
| `unknown`                | 8                     |
| `busway`                 | 5                     |

**Tratamiento Manual:**
Para corregir los 1,170 registros detallados en la tabla superior, se procedió con una asignación manual. Los valores de velocidad máxima se introdujeron considerando las características de este tipo de caminos y la normativa de tránsito aplicable al contexto vial de Bolivia.

---

## 3. Objetivos

1. **Alcance vehicular**: ¿Cuántos nodos son alcanzables desde un punto en máximo 5 km de distancia de calle?
2. **Islas viales**: ¿Cuántas "islas" desconectadas tiene la red? ¿Qué tan grande es la componente principal?
3. **Diámetro vial**: ¿Cuál es la mayor distancia mínima entre dos intersecciones dentro de la componente gigante?
4. **Red de emergencia mínima**: ¿Cuál es la infraestructura mínima (MST) para conectar toda la componente gigante?
5. **Ruta por tipo de horario**: ¿Cómo difiere la ruta más corta en distancia vs. la más rápida en tiempo?

---

## 4. Desarrollo

### 4.1 Estructura de datos: Lista de Adyacencia

**¿Por qué lista de adyacencia y no matriz de adyacencia?**

Con 887,233 nodos, una matriz de adyacencia requeriría:

```
887,233 × 887,233 = ~787 mil millones de celdas
≈ 787 GB de memoria (con doubles)
```

Esto es imposible. La lista de adyacencia solo almacena las aristas existentes (~1.2 millones), usando aproximadamente **50 MB** de memoria.

```
Estructura: vector<vector<Vecino>>

Vecino {
    int nodoDestino;        // ¿A dónde lleva esta calle?
    double distanciaMetros; // ¿Qué tan larga es?
    double tiempoSegundos;  // ¿Cuánto tarda recorrerla?
}
```

**Acceso a vecinos**: O(1) por índice directo (los node_id son 0 a N-1).

### 4.2 Diseño OOP

El proyecto se divide en **6 clases**, cada una en su propio archivo:

| Clase | Responsabilidad | Archivo |
|-------|----------------|---------|
| `Nodo` | Representa una intersección (ID, coordenadas) | `Nodo.h` |
| `Arista` | Representa un segmento de calle (distancia, tipo, velocidad) | `Arista.h` |
| `Grafo` | Estructura principal: listas de adyacencia dirigida y no dirigida | `Grafo.h/.cpp` |
| `LectorCSV` | Lee y parsea los archivos CSV | `LectorCSV.h/.cpp` |
| `UnionFind` | Estructura de conjuntos disjuntos (para Kruskal y componentes) | `UnionFind.h` |
| `Algoritmos` | Todos los algoritmos como métodos estáticos | `Algoritmos.h/.cpp` |

---

## 5. Formulación Algorítmica

### 5.1 Objetivo 1 — Alcance Vehicular: Dijkstra con Corte

**Algoritmo elegido:** Dijkstra con cola de prioridad (min-heap).

**¿Por qué Dijkstra?**  
Es el algoritmo estándar para caminos más cortos desde un origen único con pesos no negativos. Las distancias de calles siempre son positivas, así que Dijkstra es la elección natural.

**Optimización — Corte por distancia:**  
En lugar de ejecutar Dijkstra completo (que exploraría los 887K nodos), cortamos la exploración cuando la distancia acumulada supera 5,000 metros. Gracias a la propiedad del min-heap, una vez que sacamos un nodo con distancia > 5 km, todos los nodos restantes en la cola también exceden ese límite, así que podemos detenernos inmediatamente.

**Estructura de datos:**
```
priority_queue<pair<double, int>, vector<...>, greater<...>>
                ^distancia  ^nodo                   ^min-heap
```

El min-heap garantiza que siempre extraemos el nodo con menor distancia acumulada.

**Técnica "Lazy Deletion":**  
En lugar de actualizar la prioridad de un nodo en la cola (operación costosa que requiere una estructura más compleja como un heap indexado), simplemente insertamos la nueva distancia. Cuando extraemos un nodo, verificamos si ya fue procesado; si sí, lo ignoramos.

**Complejidad:** O((V + E) × log V)  
Pero en la práctica, el corte a 5 km reduce drásticamente los nodos explorados.

### Pseudocódigo

```text
Función AlcanceVehicular(Grafo, nodoOrigen, maxDistancia):
    Crear ColaDePrioridad minHeap
    Crear Arreglo distancias inicializado en INFINITO
    Crear Arreglo procesado inicializado en FALSO
    
    distancias[nodoOrigen] = 0
    minHeap.insertar(distancia: 0, nodo: nodoOrigen)
    nodosAlcanzables = 0
    
    Mientras minHeap NO esté vacía:
        (distActual, nodoActual) = minHeap.extraerMinimo()
        
        Si procesado[nodoActual] es VERDADERO: 
            Continuar iteración
            
        // Optimización: Si la distancia supera el límite, detenemos la búsqueda
        Si distActual > maxDistancia: 
            Romper bucle
            
        procesado[nodoActual] = VERDADERO
        nodosAlcanzables = nodosAlcanzables + 1
        
        Para cada vecino en Grafo.obtenerVecinos(nodoActual):
            Si procesado[vecino] es FALSO:
                nuevaDist = distActual + vecino.distancia
                
                Si nuevaDist < distancias[vecino]:
                    distancias[vecino] = nuevaDist
                    minHeap.insertar(distancia: nuevaDist, nodo: vecino)
                    
    Retornar nodosAlcanzables
```

### 5.2 Objetivo 2 — Islas Viales: BFS

**Algoritmo elegido:** BFS (Breadth-First Search) iterativo.

**¿Por qué BFS y no DFS?**
- BFS es **iterativo** (usa una cola `queue`), no hay riesgo de desbordamiento de pila con 887K nodos. DFS recursivo podría provocar stack overflow.
- Ambos tienen la misma complejidad: **O(V + E)**.

**¿Por qué componentes débilmente conexas?**  
Usamos el grafo **no dirigido** (ignorando la dirección de las calles). Esto modela la realidad física: si existe una calle entre A y B, ambas intersecciones están "conectadas" físicamente aunque la calle sea de un solo sentido.

**Procedimiento:**
1. Recorrer todos los nodos del grafo.
2. Para cada nodo no visitado, iniciar un BFS usando la adyacencia no dirigida.
3. Cada BFS descubre una componente completa.
4. Registrar el tamaño de cada componente para identificar la gigante.

**Complejidad:** O(V + E) — cada nodo y arista se visita exactamente una vez.

### Pseudocódigo

```text
Función EncontrarComponentesConexas(Grafo):
    Crear Arreglo componenteDeNodo inicializado en -1
    cantidadComponentes = 0
    tamanoGigante = 0
    idComponenteGigante = -1
    
    Para nodoInicial desde 0 hasta Grafo.TotalNodos - 1:
        Si componenteDeNodo[nodoInicial] ya tiene componente asignada:
            Continuar iteración
            
        Si nodoInicial no tiene vecinos (nodo aislado):
            Continuar iteración
            
        Crear Cola colaBFS
        colaBFS.encolar(nodoInicial)
        componenteDeNodo[nodoInicial] = cantidadComponentes
        tamanoComponenteActual = 0
        
        Mientras colaBFS NO esté vacía:
            nodoActual = colaBFS.desencolar()
            tamanoComponenteActual = tamanoComponenteActual + 1
            
            Para cada vecino en Grafo.obtenerVecinosNoDirigidos(nodoActual):
                Si componenteDeNodo[vecino] == -1: // No visitado
                    componenteDeNodo[vecino] = cantidadComponentes
                    colaBFS.encolar(vecino)
        
        // Actualizar datos si es la componente más grande hasta ahora
        Si tamanoComponenteActual > tamanoGigante:
            tamanoGigante = tamanoComponenteActual
            idComponenteGigante = cantidadComponentes
            
        cantidadComponentes = cantidadComponentes + 1
        
    Retornar cantidadComponentes, tamanoGigante, y nodos que pertenecen a idComponenteGigante
```

### 5.3 Objetivo 3 — Diámetro Vial: Heurística Double-Sweep

**Problema:** El diámetro exacto requiere ejecutar Dijkstra desde **cada nodo**, es decir O(V × (V+E) × log V). Con V = 887K, esto tomaría días.

**Solución — Heurística "Double Sweep" (Barrido Doble):**

```
1. Elegir un nodo arbitrario u
2. Dijkstra desde u → encontrar el nodo más lejano v
3. Dijkstra desde v → encontrar el nodo más lejano w
4. Dijkstra desde w → refinar una vez más
5. La mayor distancia encontrada ≈ diámetro
```

**¿Por qué funciona bien en redes viales?**  
Las redes viales tienen una estructura "alargada" (no son grafos aleatorios). El nodo más lejano desde cualquier extremo tiende a estar en el otro extremo del grafo. La heurística converge en 2-3 iteraciones.

**Complejidad:** O(k × (V + E) × log V), con k = 3 iteraciones.

### Pseudocódigo

```text
Función CalcularDiametroVial(Grafo, componenteGigante):
    nodoActual = componenteGigante[primer_elemento]
    mejorDistancia = 0
    mejorNodoA = nodoActual
    mejorNodoB = nodoActual
    
    // Se realizan 3 iteraciones de la heurística
    Para iteración desde 1 hasta 3:
        // Se ejecuta Dijkstra clásico desde el nodo actual
        distancias = EjecutarDijkstra(Grafo, nodoActual)
        
        nodoMasLejano = nodoActual
        distanciaMaxima = 0
        
        // Encontrar el nodo que quedó más lejos
        Para cada nodo en componenteGigante:
            Si distancias[nodo] < INFINITO Y distancias[nodo] > distanciaMaxima:
                distanciaMaxima = distancias[nodo]
                nodoMasLejano = nodo
                
        // Actualizar el diámetro encontrado
        Si distanciaMaxima > mejorDistancia:
            mejorDistancia = distanciaMaxima
            mejorNodoA = nodoActual
            mejorNodoB = nodoMasLejano
            
        // El nodo más lejano se convierte en el origen para la siguiente iteración
        nodoActual = nodoMasLejano
        
    Retornar mejorNodoA, mejorNodoB, mejorDistancia
```

### 5.4 Objetivo 4 — Red de Emergencia: Kruskal con Union-Find

**Algoritmo elegido:** Kruskal.

**¿Por qué Kruskal y no Prim?**
- Kruskal es más **intuitivo**: "siempre tomar la arista más corta que no forme ciclo".
- Ya tenemos las aristas en una lista (del CSV), lo cual se adapta naturalmente a Kruskal.
- La estructura **Union-Find** es un concepto fundamental que todo ingeniero de sistemas debe dominar.

**Union-Find con optimizaciones:**

| Optimización | Qué hace | Efecto |
|-------------|----------|--------|
| Compresión de camino | Cada nodo apunta directamente a la raíz | Consultas casi O(1) |
| Unión por rango | El árbol más bajo se une bajo el más alto | Mantiene el árbol balanceado |

```
encontrar(x):
    si padre[x] ≠ x:
        padre[x] = encontrar(padre[x])  ← compresión de camino
    retornar padre[x]

unir(x, y):
    raizX = encontrar(x)
    raizY = encontrar(y)
    si raizX = raizY: retornar FALSO     ← ya conectados
    si rango[raizX] < rango[raizY]: intercambiar
    padre[raizY] = raizX                 ← unión por rango
```

**Procedimiento Kruskal:**
1. Filtrar aristas donde ambos extremos están en la componente gigante.
2. Ordenar las aristas por distancia (menor a mayor).
3. Para cada arista: si une dos conjuntos distintos, agregarla al MST.
4. Parar cuando el MST tenga V-1 aristas.

**Complejidad:** O(E × log E) dominado por el ordenamiento.

### Pseudocódigo

```text
Función ConstruirMST(Grafo, componenteGigante):
    Lista aristasComponente = Vacía
    
    // 1. Filtrar solo aristas que pertenezcan a la componente gigante
    Para cada arista en Grafo.todasLasAristas():
        Si arista.nodoOrigen y arista.nodoDestino están en componenteGigante:
            aristasComponente.agregar(arista)
            
    // 2. Ordenar las aristas por distancia de menor a mayor
    Ordenar aristasComponente según arista.distancia Ascendente
    
    // 3. Aplicar algoritmo de Kruskal
    Crear Estructura UnionFind(TotalNodos)
    distanciaTotalMST = 0
    aristasEnMST = 0
    aristasNecesarias = Tamaño(componenteGigante) - 1
    
    Para cada arista en aristasComponente:
        Si aristasEnMST >= aristasNecesarias: 
            Romper bucle // El árbol está completo
            
        // Si unir origen y destino no forma un ciclo, agregar al MST
        Si UnionFind.unir(arista.nodoOrigen, arista.nodoDestino) es EXITOSO:
            distanciaTotalMST = distanciaTotalMST + arista.distancia
            aristasEnMST = aristasEnMST + 1
            
    Retornar distanciaTotalMST
```

### 5.5 Objetivo 5 — Comparación de Rutas: Dijkstra con Pesos Diferentes

**Enfoque:** Ejecutar **dos Dijkstra** entre el mismo par de nodos, cambiando solo el peso:

| Configuración | Peso de arista | Optimiza |
|-------------|---------------|----------|
| Por distancia | `distanciaMetros` | Ruta más corta en km |
| Por tiempo | `tiempoSegundos = distancia × 3.6 / velocidad` | Ruta más rápida |

Esto demuestra cómo el **criterio de optimización** cambia la ruta elegida:
- La ruta más corta puede usar calles residenciales (lentas pero directas).
- La ruta más rápida puede ser más larga pero usa autopistas (rápidas).

---

## 6. Comparación de Enfoques: Dijkstra vs. Bellman-Ford

| Aspecto | Dijkstra | Bellman-Ford |
|---------|----------|-------------|
| **Complejidad** | O((V+E) × log V) | O(V × E) |
| **Con nuestro dataset** | ~20 millones ops | ~522 mil millones ops |
| **Tiempo estimado** | **< 0.1 seg** | **~500 seg** (8 min) |
| **Pesos negativos** |  No soporta |  Soporta |
| **Nuestro caso** |  Ideal (distancias > 0) |  Innecesario |

**Conclusión:** Dijkstra es la elección correcta para nuestro problema porque:
1. Las distancias de calle son siempre positivas (no hay pesos negativos).
2. Es ~5,000× más rápido que Bellman-Ford en nuestro dataset.
3. Bellman-Ford solo sería necesario si existieran "atajos" con distancia negativa, lo cual no tiene sentido físico en una red vial.

---

## 7. Complejidad Resumen

| Objetivo | Algoritmo | Complejidad | Tiempo real |
|----------|-----------|-------------|-------------|
| 1. Alcance vehicular | Dijkstra + corte | O((V+E) log V) truncado | **0.002 seg** |
| 2. Islas viales | BFS | O(V + E) | **0.094 seg** |
| 3. Diámetro vial | 3× Dijkstra (heurística) | O(3 × (V+E) log V) | **0.064 seg** |
| 4. MST (Kruskal) | Ordenamiento + Union-Find | O(E log E) | **0.027 seg** |
| 5. Comparación rutas | 2× Dijkstra | O(2 × (V+E) log V) | **0.042 seg** |

**Tiempo de carga del dataset:** 4.09 seg (lectura de 1.4 millones de líneas CSV).

---

## 8. Implementación

### Compilación

```bash
g++ -std=c++17 -O2 -Wall -o red_vial.exe src/LectorCSV.cpp src/Grafo.cpp src/Algoritmos.cpp src/main.cpp -Isrc
```

O simplemente ejecutar `compilar.bat` en Windows.

### Estructura del código

```
Graph/
├── data/
│   ├── edges.csv          ← 588,902 aristas
│   └── nodes.csv          ← 887,233 nodos
├── src/
│   ├── Nodo.h             ← Clase intersección (header-only)
│   ├── Arista.h           ← Clase segmento de calle (header-only)
│   ├── UnionFind.h        ← Conjuntos disjuntos (header-only)
│   ├── LectorCSV.h/cpp    ← Parser de archivos CSV
│   ├── Grafo.h/cpp        ← Grafo con listas de adyacencia
│   ├── Algoritmos.h/cpp   ← Todos los algoritmos
│   └── main.cpp           ← Punto de entrada con menú
├── Makefile               ← Para compilar con make
├── compilar.bat           ← Para compilar en Windows
└── reporte.md             ← Este documento
```

### Retos encontrados

1. **Compatibilidad del compilador**: MinGW 6.3 no soporta completamente C++17 (structured bindings). Se adaptó el código para usar `std::make_pair` y acceso `.first/.second`.
2. **Memoria**: Con 887K nodos y dos listas de adyacencia (dirigida y no dirigida), el uso de memoria es ~100 MB. Se usaron estructuras ligeras (`Vecino` con solo 3 campos) en lugar de copiar objetos `Arista` completos.
3. **Diámetro**: Calcular el diámetro exacto es inviable (O(V²)). La heurística double-sweep resuelve esto eficientemente.

---

## 9. Resultados

### Objetivo 1: Alcance Vehicular

```
Nodo origen: 0
Distancia máxima: 5.00 km
Nodos alcanzables: 2,525
Tiempo de ejecución: 0.002 seg
```

**Interpretación:** Desde la intersección 0, se puede llegar a 2,525 intersecciones recorriendo como máximo 5 km de calles. Este es un radio de acción local típico de un servicio de reparto o patrulla.

### Objetivo 2: Islas Viales

```
Número total de islas (componentes): 314,807
Tamaño de la componente gigante: 72,144 nodos (8.13%)
```

**Interpretación:** La red vial de Bolivia está muy **fragmentada**: hay más de 314 mil "islas" desconectadas. La componente gigante contiene solo el 8.13% de los nodos. Esto se debe a que el dataset incluye muchos caminos rurales, senderos y vías aisladas (tracks, paths, footways) que no están conectados con la red principal. En una red puramente urbana, la componente gigante sería mucho más grande.

### Objetivo 3: Diámetro Vial

```
Par de intersecciones: nodo 611806 ↔ nodo 681710
Distancia mínima entre ellos: 2,329.28 km
Tiempo de ejecución: 0.064 seg
```

**Interpretación:** Las dos intersecciones más "lejanas" dentro de la componente gigante están a 2,329 km de distancia por carretera. Esto es coherente con la extensión norte-sur de Bolivia (~1,500 km en línea recta), considerando que las carreteras no son rectas.

### Objetivo 4: Red de Emergencia Mínima

```
Distancia total del MST: 40,338.56 km
Aristas en el MST: 72,143
Tiempo de ejecución: 0.027 seg
```

**Interpretación:** Para conectar las 72,144 intersecciones de la componente gigante con la mínima infraestructura posible, se necesitan 72,143 segmentos de calle que suman 40,338.56 km. Esto representa la red de emergencia mínima: si se eliminara cualquier segmento del MST, alguna zona quedaría desconectada.

### Objetivo 5: Ruta Distancia vs. Tiempo

```
Nodo origen: 171868 → Nodo destino: 497672

Ruta más CORTA (por distancia):
  Distancia: 383.63 km
  Tiempo: 323 min 14 seg (~5h 23min)
  Nodos: 639

Ruta más RÁPIDA (por tiempo):
  Distancia: 386.27 km
  Tiempo: 310 min 35 seg (~5h 10min)
  Nodos: 690

Diferencia:
  La ruta rápida recorre 2.64 km MÁS
  La ruta rápida ahorra 12.65 min
```

**Interpretación:** La ruta optimizada por tiempo es 2.64 km más larga, pero ahorra casi 13 minutos. Esto ocurre porque prefiere vías rápidas (troncales, primarias con 80 km/h) aunque sean más largas, en lugar de vías residenciales cortas pero lentas (20 km/h). Este es exactamente el tipo de decisión que toma un GPS moderno.

---

## 10. Conclusiones

1. **La lista de adyacencia** es la estructura ideal para grafos grandes y dispersos. Con 887K nodos, una matriz de adyacencia sería imposible (~787 GB).

2. **Dijkstra** es el algoritmo correcto para este problema: los pesos son no negativos y su complejidad O((V+E) log V) permite ejecutar búsquedas sobre el grafo completo en milisegundos.

3. **La red vial de Bolivia está altamente fragmentada** (314K componentes), lo cual refleja la realidad geográfica del país: muchas zonas rurales con vías no conectadas a la red principal.

4. **El criterio de optimización importa**: la ruta más corta en distancia NO es la más rápida. Un sistema de navegación debe considerar el tipo de vía y la velocidad, no solo la distancia.

5. **Kruskal con Union-Find** es una combinación poderosa y elegante para construir árboles de cobertura mínima, con complejidad dominada por el ordenamiento O(E log E).

6. **Las heurísticas son necesarias** para problemas intratables como el diámetro de grafos grandes. La heurística double-sweep reduce el problema de O(V²) a O(V) con resultados prácticamente exactos en redes viales.

---

## 11. Referencias

1. **Cormen, T. H., Leiserson, C. E., Rivest, R. L., & Stein, C.** (2009). *Introduction to Algorithms* (3rd ed.). MIT Press. — Capítulos 22 (BFS), 23 (MST), 24 (Dijkstra).

2. **OpenStreetMap/Geofabrik.** Bolivia Road Network. Disponible en: [https://download.geofabrik.de/south-america/bolivia.html](https://download.geofabrik.de/south-america/bolivia.html)

3. **Dijkstra, E. W.** (1959). "A note on two problems in connexion with graphs". *Numerische Mathematik*, 1(1), 269–271.

4. **Kruskal, J. B.** (1956). "On the shortest spanning subtree of a graph and the traveling salesman problem". *Proceedings of the American Mathematical Society*, 7(1), 48–50.

5. **Handler, G. Y., & Zang, I.** (1980). "A dual algorithm for the constrained shortest path problem". *Networks*, 10(4), 293–309. — Sobre la heurística double-sweep para diámetros de grafos.

6. **Tarjan, R. E.** (1975). "Efficiency of a good but not linear set union algorithm". *Journal of the ACM*, 22(2), 215–225. — Análisis de Union-Find.
