#include <iostream>
#include <utility>
#include <fstream>
#include <cstring>
#include <sstream>
#include <chrono>

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

    bool hasLeft() const {
        return left != -1;
    }

    bool hasRight() const {
        return right != -1;
    }
};

struct Header {
    long long root = -1; // Root node position
    long long next = -1; // Next removed node position

    bool hasNext() const {
        return next != -1;
    }
};

constexpr auto FILENAME = "../data.txt";
constexpr auto DATASET = "../sales_dataset.csv";

class Manager {
    static bool fileIsEmptyOrNonExistent(std::fstream &file) {
        if (!file) {
            return true;
        }
        return file.peek() == std::ifstream::traits_type::eof();
    }

    static void createFile(std::fstream &file) {
        Header header;
        file.open(FILENAME, std::ios::binary | std::ios::out);
        file.write(reinterpret_cast<char *>(&header), sizeof(Header));
    }

    static Header getHeader(std::fstream &file) {
        Header header;
        file.seekg(0);
        file.read(reinterpret_cast<char *>(&header), sizeof(Header));
        return header;
    }

    static void updateRootPos(std::fstream &file, long long pos) {
        file.seekp(0);
        file.write(reinterpret_cast<char *>(&pos), sizeof(long long));
    }

    static long long getRootPos(std::fstream &file) {
        long long root;
        file.seekg(0);
        file.read(reinterpret_cast<char *>(&root), sizeof(long long));
        return root;
    }

    static bool thereIsNotRoot(std::fstream &file) {
        return getRootPos(file) == -1;
    }

    static void updateHeaderNext(std::fstream &file, long long pos) {
        file.seekp(sizeof(long long));
        file.write(reinterpret_cast<char *>(&pos), sizeof(long long));
    }

    static Node getNode(std::fstream &file, const long long &pos) {
        Node node;
        file.seekg(pos);
        file.read(reinterpret_cast<char *>(&node), sizeof(Node));
        return node;
    }

    static long long appendNode(std::fstream &file, const Node &node) {
        Header header = getHeader(file);
        if (header.hasNext()) {
            const long long nextPos = header.next;
            const Node nextNode = getNode(file, nextPos);
            updateHeaderNext(file, nextNode.next);
            updateNode(file, node, nextPos);
            return nextPos;
        }
        file.seekp(0, std::ios::end);
        const long long pos = file.tellp();
        file.write(reinterpret_cast<const char *>(&node), sizeof(Node));
        return pos;
    }

    static void updateNode(std::fstream &file, const Node &node, const long long &pos) {
        file.seekp(pos);
        file.write(reinterpret_cast<const char *>(&node), sizeof(Node));
    }

