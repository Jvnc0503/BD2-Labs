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

    bool hasLeft() {
        return left != -1;
    }

    bool hasRight() {
        return right != -1;
    }

    bool hasNext() {
        return next != -1;
    }
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

    static Node getNode(std::fstream &file, const long long &pos) {
        Node node;
        file.seekg(pos);
        file.read(reinterpret_cast<char *>(&node), sizeof(Node));
        return node;
    }

    static long long appendNode(std::fstream &file, const Node &node) {
        file.seekp(0, std::ios::end);
        const long long pos = file.tellp();
        file.write(reinterpret_cast<const char *>(&node), sizeof(Node));
        return pos;
    }

    static void writeNode(std::fstream &file, const Node &node, const long long &pos) {
        file.seekp(pos);
        file.write(reinterpret_cast<const char *>(&node), sizeof(Node));
    }

    static void insert(std::fstream &file, Node &node, long long pos, const Record &record) {
        if (record.id == node.record.id) {
            std::cout << "Record with ID " << record.id << " already exists.\n";
            return;
        }
        if (record.id < node.record.id) {
            if (node.hasLeft()) {
                Node leftChild = getNode(file, node.left);
                insert(file, leftChild, node.left, record);
            } else {
                const Node newNode(record);
                const long long newPos = appendNode(file, newNode);
                node.left = newPos;
                writeNode(file, node, pos);
            }
        } else {
            if (node.hasRight()) {
                Node rightChild = getNode(file, node.right);
                insert(file, rightChild, node.right, record);
            } else {
                const Node newNode(record);
                const long long newPos = appendNode(file, newNode);
                node.right = newPos;
                writeNode(file, node, pos);
            }
        }
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
            Node root(record);
            file.seekp(header.root);
            file.write(reinterpret_cast<char *>(&root), sizeof(Node));
            return;
        }
        Node root = getNode(file, getRootPosition(file));
        insert(file, root, getRootPosition(file), record);
    }
};

int main() {
    return 0;
}
