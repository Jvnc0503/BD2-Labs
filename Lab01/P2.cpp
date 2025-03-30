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
        // Move to record position
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
        // Close file
        dataFile.close();

        return records;
    }

    void add(const Matricula &record) {
        std::ofstream dataFile(dataFilename, std::ios::binary | std::ios::app);
        std::ofstream metaFile(metaFilename, std::ios::binary | std::ios::app);

        // Get current position in data file
        size_t currentPos = dataFile.tellp();

        // Get fields sizes
        int codSize = static_cast<int>(record.codigo.size());
        int obsSize = static_cast<int>(record.observaciones.size());

        // Get record total size
        size_t recordSize = (sizeof(int) + codSize) + sizeof(int) + sizeof(double) + (sizeof(int) + obsSize);

        // Write record to data file
        dataFile.write(reinterpret_cast<const char *>(&codSize), sizeof(int)); // Write "codigo" size
        dataFile.write(record.codigo.c_str(), codSize);
        dataFile.write(reinterpret_cast<const char *>(&record.ciclo), sizeof(int));
        dataFile.write(reinterpret_cast<const char *>(&record.mensualidad), sizeof(double));
        dataFile.write(reinterpret_cast<const char *>(&obsSize), sizeof(int)); // Write "observaciones" size
        dataFile.write(record.observaciones.c_str(), obsSize);

        // Append metadata entry
        Metadata metaEntry = {currentPos, recordSize, true};
        metaFile.write(reinterpret_cast<const char *>(&metaEntry), sizeof(Metadata));

        // Close files
        dataFile.close();
        metaFile.close();
    }

    Matricula readRecord(const size_t &pos) const {
        // Load metadata
        auto metadata = loadMetadata();

        // Check if position is valid
        if (pos >= metadata.size()) {
            throw std::runtime_error("Invalid record position");
        }

        // Open data file
        std::ifstream dataFile(dataFilename, std::ios::binary);

        // Return record read from position
        return readFromPos(dataFile, metadata[pos].pos);
    }

    void remove(const size_t &pos) {
        // Load metadata
        auto metadata = loadMetadata();

        // Check if position is valid
        if (pos >= metadata.size()) {
            throw std::runtime_error("Invalid record position");
        }

        // Overwrite metadata
        metadata[pos].active = false;
        std::ofstream metaFile(metaFilename, std::ios::binary | std::ios::trunc);
        for (const auto &entry: metadata) {
            metaFile.write(reinterpret_cast<const char *>(&entry), sizeof(Metadata));
        }

        // Close file
        metaFile.close();
    }
};

int main() {
    Manager manager("data.bin", "metadata.bin");
    manager.add({"A123", 1, 500.50, "First semester"});
    manager.add({"B456", 2, 600.75, "Second semester"});

    auto records = manager.load();
    for (const auto &record : records) {
        std::cout << "Codigo: " << record.codigo << ", Ciclo: " << record.ciclo
                  << ", Mensualidad: " << record.mensualidad
                  << ", Observaciones: " << record.observaciones << std::endl;
    }

    return 0;
}