    static int getHeight(std::fstream &file, const long long &pos) {
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

    static long long insert(std::fstream &file, Node &node, const long long &pos, const Record &record) {
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

    static Record search(std::fstream &file, const Node &node, const long long &id) {
        if (id == node.record.id) {
            std::cout << "Record with ID " << id << " found.\n";
            return node.record;
        }
        if (id < node.record.id) {
            if (node.hasLeft()) {
                return search(file, getNode(file, node.left), id);
            }
        } else if (id > node.record.id) {
            if (node.hasRight()) {
                return search(file, getNode(file, node.right), id);
            }
        }
        std::cout << "Record with ID " << id << " not found.\n";
        return {};
    }

    static void searchRange(std::fstream &file, const long long &pos, const long long &min, const long long &max,
                            std::vector<Record> &records) {
        if (pos == -1) {
            return;
        }
        const Node node = getNode(file, pos);
        if (min < node.record.id) {
            searchRange(file, node.left, min, max, records);
        }
        if (min <= node.record.id && max >= node.record.id) {
            records.push_back(node.record);
        }
        if (max > node.record.id) {
            searchRange(file, node.right, min, max, records);
        }
    }

    static std::pair<Node, long long> findMin(std::fstream &file, long long pos) {
        Node node = getNode(file, pos);
        while (node.hasLeft()) {
            pos = node.left;
            node = getNode(file, pos);
        }
        return {node, pos};
    }

    static long long remove(std::fstream &file, Node &node, const long long &pos, const long long &id) {
        if (id < node.record.id) {
            if (!node.hasLeft()) {
                std::cout << "Record with ID " << id << " not found.\n";
                return pos;
            }
            Node leftChild = getNode(file, node.left);
            node.left = remove(file, leftChild, node.left, id);
        } else if (id > node.record.id) {
            if (!node.hasRight()) {
                std::cout << "Record with ID " << id << " not found.\n";
                return pos;
            }
            Node rightChild = getNode(file, node.right);
            node.right = remove(file, rightChild, node.right, id);
        } else {
            if (!node.hasLeft() && !node.hasRight()) {
                Header header = getHeader(file);
                node.next = header.next;
                updateHeaderNext(file, pos);
                updateNode(file, node, pos);
                return -1;
            }
            if (!node.hasLeft()) {
                Header header = getHeader(file);
                node.next = header.next;
                updateHeaderNext(file, pos);
                updateNode(file, node, pos);
                return node.right;
            }
            if (!node.hasRight()) {
                Header header = getHeader(file);
                node.next = header.next;
                updateHeaderNext(file, pos);
                updateNode(file, node, pos);
                return node.left;
            }
            auto [successor, successorPos] = findMin(file, node.right);
            node.record = successor.record;
            Node rightChild = getNode(file, node.right);
            node.right = remove(file, rightChild, node.right, successor.record.id);
            updateHeight(file, node);
            return balance(file, node, pos);
        }
        updateHeight(file, node);
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

    void insert(const Record &record) {
        std::fstream file(FILENAME, std::ios::binary | std::ios::in | std::ios::out);
        if (thereIsNotRoot(file)) {
            const long long rootPos = appendNode(file, {record});
            updateRootPos(file, rootPos);
            return;
        }
        long long rootPos = getRootPos(file);
        Node root = getNode(file, rootPos);
        rootPos = insert(file, root, rootPos, record);
        updateRootPos(file, rootPos);
        file.close();
    }

    Record search(const long long &id) {
        std::fstream file(FILENAME, std::ios::binary | std::ios::in);
        if (thereIsNotRoot(file)) {
            std::cout << "File has no records.\n";
            return {};
        }
        const long long rootPos = getRootPos(file);
        const Node root = getNode(file, rootPos);
        return search(file, root, id);
    }

    void remove(const long long &id) {
        std::fstream file(FILENAME, std::ios::binary | std::ios::in | std::ios::out);
        if (thereIsNotRoot(file)) {
            std::cout << "File has no records.\n";
            return;
        }
        long long rootPos = getRootPos(file);
        Node root = getNode(file, rootPos);
        rootPos = remove(file, root, rootPos, id);
        updateRootPos(file, rootPos);
        file.close();
    }

    std::vector<Record> searchRange(const long long &min, const long long &max) {
        std::fstream file(FILENAME, std::ios::binary | std::ios::in);
        std::vector<Record> records = {};

        if (thereIsNotRoot(file)) {
            std::cout << "File has no records.\n";
            return records;
        }
        long long rootPos = getRootPos(file);
        searchRange(file, rootPos, min, max, records);
        return records;
    }

    void loadCSV(const std::string &dataset = DATASET) {
        std::ifstream csv(dataset);
        if (!csv.is_open()) {
            std::cout << "CSV file could not be opened.\n";
            return;
        }

        std::string line;
        std::getline(csv, line);

        while (std::getline(csv, line)) {
            std::istringstream ss(line);
            std::string token;
            Record record;

            // ID
            std::getline(ss, token, ',');
            record.id = std::stoi(token);

            // Name
            std::getline(ss, token, ',');
            std::strncpy(record.name, token.c_str(), sizeof(record.name) - 1);
            record.name[sizeof(record.name) - 1] = '\0';

            // Sold
            std::getline(ss, token, ',');
            record.sold = std::stoi(token);

            // Price
            std::getline(ss, token, ',');
            record.price = std::stof(token);

            // Date
            std::getline(ss, token, ',');
            std::strncpy(record.date, token.c_str(), sizeof(record.date) - 1);
            record.date[sizeof(record.date) - 1] = '\0';

            insert(record);
        }
        csv.close();
    }
};

int main() {
    Manager manager;

    auto start = std::chrono::high_resolution_clock::now();
    manager.loadCSV();
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout << "CSV file loaded in " << duration.count() << " ns\n\n";

    start = std::chrono::high_resolution_clock::now();
    Record record = manager.search(250);
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout << "ID: " << record.id << ", Name: " << record.name
            << ", Sold: " << record.sold << ", Price: " << record.price
            << ", Date: " << record.date << "\n";
    std::cout << "Search time: " << duration.count() << " ns\n\n";

    start = std::chrono::high_resolution_clock::now();
    std::vector<Record> records = manager.searchRange(250, 750);
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout << "Total records found in range [250, 750]: " << records.size() << "\n";
    std::cout << "Range search time: " << duration.count() << " ns\n\n";

    start = std::chrono::high_resolution_clock::now();
    for (int i = 1; i <= 1000; i++) {
        manager.remove(i);
    }
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout << "All records removed in " << duration.count() << " ns\n\n";
    return 0;
}
