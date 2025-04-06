#include <iostream>
#include <utility>
#include <fstream>
#include <cstring>

struct Alumno {
    char id[6];
    char name[10];
    unsigned cycle;
    long long left = -1;
    long long right = -1;
};

class Manager {
    std::string filename;
    const unsigned size = sizeof(Alumno);

    void addAux(std::fstream &file, Alumno &alumno, const long long head, const long long pos) {
        file.seekg(head * size);
        Alumno temp;
        file.read(reinterpret_cast<char *>(&temp), size);
        const int condition = std::strcmp(temp.name, alumno.name);
        if (condition == 0) {
            std::cout << "ID already present\n";
        } else if (condition > 0) {
            if (temp.right == -1) {
                temp.right = pos;
                file.seekp(head * size);
                file.write(reinterpret_cast<char *>(&temp), size);
                std::cout << "Record added successfully\n";
                return;
            }
            addAux(file, alumno, temp.right, pos);
        } else {
            if (temp.left == -1) {
                temp.left = pos;
                file.seekp(head * size);
                file.write(reinterpret_cast<char *>(&temp), size);
                std::cout << "Record added successfully\n";
                return;
            }
            addAux(file, alumno, temp.left, pos);
        }
    }

    Alumno searchAux(std::ifstream &file, const char id[6], const long long head) {
        file.seekg(head * size);
        Alumno temp;
        file.read(reinterpret_cast<char *>(&temp), size);
        const int condition = std::strcmp(temp.id, id);
        if (condition == 0) {
            std::cout << "Record found successfully\n";
            return temp;
        } else if (condition > 0) {
            if (temp.right == -1) {
                std::cout << "ID not found\n";
                return {};
            }
            return searchAux(file, id, temp.right);
        } else {
            if (temp.left == -1) {
                std::cout << "ID not found\n";
                return {};
            }
            return searchAux(file, id, temp.left);
        }
    }

public:
    explicit Manager(std::string filename) : filename(std::move(filename)) {
    }

    void add(Alumno &alumno) {
        std::fstream file(filename, std::ios::binary | std::ios::in | std::ios::out);
        if (!file) {
            file.open(filename, std::ios::binary | std::ios::out);
            file.close();
            file.open(filename, std::ios::binary | std::ios::in | std::ios::out);
        }
        file.seekp(0, std::ios::end);

        file.write(reinterpret_cast<char *>(&alumno), size);

        if (file.tellp() != size) {
            addAux(file, alumno, 0, file.tellp());
        }

        file.close();
    }

    Alumno search(const char id[6]) {
        std::ifstream file(filename, std::ios::binary);
        if (!file) {
            std::cout << "File not found\n";
        }
        return searchAux(file, id, 0);
    }
};

int main() {
    Alumno alumno = {"P-271", "Josimar", 5};
    Manager manager("test.txt");
    manager.add(alumno);
    Alumno result = manager.search("P-271");
    std::cout << result.id << '\n' << result.name << '\n' << result.cycle << '\n';
    return 0;
}
