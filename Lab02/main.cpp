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
        Header header;
        file.open(FILENAME, std::ios::binary | std::ios::app);
        file.write(reinterpret_cast<char *>(&header), sizeof(Header));
    }

    static Header getHeader(std::fstream &file) {
        Header header;
        file.seekg(0);
        file.read(reinterpret_cast<char *>(&header), sizeof(Header));
        return header;
    }

    static void updateHeader(std::fstream &file, const Header &header) {
        file.seekp(0);
        file.write(reinterpret_cast<const char *>(&header), sizeof(Header));
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

    static void updateNode(std::fstream &file, const Node &node, const long long &pos) {
        file.seekp(pos);
        file.write(reinterpret_cast<const char *>(&node), sizeof(Node));
    }

    static int getHeight(std::fstream &file, const long long pos) {
        return pos == -1 ? -1 : getNode(file, pos).height;
    }

    static void updateHeight(std::fstream &file, Node &node) {
        node.height = 1 + std::max(getHeight(file, node.left), getHeight(file, node.right));
    }

    static int getBalance(std::fstream &file, const Node &node) {
        return getHeight(file, node.left) - getHeight(file, node.right);
    }

    static long long LL(std::fstream &file, Node &A, const long long &posA) {
        const long long posB = A.left;
        Node B = getNode(file, posB);

        A.left = B.right;
        B.right = posA;

        updateHeight(file, A);
        updateHeight(file, B);

        updateNode(file, A, posA);
        updateNode(file, B, posB);

        return posB;
    }

    static long long LR(std::fstream &file, Node &A, const long long &posA) {
        const long long posB = A.left;
        Node B = getNode(file, posB);
        const long long posC = B.right;
        Node C = getNode(file, posC);

        B.right = C.left;
        C.left = posB;
        A.left = C.right;
        C.right = posA;

        updateHeight(file, A);
        updateHeight(file, B);
        updateHeight(file, C);

        updateNode(file, A, posA);
        updateNode(file, B, posB);
        updateNode(file, C, posC);

        return posC;
    }

    static long long RR(std::fstream &file, Node &A, const long long &posA) {
        const long long posB = A.right;
        Node B = getNode(file, posB);

        A.right = B.left;
        B.left = posA;

        updateHeight(file, A);
        updateHeight(file, B);

        updateNode(file, A, posA);
        updateNode(file, B, posB);

        return posB;
    }

    static long long RL(std::fstream &file, Node &A, const long long &posA) {
        const long long posB = A.right;
        Node B = getNode(file, posB);
        const long long posC = B.left;
        Node C = getNode(file, posC);

        B.left = C.right;
        C.right = posB;
        A.right = C.left;
        C.left = posA;

        updateHeight(file, A);
        updateHeight(file, B);
        updateHeight(file, C);

        updateNode(file, A, posA);
        updateNode(file, B, posB);
        updateNode(file, C, posC);

        return posC;
    }

    static long long balance(std::fstream &file, Node &node, const long long &pos) {
        const int factor = getBalance(file, node);
        if (factor > 1) {
            const Node leftChild = getNode(file, node.left);
            if (getBalance(file, leftChild) >= 0) {
                return LL(file, node, pos);
            }
            return LR(file, node, pos);
        }
        if (factor < -1) {
            const Node rightChild = getNode(file, node.right);
            if (getBalance(file, rightChild) <= 0) {
                return RR(file, node, pos);
            }
            return RL(file, node, pos);
        }
        updateNode(file, node, pos);
        return pos;
    }

    static long long insert(std::fstream &file, Node &node, const long long pos, const Record &record) {
        if (record.id == node.record.id) {
            std::cout << "Record with ID " << record.id << " already exists.\n";
            return pos;
        }
        if (record.id < node.record.id) {
            if (node.hasLeft()) {
                Node leftChild = getNode(file, node.left);
                node.left = insert(file, leftChild, node.left, record);
            } else {
                const Node newNode = {record};
                node.left = appendNode(file, newNode);
            }
        } else {
            if (node.hasRight()) {
                Node rightChild = getNode(file, node.right);
                node.right = insert(file, rightChild, node.right, record);
            } else {
                const Node newNode = {record};
                node.right = appendNode(file, newNode);
            }
        }
        updateHeight(file, node);
        updateNode(file, node, pos);
        return balance(file, node, pos);
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
            const Node root(record);
            const long long rootPos = appendNode(file, root);
            header.root = rootPos;
            updateHeader(file, header);
            return;
        }
        const long long rootPos = getRootPosition(file);
        Node root = getNode(file, rootPos);
        Header header = getHeader(file);
        header.root = insert(file, root, rootPos, record);
        updateHeader(file, header);
    }
};

int main() {
    return 0;
}
