#pragma once

#include <limits>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

struct DirectedEdge {
    int to;
    double distanceMeters;
    double timeSeconds;
};

struct UndirectedEdge {
    int to;
    double distanceMeters;
};

struct UndirectedEdgeRecord {
    int from;
    int to;
    double distanceMeters;
};

enum class WeightType { Distance, Time };

class DisjointSetUnion;

class RoadNetwork {
public:
    struct ComponentSummary {
        int componentCount = 0;
        int islandCount = 0;
        int giantComponentSize = 0;
        int giantComponentRoot = -1;
        std::vector<int> giantComponentNodes;
        std::vector<char> inGiantComponent;
    };

    struct LoadTimings {
        long long readNodesMs = 0;
        long long readEdgesMs = 0;
    };

    struct ReachabilityResult {
        long long reachableNodeCount = 0;
    };

    struct DiameterResult {
        double diameterMeters = -1.0;
        int fromIndex = -1;
        int toIndex = -1;
        std::string modeUsed = "approx";
    };

    struct MstResult {
        double totalLengthKm = 0.0;
    };

    struct RouteComparisonResult {
        int destinationIndex = -1;
        double distanceShortestMeters = std::numeric_limits<double>::infinity();
        double timeOnDistanceRouteSeconds = std::numeric_limits<double>::infinity();
        double timeShortestSeconds = std::numeric_limits<double>::infinity();
        double distanceOnTimeRouteMeters = std::numeric_limits<double>::infinity();
    };

    RoadNetwork();

    LoadTimings loadFromCsv(const std::string& nodesPath, const std::string& edgesPath);
    int nodeCount() const;
    long long edgeRowCount() const;

    int nodeIndexFromId(int nodeId) const;
    int nodeIdFromIndex(int index) const;

    const ComponentSummary& components() const;

    ReachabilityResult computeReachableWithinMeters(int originIndex, double maxDistanceMeters) const;
    DiameterResult computeDiameter(const std::string& mode, int exactThreshold, int seedIndex) const;
    MstResult computeMstLengthKm() const;
    RouteComparisonResult compareDistanceVsTime(int originIndex, int destinationIndex) const;

private:
    struct DirectedDijkstraTrace {
        std::vector<double> distance;
        std::vector<int> previous;
        std::vector<double> previousDistanceEdge;
        std::vector<double> previousTimeEdge;
    };

    std::vector<int> nodeIds;
    std::unordered_map<int, int> nodeIdToIndex;
    bool contiguousIds;
    int minNodeId;
    int maxNodeId;

    std::vector<std::vector<DirectedEdge>> directedAdjacency;
    std::vector<std::vector<UndirectedEdge>> undirectedAdjacency;
    std::vector<UndirectedEdgeRecord> undirectedEdges;
    long long edgesRowCount;

    std::vector<int> componentRootByNode;
    std::vector<int> componentSizeByRoot;
    ComponentSummary componentSummary;

    static void splitCsvLine(const std::string& line, std::vector<std::string>& out);
    static void trimCarriageReturn(std::string& s);
    static double parseSpeedKmh(const std::string& s);
    static double defaultSpeedKmh(const std::string& roadClass);
    static double secondsFromDistance(double distanceMeters, double speedKmh);

    void loadNodes(const std::string& nodesPath);
    void loadEdges(const std::string& edgesPath);
    void buildComponentSummary(DisjointSetUnion& dsu);

    std::vector<double> runDijkstraDirected(
        int sourceIndex,
        WeightType weight,
        double maxDistance
    ) const;
    DirectedDijkstraTrace runDijkstraDirectedWithTrace(int sourceIndex, WeightType weight) const;
    std::vector<double> runDijkstraUndirected(int sourceIndex) const;

    static std::pair<double, double> computePathTotals(
        const DirectedDijkstraTrace& trace,
        int sourceIndex,
        int destinationIndex
    );
};
