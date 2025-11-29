#include "adiel.h"
#include <ctime>

// zonas prohibidas
static Rect zonaA{204,150,653,509};
static Rect zonaB{963,193,1412,642};
static Rect zonaC{425,1310,1224,1759};
static Rect zonaPalanca{1160,840,1459,1139};

ObjectSpawner::ObjectSpawner(int minX, int maxX, int minY, int maxY)
    : distX(minX, maxX),
      distY(minY, maxY),
      rng(static_cast<unsigned>(time(nullptr))) {}

bool ObjectSpawner::validarZona(int x, int y, const Rect& zona) {
    return !zona.contains(x, y);
}

std::vector<Objeto> ObjectSpawner::generarObjetos() {

    std::vector<Objeto> lista;

    // --- Colocar LLAVES ---
    auto colocarSeguro = [&](std::string name, const Rect& z) {
        int x, y;
        do {
            x = distX(rng);
            y = distY(rng);
        } while(!validarZona(x, y, z));
        lista.push_back({name, x, y});
    };

    colocarSeguro("LlaveA", zonaA);
    colocarSeguro("LlaveB", zonaB);
    colocarSeguro("LlaveC", zonaC);
    colocarSeguro("Palanca", zonaPalanca);

    // --- Resto de objetos (sin restricciones) ---
    for(int i = 1; i <= 11; i++){
        int x = distX(rng);
        int y = distY(rng);
        lista.push_back({"Objeto" + std::to_string(i), x, y});
    }

    return lista;
}
