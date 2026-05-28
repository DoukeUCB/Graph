#include "RoadNetwork.h"

#include "DisjointSetUnion.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <queue>

RoadNetwork::RoadNetwork()
    : contiguousIds(false),
      minNodeId(0),
      maxNodeId(-1),
      edgesRowCount(0) {}

RoadNetwork::LoadTimings RoadNetwork::loadFromCsv(
    const std::string& nodesPath,
    const std::string& edgesPath
) {
    using Clock = std::chrono::high_resolution_clock;
    auto t0 = Clock::now();
    loadNodes(nodesPath);
    auto t1 = Clock::now();
    loadEdges(edgesPath);
    auto t2 = Clock::now();

    LoadTimings timings;
    timings.readNodesMs =
        std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    timings.readEdgesMs =
        std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    return timings;
}

int RoadNetwork::nodeCount() const {
    return static_cast<int>(nodeIds.size());
}

long long RoadNetwork::edgeRowCount() const {
    return edgesRowCount;
}

int RoadNetwork::nodeIndexFromId(int nodeId) const {
    if (contiguousIds) {
        return (nodeId >= 0 && nodeId < nodeCount()) ? nodeId : -1;
    }
    auto it = nodeIdToIndex.find(nodeId);
    return it == nodeIdToIndex.end() ? -1 : it->second;
}

int RoadNetwork::nodeIdFromIndex(int index) const {
    return contiguousIds ? index : nodeIds[index];
}

const RoadNetwork::ComponentSummary& RoadNetwork::components() const {
    return componentSummary;
}

RoadNetwork::ReachabilityResult RoadNetwork::computeReachableWithinMeters(
    int originIndex,
    double maxDistanceMeters
) const {
    std::vector<double> distance =
        runDijkstraDirected(originIndex, WeightType::Distance, maxDistanceMeters);
    long long count = 0;
    for (double d : distance) {
        if (d <= maxDistanceMeters) count++;
    }
    ReachabilityResult result;
    result.reachableNodeCount = count;
    return result;
}

RoadNetwork::DiameterResult RoadNetwork::computeDiameter(
    const std::string& mode,
    int exactThreshold,
    int seedIndex
) const {
    std::string modeUsed = mode;
    if (modeUsed != "exact" && modeUsed != "approx" && modeUsed != "auto") {
        modeUsed = "auto";
    }
    if (modeUsed == "auto") {
        modeUsed = (componentSummary.giantComponentSize <= exactThreshold) ? "exact" : "approx";
    }
    if (modeUsed == "exact" && componentSummary.giantComponentSize > exactThreshold) {
        modeUsed = "approx";
    }

    DiameterResult result;
    result.modeUsed = modeUsed;

    int seed = seedIndex;
    if (seed < 0 || seed >= nodeCount() || !componentSummary.inGiantComponent[seed]) {
        seed = componentSummary.giantComponentNodes.empty() ? 0 : componentSummary.giantComponentNodes[0];
    }

    if (modeUsed == "exact") {
        double best = -1.0;
        int bestU = -1;
        int bestV = -1;
        for (int u : componentSummary.giantComponentNodes) {
            std::vector<double> dist = runDijkstraUndirected(u);
            for (int v : componentSummary.giantComponentNodes) {
                if (dist[v] < 1e18 && dist[v] > best) {
                    best = dist[v];
                    bestU = u;
                    bestV = v;
                }
            }
        }
        result.diameterMeters = best;
        result.fromIndex = bestU;
        result.toIndex = bestV;
        return result;
    }

    std::vector<double> distFromSeed = runDijkstraUndirected(seed);
    int farA = seed;
    double farDist = -1.0;
    for (int v : componentSummary.giantComponentNodes) {
        if (distFromSeed[v] < 1e18 && distFromSeed[v] > farDist) {
            farDist = distFromSeed[v];
            farA = v;
        }
    }

    std::vector<double> distFromFarA = runDijkstraUndirected(farA);
    int farB = farA;
    double farDist2 = -1.0;
    for (int v : componentSummary.giantComponentNodes) {
        if (distFromFarA[v] < 1e18 && distFromFarA[v] > farDist2) {
            farDist2 = distFromFarA[v];
            farB = v;
        }
    }

    result.diameterMeters = farDist2;
    result.fromIndex = farA;
    result.toIndex = farB;
    return result;
}

