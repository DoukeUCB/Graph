#include "RoadNetwork.h"

#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>

struct ProgramOptions {
    std::string nodesPath = "data/nodes.csv";
    std::string edgesPath = "data/edges.csv";
    int originNodeId = -1;
    int destinationNodeId = -1;
    double maxDistanceKm = 5.0;
    std::string diameterMode = "auto"; // auto | exact | approx
    int diameterExactThreshold = 20000;
};

void printUsage() {
    std::cout
        << "Uso: build/graph [opciones]\n"
        << "Opciones:\n"
        << "  --nodes <ruta>          (default: data/nodes.csv)\n"
        << "  --edges <ruta>          (default: data/edges.csv)\n"
        << "  --origin <node_id>      (default: primer nodo en componente gigante)\n"
        << "  --dest <node_id>        (default: mas lejano por distancia desde origin)\n"
        << "  --maxkm <km>            (default: 5)\n"
        << "  --diameter <auto|exact|approx> (default: auto)\n"
        << "  --diam-threshold <n>    (default: 20000)\n";
}

ProgramOptions parseArgs(int argc, char** argv) {
    ProgramOptions options;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--nodes" && i + 1 < argc) options.nodesPath = argv[++i];
        else if (arg == "--edges" && i + 1 < argc) options.edgesPath = argv[++i];
        else if (arg == "--origin" && i + 1 < argc) options.originNodeId = std::stoi(argv[++i]);
        else if (arg == "--dest" && i + 1 < argc)
            options.destinationNodeId = std::stoi(argv[++i]);
        else if (arg == "--maxkm" && i + 1 < argc)
            options.maxDistanceKm = std::stod(argv[++i]);
        else if (arg == "--diameter" && i + 1 < argc) options.diameterMode = argv[++i];
        else if (arg == "--diam-threshold" && i + 1 < argc)
            options.diameterExactThreshold = std::stoi(argv[++i]);
        else {
            printUsage();
            std::exit(1);
        }
    }
    return options;
}

int main(int argc, char** argv) {
    ProgramOptions options = parseArgs(argc, argv);
    using Clock = std::chrono::high_resolution_clock;

    RoadNetwork network;
    RoadNetwork::LoadTimings loadTimings =
        network.loadFromCsv(options.nodesPath, options.edgesPath);

    int originIndex = -1;
    if (options.originNodeId >= 0) {
        originIndex = network.nodeIndexFromId(options.originNodeId);
        if (originIndex < 0) {
            std::cerr << "origin_id no encontrado\n";
            return 1;
        }
    } else if (!network.components().giantComponentNodes.empty()) {
        originIndex = network.components().giantComponentNodes[0];
    } else {
        originIndex = 0;
    }

    int destinationIndex = -1;
    if (options.destinationNodeId >= 0) {
        destinationIndex = network.nodeIndexFromId(options.destinationNodeId);
        if (destinationIndex < 0) {
            std::cerr << "dest_id no encontrado\n";
            return 1;
        }
    }

    auto tReachStart = Clock::now();
    RoadNetwork::ReachabilityResult reach =
        network.computeReachableWithinMeters(originIndex, options.maxDistanceKm * 1000.0);
    auto tReachEnd = Clock::now();

    auto tDiameterStart = Clock::now();
    RoadNetwork::DiameterResult diameter =
        network.computeDiameter(options.diameterMode, options.diameterExactThreshold, originIndex);
    auto tDiameterEnd = Clock::now();

    auto tMstStart = Clock::now();
    RoadNetwork::MstResult mst = network.computeMstLengthKm();
    auto tMstEnd = Clock::now();

    auto tRouteStart = Clock::now();
    RoadNetwork::RouteComparisonResult routeComparison =
        network.compareDistanceVsTime(originIndex, destinationIndex);
    auto tRouteEnd = Clock::now();

    auto ms = [](Clock::time_point a, Clock::time_point b) {
        return std::chrono::duration_cast<std::chrono::milliseconds>(b - a).count();
    };

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "RESULTADOS\n";
    std::cout << "Nodos: " << network.nodeCount() << "\n";
    std::cout << "Aristas (filas): " << network.edgeRowCount() << "\n";
    std::cout << "Componentes debiles: " << network.components().componentCount
              << " (islas: " << network.components().islandCount << ")\n";
    std::cout << "Componente gigante: " << network.components().giantComponentSize << " nodos\n";
    std::cout << "Origen: " << network.nodeIdFromIndex(originIndex) << "\n";
    std::cout << "Alcance <= " << options.maxDistanceKm << " km: "
              << reach.reachableNodeCount << " nodos\n";
    std::cout << "Diametro (" << diameter.modeUsed << "): " << diameter.diameterMeters << " m";
    std::cout << " (nodos " << network.nodeIdFromIndex(diameter.fromIndex)
              << " -> " << network.nodeIdFromIndex(diameter.toIndex) << ")\n";
    std::cout << "MST (componente gigante): " << mst.totalLengthKm << " km\n";
    std::cout << "Destino: " << network.nodeIdFromIndex(routeComparison.destinationIndex) << "\n";
    std::cout << "Ruta mas corta (distancia): " << routeComparison.distanceShortestMeters << " m\n";
    std::cout << "Tiempo en esa ruta: " << routeComparison.timeOnDistanceRouteSeconds << " s\n";
    std::cout << "Ruta mas corta (tiempo): " << routeComparison.timeShortestSeconds << " s\n";
    std::cout << "Distancia en esa ruta: " << routeComparison.distanceOnTimeRouteMeters << " m\n";

    std::cout << "\nTIEMPOS (ms)\n";
    std::cout << "Lectura nodos: " << loadTimings.readNodesMs << "\n";
    std::cout << "Lectura aristas y grafo: " << loadTimings.readEdgesMs << "\n";
    std::cout << "Alcance 5 km: " << ms(tReachStart, tReachEnd) << "\n";
    std::cout << "Diametro: " << ms(tDiameterStart, tDiameterEnd) << "\n";
    std::cout << "MST: " << ms(tMstStart, tMstEnd) << "\n";
    std::cout << "Rutas d vs t: " << ms(tRouteStart, tRouteEnd) << "\n";

    return 0;
}
