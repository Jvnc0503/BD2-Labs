#include <iostream>
#include <utility>
#include <fstream>
#include <cstring>

struct Record {
    int id{};
    char name[30]{};
    int sold{};
    float price{};
    char date[10]{};
};

struct Node {
    Record record = {};
    int height = 0; // Height of the node
    long long left = -1; // Left child position
    long long right = -1; // Right child position
    long long next = -1; // Next removed
};

struct Header {
    long long root = -1; // Root node position
    long long next = -1; // Next removed node position
};

constexpr auto FILENAME = "data.txt";

class Manager {
    static bool fileIsEmptyOrNonExistent(std::fstream &file) {
        if (!file) {
            return true;
        }
        return file.peek() == std::ifstream::traits_type::eof();
    }

    static void createFile(std::fstream &file) {
        file.open(FILENAME, std::ios::binary | std::ios::app);
        Header header;
        file.write(reinterpret_cast<char *>(&header), sizeof(Header));
    }

    static Header getHeader(std::fstream &file) {
        Header header;
        file.seekg(0);
        file.read(reinterpret_cast<char *>(&header), sizeof(Header));
        return header;
    }

    static long long getRootPosition(std::fstream &file) {
        long long root;
        file.seekg(0);
        file.read(reinterpret_cast<char *>(&root), sizeof(long long));
        return root;
    }

    static bool thereIsNotRoot(std::fstream &file) {
        return getRootPosition(file) == -1;
    }

    static void insert(const Record &record, Node &node, std::fstream &file) {
    }

public:
    Manager() {
        std::fstream file(FILENAME, std::ios::binary | std::ios::in | std::ios::out);
        if (fileIsEmptyOrNonExistent(file)) {
            createFile(file);
        }
        file.close();
    }

    static void insert(const Record &record) {
        std::fstream file(FILENAME, std::ios::binary | std::ios::in | std::ios::out);
        if (thereIsNotRoot(file)) {
            Header header = getHeader(file);
            header.root = sizeof(Header);
            file.seekp(0);
            file.write(reinterpret_cast<char *>(&header), sizeof(Header));
            Node root = {record};
            file.seekp(header.root);
            file.write(reinterpret_cast<char *>(&root), sizeof(Node));
            return;
        }
        const long long rootPos = getRootPosition(file);
        Node root;
        file.seekg(rootPos);
        file.read(reinterpret_cast<char *>(&root), sizeof(Node));
        insert(record, root, file);
    }
};

int main() {
    return 0;
}