RoadNetwork::MstResult RoadNetwork::computeMstLengthKm() const {
    std::vector<UndirectedEdgeRecord> edgesInGiant;
    edgesInGiant.reserve(undirectedEdges.size());
    for (const auto& e : undirectedEdges) {
        if (componentSummary.inGiantComponent[e.from] && componentSummary.inGiantComponent[e.to]) {
            edgesInGiant.push_back(e);
        }
    }
    std::sort(edgesInGiant.begin(), edgesInGiant.end(),
              [](const UndirectedEdgeRecord& a, const UndirectedEdgeRecord& b) {
                  return a.distanceMeters < b.distanceMeters;
              });

    DisjointSetUnion dsu(nodeCount());
    double totalMeters = 0.0;
    int edgesUsed = 0;
    for (const auto& e : edgesInGiant) {
        if (dsu.unite(e.from, e.to)) {
            totalMeters += e.distanceMeters;
            edgesUsed += 1;
            if (edgesUsed == componentSummary.giantComponentSize - 1) break;
        }
    }

    MstResult result;
    result.totalLengthKm = totalMeters / 1000.0;
    return result;
}

RoadNetwork::RouteComparisonResult RoadNetwork::compareDistanceVsTime(
    int originIndex,
    int destinationIndex
) const {
    DirectedDijkstraTrace distanceTrace =
        runDijkstraDirectedWithTrace(originIndex, WeightType::Distance);

    int destination = destinationIndex;
    if (destination < 0 || destination >= nodeCount()) {
        double maxDistance = -1.0;
        for (int i = 0; i < nodeCount(); ++i) {
            if (distanceTrace.distance[i] < 1e18 && distanceTrace.distance[i] > maxDistance) {
                maxDistance = distanceTrace.distance[i];
                destination = i;
            }
        }
    }

    RouteComparisonResult result;
    result.destinationIndex = destination;
    result.distanceShortestMeters = distanceTrace.distance[destination];
    std::pair<double, double> distanceRouteTotals =
        computePathTotals(distanceTrace, originIndex, destination);
    result.timeOnDistanceRouteSeconds = distanceRouteTotals.second;

    DirectedDijkstraTrace timeTrace =
        runDijkstraDirectedWithTrace(originIndex, WeightType::Time);
    result.timeShortestSeconds = timeTrace.distance[destination];
    std::pair<double, double> timeRouteTotals =
        computePathTotals(timeTrace, originIndex, destination);
    result.distanceOnTimeRouteMeters = timeRouteTotals.first;

    return result;
}

void RoadNetwork::splitCsvLine(const std::string& line, std::vector<std::string>& out) {
    out.clear();
    size_t start = 0;
    for (size_t i = 0; i <= line.size(); ++i) {
        if (i == line.size() || line[i] == ',') {
            out.emplace_back(line.substr(start, i - start));
            start = i + 1;
        }
    }
}

void RoadNetwork::trimCarriageReturn(std::string& s) {
    if (!s.empty() && s.back() == '\r') s.pop_back();
}

double RoadNetwork::parseSpeedKmh(const std::string& s) {
    double integerPart = 0.0;
    double fractionalPart = 0.0;
    double base = 1.0;
    bool found = false;
    bool inFraction = false;
    for (char c : s) {
        if (std::isdigit(static_cast<unsigned char>(c))) {
            found = true;
            int digit = c - '0';
            if (!inFraction) {
                integerPart = integerPart * 10.0 + digit;
            } else {
                base *= 10.0;
                fractionalPart += digit / base;
            }
        } else if (c == '.' && found && !inFraction) {
            inFraction = true;
        } else if (found) {
            break;
        }
    }
    return found ? (integerPart + fractionalPart) : -1.0;
}

double RoadNetwork::defaultSpeedKmh(const std::string& roadClass) {
    if (roadClass == "motorway") return 100.0;
    if (roadClass == "trunk") return 80.0;
    if (roadClass == "primary") return 60.0;
    if (roadClass == "secondary") return 60.0;
    if (roadClass == "tertiary") return 50.0;
    if (roadClass == "residential") return 30.0;
    if (roadClass == "path") return 20.0;
    return 20.0;
}

double RoadNetwork::secondsFromDistance(double distanceMeters, double speedKmh) {
    return distanceMeters / (speedKmh * 1000.0 / 3600.0);
}

