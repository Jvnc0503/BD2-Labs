#include <iostream>
#include <fstream>
#include <string>
#include <vector>

struct Record {
    int id;
};

constexpr int BUCKET_SIZE = 4;
constexpr auto FILENAME = "data.txt";
constexpr int BUCKETS = 5;

struct Bucket {
    Record records[BUCKET_SIZE] = {};
    int count = 0;
    long long next = -1;

    bool isFull() const {
        return count >= BUCKET_SIZE;
    }

    bool hasNext() const {
        return next != -1;
    }

    bool isEmpty() const {
        return count == 0;
    }
};

class Manager {
    static bool isFileEmptyOrNonExistent(std::fstream &file) {
        if (!file) {
            // File does not exist
            return true;
        }
        // Check if the file is empty
        return file.peek() == std::ifstream::traits_type::eof();
    }

    // Creates the file using append mode
    static void createFile(std::fstream &file) {
        file.open(FILENAME, std::ios::binary | std::ios::app);
    }

    // Writes initials buckets to the file
    static void createBuckets(std::fstream &file) {
        for (int i = 0; i < BUCKETS; ++i) {
            Bucket bucket;
            file.write(reinterpret_cast<char *>(&bucket), sizeof(bucket));
        }
    }

    // Simple hash function to map keys to bucket indexes
    static long long hash(const int &k) {
        return k % BUCKETS;
    }

    // Retrieves bucket from file at the given position
    static Bucket getBucket(const long long &pos, std::fstream &file) {
        Bucket bucket;
        file.seekg(pos);
        file.read(reinterpret_cast<char *>(&bucket), sizeof(Bucket));
        return bucket;
    }

    static void insert(const Record &record, Bucket &bucket, std::fstream &file, const long long &pos) {
        for (int i = 0; i < bucket.count; ++i) {
            if (bucket.records[i].id == record.id) {
                std::cout << "Record with ID " << record.id << " already exists\n";
                return;
            }
        }
        if (bucket.isFull()) {
            if (bucket.hasNext()) {
                Bucket nextBucket = getBucket(bucket.next, file);
                insert(record, nextBucket, file, bucket.next);
            } else {
                Bucket newBucket;
                newBucket.records[0] = record;
                newBucket.count = 1;

                file.seekp(0, std::ios::end);
                const long long nextPos = file.tellp();
                file.write(reinterpret_cast<char *>(&newBucket), sizeof(newBucket));

                bucket.next = nextPos;
                file.seekp(pos);
                file.write(reinterpret_cast<char *>(&bucket), sizeof(bucket));

                std::cout << "Record with ID " << record.id << " inserted into new bucket\n";
            }
        } else {
            bucket.records[bucket.count++] = record;
            file.seekp(pos);
            file.write(reinterpret_cast<char *>(&bucket), sizeof(bucket));
            std::cout << "Record with ID " << record.id << " inserted into bucket\n";
        }
    }

    static Record search(const int &id, const Bucket &bucket, std::fstream &file) {
        for (int i = 0; i < bucket.count; ++i) {
            if (bucket.records[i].id == id) {
                std::cout << "Record with ID " << id << " found\n";
                return bucket.records[i];
            }
        }
        if (bucket.hasNext()) {
            const Bucket nextBucket = getBucket(bucket.next, file);
            return search(id, nextBucket, file);
        }
        std::cout << "ID " << id << " not found\n";
        return {};
    }

    static void remove(const int &id, Bucket &bucket, std::fstream &file, const long long &pos) {
        for (int i = 0; i < bucket.count; ++i) {
            if (bucket.records[i].id == id) {
                // Shift records to remove the found record
                for (int j = i; j < bucket.count - 1; ++j) {
                    bucket.records[j] = bucket.records[j + 1];
                }
                bucket.records[bucket.count - 1] = {};
                --bucket.count;
                file.seekp(pos);
                file.write(reinterpret_cast<char *>(&bucket), sizeof(bucket));
                std::cout << "Record with ID " << id << " removed\n";
                return;
            }
        }
        if (bucket.hasNext()) {
            Bucket nextBucket = getBucket(bucket.next, file);
            remove(id, nextBucket, file, bucket.next);
            return;
        }
        std::cout << "ID " << id << " not found\n";
    }

public:
    Manager() {
        // Try opening the file in editing mode
        std::fstream file(FILENAME, std::ios::binary | std::ios::in | std::ios::out);
        // Check if the file is empty or does not exist
        if (isFileEmptyOrNonExistent(file)) {
            createFile(file); // Create the file
            createBuckets(file); // Create initial buckets
        }
        file.close(); // Close the file
    }

    static void insert(const Record &record) {
        // Open the file in read/write mode
        std::fstream file(FILENAME, std::ios::binary | std::ios::in | std::ios::out);
        // Calculate position using hash function
        const long long pos = hash(record.id) * sizeof(Bucket);
        // Get the bucket at the calculated position
        Bucket bucket = getBucket(pos, file);
        // Insert the record into the bucket recursively
        insert(record, bucket, file, pos);
        // Close the file
        file.close();
    }

    static Record search(const int &id) {
        // Open file in read mode
        std::fstream file(FILENAME, std::ios::binary | std::ios::in);
        // Calculate position using hash function
        const long long pos = hash(id) * sizeof(Bucket);
        // Get the bucket at the calculated position
        Bucket bucket = getBucket(pos, file);
        // Search for the record in the bucket recursively
        return search(id, bucket, file);
    }

    static void remove(const int &id) {
        // Open file in read/write mode
        std::fstream file(FILENAME, std::ios::binary | std::ios::in | std::ios::out);
        // Calculate position using hash function
        const long long pos = hash(id) * sizeof(Bucket);
        // Get the bucket at the calculated position
        Bucket bucket = getBucket(pos, file);
        // Remove the record from the bucket recursively
        remove(id, bucket, file, pos);
        // Close the file
        file.close();
    }
};

int main() {
    Manager manager;
    std::vector<int> ids = {3, 6, 20, 19, 13, 45, 36, 27, 2, 50, 89, 23, 44, 71, 38, 49, 53, 25, 22, 31, 60, 85, 43};

    for (const int id: ids) Manager::insert({id});
    for (int id: ids) Manager::search(id);
    // for (int id: ids) Manager::remove(id);
    // for (int id: ids) Manager::search(id);

    return 0;
}
