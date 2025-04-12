#include <iostream>
#include <fstream>
#include <string>

struct Record {
    int id;
};

constexpr int BUCKET_SIZE = 4;
constexpr std::string FILENAME = "data.txt";
constexpr int RECORD_SIZE = sizeof(Record);
constexpr int BUCKETS = 5;

struct Bucket {
    Record records[BUCKET_SIZE];
    int count = 0;
    int next = -1;

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

    static void createFile(std::fstream &file) {
        file.open(FILENAME, std::ios::binary | std::ios::app);
    }

    static void createBuckets(std::fstream &file) {
        for (int i = 0; i < BUCKETS; ++i) {
            Bucket bucket;
            file.write(reinterpret_cast<char *>(&bucket), sizeof(bucket));
        }
    }

    static int hash(const int& k) {
        return k % BUCKETS;
    }

    static Bucket getBucket(const int pos, std::fstream &file) {
        Bucket bucket;
        file.seekg(pos); // Move the file pointer to the position of the bucket
        // Read the bucket from the file
        file.read(reinterpret_cast<char *>(&bucket), sizeof(Bucket));
        return bucket;
    }

    void insert(const Record& record, Bucket &bucket, std::fstream &file, const int& pos) {
        // Check if the bucket is full
        if (bucket.isFull()) {
            // Check if there is a next bucket
            if (bucket.hasNext()) {
                Bucket nextBucket = getBucket(bucket.next, file); // Get the next bucket
                insert(record, nextBucket, file, bucket.next); // Insert into the next bucket
                return;
            }

            // If there is no next bucket, create a new one
            Bucket newBucket;
            newBucket.records[0] = record;
            newBucket.count = 1;

            // Go to the end of the file
            file.seekp(0, std::ios::end);

            // Update the current bucket's next pointer
            bucket.next = file.tellp();

            // Write the new bucket to the file
            file.write(reinterpret_cast<char *>(&newBucket), sizeof(Bucket));

            // Update the current bucket in the file
            file.seekp(pos * sizeof(Bucket), std::ios::beg);
            file.write(reinterpret_cast<char *>(&bucket), sizeof(Bucket));
            return;
        }
        // If the bucket is not full, insert the record
        bucket.records[bucket.count] = record;
        bucket.count++;

        // Write the updated bucket to the file
        file.seekg(pos * RECORD_SIZE, std::ios::beg);
        file.write(reinterpret_cast<char *>(&bucket), sizeof(Bucket));
    }

    Record search(const int& id, Bucket& bucket, std::fstream &file) {
    }

    void remove(const int& id, Bucket& bucket, std::fstream &file, const int& pos) {
    }

    void reallocate(Bucket& bucket, std::fstream &file, const int& pos) {
    }

public:
    Manager() {
        // Try to open the file in editing mode
        std::fstream file(FILENAME, std::ios::binary | std::ios::in | std::ios::out);

        // Check if the file is empty or does not exist
        if (isFileEmptyOrNonExistent(file)) {
            createFile(file);   // Create the file
            createBuckets(file);    // Create initial buckets
        }

        file.close();   // Close the file
    }

    void insert(const Record& record) {
        // Open the file in binary mode for reading and writing
        std::fstream file(FILENAME, std::ios::binary | std::ios::in | std::ios::out);
        const int h = hash(record.id);  // Hash ID to get the bucket index
        Bucket bucket = getBucket(h * sizeof(Bucket), file);    // Get the bucket
        insert(record, bucket, file, h);    // Call aux function for insertion
    }

    Record search(const int& id) {
    }

    void remove(const int& id) {
    }
};

int main() {
    std::cout << "Hello, World!" << std::endl;
    return 0;
}