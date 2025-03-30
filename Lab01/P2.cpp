#include <iostream>
#include <ios>
#include <utility>
#include <vector>

// Registro Matrícula con tamaño variable
struct Matricula {
    std::string codigo;
    int ciclo;
    double mensualidad;
    std::string observaciones;
};

// Metadata para cada registro
struct Metadata {
    size_t pos; // Posición del registro en el archivo
    size_t size; // Tamaño del registro
    bool active; // Indica si el registro está activo o no
};

class Manager {
    std::string dataFilename;
    std::string metaFilename;

public:
    Manager(std::string data, std::string meta) : dataFilename(std::move(data)), metaFilename(std::move(meta)) {
    }
};

int main() {
    return 0;
}