void RoadNetwork::loadNodes(const std::string& nodesPath) {
    std::ifstream nodesFile(nodesPath.c_str());
    if (!nodesFile) {
        std::cerr << "No se pudo abrir " << nodesPath << "\n";
        std::exit(1);
    }

    std::string line;
    std::getline(nodesFile, line); // header
    std::vector<std::string> columns;
    columns.reserve(8);

    nodeIds.clear();
    nodeIds.reserve(900000);
    minNodeId = std::numeric_limits<int>::max();
    maxNodeId = -1;

    while (std::getline(nodesFile, line)) {
        if (line.empty()) continue;
        splitCsvLine(line, columns);
        if (columns.empty()) continue;
        trimCarriageReturn(columns[0]);
        int nodeId = std::stoi(columns[0]);
        nodeIds.push_back(nodeId);
        if (nodeId < minNodeId) minNodeId = nodeId;
        if (nodeId > maxNodeId) maxNodeId = nodeId;
    }
    nodesFile.close();

    contiguousIds = (minNodeId == 0 && maxNodeId == static_cast<int>(nodeIds.size()) - 1);
    nodeIdToIndex.clear();
    if (!contiguousIds) {
        nodeIdToIndex.reserve(static_cast<size_t>(nodeIds.size() * 1.3));
        for (int i = 0; i < static_cast<int>(nodeIds.size()); ++i) {
            nodeIdToIndex[nodeIds[i]] = i;
        }
    }
}

void RoadNetwork::loadEdges(const std::string& edgesPath) {
    std::ifstream edgesFile(edgesPath.c_str());
    if (!edgesFile) {
        std::cerr << "No se pudo abrir " << edgesPath << "\n";
        std::exit(1);
    }

    std::string line;
    std::getline(edgesFile, line); // header
    std::vector<std::string> columns;
    columns.reserve(10);

    directedAdjacency.assign(nodeCount(), std::vector<DirectedEdge>());
    undirectedAdjacency.assign(nodeCount(), std::vector<UndirectedEdge>());
    undirectedEdges.clear();
    undirectedEdges.reserve(600000);
    edgesRowCount = 0;

    DisjointSetUnion dsu(nodeCount());

    while (std::getline(edgesFile, line)) {
        if (line.empty()) continue;
        splitCsvLine(line, columns);
        if (columns.size() < 7) continue;
        for (size_t i = 0; i < columns.size(); ++i) {
            trimCarriageReturn(columns[i]);
        }

        int fromNodeId = std::stoi(columns[1]);
        int toNodeId = std::stoi(columns[2]);
        double distanceMeters = std::stod(columns[3]);
        const std::string& roadClass = columns[4];
        int onewayFlag = columns[5].empty() ? 0 : std::stoi(columns[5]);
        double speedKmh = parseSpeedKmh(columns[6]);
        if (speedKmh <= 0.0) speedKmh = defaultSpeedKmh(roadClass);
        double timeSeconds = secondsFromDistance(distanceMeters, speedKmh);

        int fromIndex = nodeIndexFromId(fromNodeId);
        int toIndex = nodeIndexFromId(toNodeId);
        if (fromIndex < 0 || toIndex < 0) continue;

        directedAdjacency[fromIndex].push_back({toIndex, distanceMeters, timeSeconds});
        if (onewayFlag == 0) {
            directedAdjacency[toIndex].push_back({fromIndex, distanceMeters, timeSeconds});
        }

        undirectedAdjacency[fromIndex].push_back({toIndex, distanceMeters});
        undirectedAdjacency[toIndex].push_back({fromIndex, distanceMeters});
        undirectedEdges.push_back({fromIndex, toIndex, distanceMeters});

        dsu.unite(fromIndex, toIndex);
        edgesRowCount += 1;
    }
    edgesFile.close();

    buildComponentSummary(dsu);
}

void RoadNetwork::buildComponentSummary(DisjointSetUnion& dsu) {
    componentRootByNode.assign(nodeCount(), -1);
    componentSizeByRoot.assign(nodeCount(), 0);

    for (int i = 0; i < nodeCount(); ++i) {
        int root = dsu.find(i);
        componentRootByNode[i] = root;
        componentSizeByRoot[root] += 1;
    }

    componentSummary = ComponentSummary();
    for (int i = 0; i < nodeCount(); ++i) {
        if (componentSizeByRoot[i] > 0) {
            componentSummary.componentCount += 1;
            if (componentSizeByRoot[i] > componentSummary.giantComponentSize) {
                componentSummary.giantComponentSize = componentSizeByRoot[i];
                componentSummary.giantComponentRoot = i;
            }
        }
    }
    componentSummary.islandCount = componentSummary.componentCount - 1;

    componentSummary.inGiantComponent.assign(nodeCount(), 0);
    componentSummary.giantComponentNodes.clear();
    componentSummary.giantComponentNodes.reserve(componentSummary.giantComponentSize);
    for (int i = 0; i < nodeCount(); ++i) {
        if (componentRootByNode[i] == componentSummary.giantComponentRoot) {
            componentSummary.inGiantComponent[i] = 1;
            componentSummary.giantComponentNodes.push_back(i);
        }
    }
}

