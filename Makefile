CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall

SRCDIR = src
SOURCES = $(SRCDIR)/LectorCSV.cpp $(SRCDIR)/Grafo.cpp $(SRCDIR)/Algoritmos.cpp $(SRCDIR)/main.cpp
TARGET = red_vial

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES) -I$(SRCDIR)

clean:
	rm -f $(TARGET) $(TARGET).exe

.PHONY: all clean
