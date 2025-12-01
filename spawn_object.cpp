#include "adiel.h"
#include <ctime>
#include <cmath>
#include <limits>
#include <algorithm>

// El mapa en main.cpp se coloca en (300,200). Las zonas siguientes están
// definidas en coordenadas de imagen del mapa (0.1500, 0.2500).
// Convertimos esas zonas a coordenadas del mundo sumando el offset.
static const int MAP_OFFSET_X = 300;
static const int MAP_OFFSET_Y = 200;

// zonas prohibidas (valores en coordenadas del mapa + offset)
static Rect zonaA{204 + MAP_OFFSET_X, 150 + MAP_OFFSET_Y, 653 + MAP_OFFSET_X, 509 + MAP_OFFSET_Y};
static Rect zonaB{963 + MAP_OFFSET_X, 193 + MAP_OFFSET_Y, 1412 + MAP_OFFSET_X, 642 + MAP_OFFSET_Y};
static Rect zonaC{425 + MAP_OFFSET_X, 1310 + MAP_OFFSET_Y, 1224 + MAP_OFFSET_X, 1759 + MAP_OFFSET_Y};
static Rect zonaPalanca{1160 + MAP_OFFSET_X, 840 + MAP_OFFSET_Y, 1459 + MAP_OFFSET_X, 1139 + MAP_OFFSET_Y};

// Definir habitaciones (rectángulos) donde queremos que algunos objetos aparezcan.
// Ajusta estas coordenadas si tu mapa usa otras posiciones.
static std::vector<Rect> habitaciones = {
    {350, 260, 600, 460},   // habitación 1 (aprox)
    {720, 220, 1080, 480},  // habitación 2 (aprox)
    {440, 1330, 1180, 1700} // habitación 3 (aprox)
};

ObjectSpawner::ObjectSpawner(int minX, int maxX, int minY, int maxY)
    : distX(minX, maxX),
      distY(minY, maxY),
      rng(static_cast<unsigned>(time(nullptr))) {}

bool ObjectSpawner::validarZona(int x, int y, const Rect& zona) {
    return !zona.contains(x, y);
}

std::vector<Objeto> ObjectSpawner::generarObjetos() {

    std::vector<Objeto> lista;
    const int minSpacing = 75; // distancia mínima en píxeles entre objetos

    // --- Colocar LLAVES (fuera de habitaciones, en pasillos) ---
    auto colocarLlave = [&](std::string name) {
        int x, y;
        int attempts = 0;
        do {
            x = distX(rng);
            y = distY(rng);
            attempts++;
            if (attempts > 3000) break;
            
            // Verificar que NO esté dentro de ninguna habitación
            bool inRoom = false;
            for (const auto &h : habitaciones) {
                if (h.contains(x, y)) {
                    inRoom = true;
                    break;
                }
            }
            
            // Verificar que no esté en zonas prohibidas
            bool inZona = zonaA.contains(x, y) || zonaB.contains(x, y) || 
                         zonaC.contains(x, y) || zonaPalanca.contains(x, y);
            
            if (inRoom || inZona) continue;
            
            // Verificar espacio con otros objetos
            int minSq = minSpacing * minSpacing;
            bool tooClose = false;
            for (auto &o : lista) {
                int dx = o.x - x;
                int dy = o.y - y;
                if (dx*dx + dy*dy < minSq) {
                    tooClose = true;
                    break;
                }
            }
            if (tooClose) continue;
            
            break;
        } while(attempts < 3000);
        
        lista.push_back({name, x, y});
    };

    colocarLlave("LlaveA");
    colocarLlave("LlaveB");
    colocarLlave("LlaveC");
    colocarLlave("Palanca");

    // --- Resto de objetos (repartidos uniformemente) ---
    const int totalObjects = 11;
    
    // Validar posición para objetos generales: solo respeta minSpacing
    auto isValidForObjects = [&](int cx, int cy)->bool{
        int minSq = minSpacing * minSpacing;
        for (auto &o : lista) {
            int dx = o.x - cx;
            int dy = o.y - cy;
            if (dx*dx + dy*dy < minSq) return false;
        }
        return true;
    };

    // Generar candidatos de malla
    const int step = minSpacing;
    std::vector<std::pair<int,int>> allCandidates;
    for (int x = distX.min(); x <= distX.max(); x += step) {
        for (int y = distY.min(); y <= distY.max(); y += step) {
            allCandidates.emplace_back(x + (step/2), y + (step/2));
        }
    }
    std::shuffle(allCandidates.begin(), allCandidates.end(), rng);

    // Colocar objetos usando candidatos
    int candidateIdx = 0;
    for (int i = 1; i <= totalObjects; ++i) {
        int x = 0, y = 0;
        bool placed = false;

        // Intentar con candidatos de la malla
        while (candidateIdx < (int)allCandidates.size()) {
            auto [cx, cy] = allCandidates[candidateIdx++];
            if (isValidForObjects(cx, cy)) {
                x = cx;
                y = cy;
                placed = true;
                break;
            }
        }

        // Si no hay candidatos válidos, buscar aleatorios
        if (!placed) {
            for (int attempt = 0; attempt < 3000; ++attempt) {
                int fx = distX(rng);
                int fy = distY(rng);
                if (isValidForObjects(fx, fy)) {
                    x = fx;
                    y = fy;
                    placed = true;
                    break;
                }
            }
        }

        // Último recurso (nunca debería llegar aquí con suficiente espacio)
        if (!placed) {
            x = distX.min() + minSpacing;
            y = distY.min() + minSpacing;
        }

        lista.push_back({"Objeto" + std::to_string(i), x, y});
    }

    return lista;
}