std::vector<double> RoadNetwork::runDijkstraDirected(
    int sourceIndex,
    WeightType weight,
    double maxDistance
) const {
    const double INF = 1e18;
    std::vector<double> distance(nodeCount(), INF);
    using NodeDistance = std::pair<double, int>;
    std::priority_queue<NodeDistance, std::vector<NodeDistance>, std::greater<NodeDistance>> pq;

    distance[sourceIndex] = 0.0;
    pq.push(std::make_pair(0.0, sourceIndex));

    while (!pq.empty()) {
        NodeDistance top = pq.top();
        double currentDistance = top.first;
        int node = top.second;
        pq.pop();
        if (currentDistance != distance[node]) continue;
        if (maxDistance >= 0.0 && currentDistance > maxDistance) break;
        for (const auto& edge : directedAdjacency[node]) {
            double weightValue =
                (weight == WeightType::Time) ? edge.timeSeconds : edge.distanceMeters;
            double nextDistance = currentDistance + weightValue;
            if (nextDistance < distance[edge.to]) {
                distance[edge.to] = nextDistance;
                pq.push(std::make_pair(nextDistance, edge.to));
            }
        }
    }
    return distance;
}

RoadNetwork::DirectedDijkstraTrace RoadNetwork::runDijkstraDirectedWithTrace(
    int sourceIndex,
    WeightType weight
) const {
    const double INF = 1e18;
    DirectedDijkstraTrace trace;
    trace.distance.assign(nodeCount(), INF);
    trace.previous.assign(nodeCount(), -1);
    trace.previousDistanceEdge.assign(nodeCount(), 0.0);
    trace.previousTimeEdge.assign(nodeCount(), 0.0);

    using NodeDistance = std::pair<double, int>;
    std::priority_queue<NodeDistance, std::vector<NodeDistance>, std::greater<NodeDistance>> pq;

    trace.distance[sourceIndex] = 0.0;
    pq.push(std::make_pair(0.0, sourceIndex));

    while (!pq.empty()) {
        NodeDistance top = pq.top();
        double currentDistance = top.first;
        int node = top.second;
        pq.pop();
        if (currentDistance != trace.distance[node]) continue;
        for (const auto& edge : directedAdjacency[node]) {
            double weightValue =
                (weight == WeightType::Time) ? edge.timeSeconds : edge.distanceMeters;
            double nextDistance = currentDistance + weightValue;
            if (nextDistance < trace.distance[edge.to]) {
                trace.distance[edge.to] = nextDistance;
                trace.previous[edge.to] = node;
                trace.previousDistanceEdge[edge.to] = edge.distanceMeters;
                trace.previousTimeEdge[edge.to] = edge.timeSeconds;
                pq.push(std::make_pair(nextDistance, edge.to));
            }
        }
    }
    return trace;
}

std::vector<double> RoadNetwork::runDijkstraUndirected(int sourceIndex) const {
    const double INF = 1e18;
    std::vector<double> distance(nodeCount(), INF);
    using NodeDistance = std::pair<double, int>;
    std::priority_queue<NodeDistance, std::vector<NodeDistance>, std::greater<NodeDistance>> pq;

    distance[sourceIndex] = 0.0;
    pq.push(std::make_pair(0.0, sourceIndex));

    while (!pq.empty()) {
        NodeDistance top = pq.top();
        double currentDistance = top.first;
        int node = top.second;
        pq.pop();
        if (currentDistance != distance[node]) continue;
        for (const auto& edge : undirectedAdjacency[node]) {
            double nextDistance = currentDistance + edge.distanceMeters;
            if (nextDistance < distance[edge.to]) {
                distance[edge.to] = nextDistance;
                pq.push(std::make_pair(nextDistance, edge.to));
            }
        }
    }
    return distance;
}

std::pair<double, double> RoadNetwork::computePathTotals(
    const DirectedDijkstraTrace& trace,
    int sourceIndex,
    int destinationIndex
) {
    if (destinationIndex != sourceIndex && trace.previous[destinationIndex] == -1) {
        return {std::numeric_limits<double>::infinity(),
                std::numeric_limits<double>::infinity()};
    }
    double totalDistance = 0.0;
    double totalTime = 0.0;
    int node = destinationIndex;
    while (node != sourceIndex) {
        totalDistance += trace.previousDistanceEdge[node];
        totalTime += trace.previousTimeEdge[node];
        node = trace.previous[node];
        if (node == -1) {
            return {std::numeric_limits<double>::infinity(),
                    std::numeric_limits<double>::infinity()};
        }
    }
    return {totalDistance, totalTime};
}
