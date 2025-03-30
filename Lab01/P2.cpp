#include <fstream>
#include <iostream>
#include <ios>
#include <utility>
#include <vector>

// Variable size record entry
struct Matricula {
    std::string codigo;
    int ciclo;
    double mensualidad;
    std::string observaciones;
};

// Metadata entry for each record
struct Metadata {
    size_t pos; // Record position in the file
    size_t size; // Record size in bytes
    bool active; // Determines if record is active or not
};

class Manager {
    std::string dataFilename;
    std::string metaFilename;

    // Loads metadata from file
    std::vector<Metadata> loadMetadata() const {
        std::vector<Metadata> metadata;
        std::ifstream metaFile(metaFilename, std::ios::binary);
        if (!metaFile) return metadata;

        Metadata entry;
        while (metaFile.read(reinterpret_cast<char *>(&entry), sizeof(Metadata))) {
            metadata.push_back(entry);
        }
        metaFile.close();
        return metadata;
    }

    static Matricula readFromPos(std::ifstream &file, const size_t &pos) {
        file.seekg(static_cast<std::streamoff>(pos), std::ios::beg);

        // Read "codigo" field
        int codSize;
        file.read(reinterpret_cast<char *>(&codSize), sizeof(int));
        std::string codigo(codSize, '\0');
        file.read(&codigo[0], codSize);

        // Read "ciclo" field
        int ciclo;
        file.read(reinterpret_cast<char *>(&ciclo), sizeof(int));

        // Read "mensualidad" field
        double mensualidad;
        file.read(reinterpret_cast<char *>(&mensualidad), sizeof(double));

        // Read "observaciones" field
        int obsSize;
        file.read(reinterpret_cast<char *>(&obsSize), sizeof(int));
        std::string observaciones(obsSize, '\0');
        file.read(&observaciones[0], obsSize);

        return {codigo, ciclo, mensualidad, observaciones};
    }

public:
    Manager(std::string data, std::string meta) : dataFilename(std::move(data)), metaFilename(std::move(meta)) {
    }

    std::vector<Matricula> load() const {
        std::vector<Matricula> records;
        auto metadata = loadMetadata();
        std::ifstream dataFile(dataFilename, std::ios::binary);

        for (const auto &entry: metadata) {
            if (entry.active) {
                records.emplace_back(readFromPos(dataFile, entry.pos));
            }
        }
        return records;
    }
};

int main() {
    return 0;
}
