import struct
import os
import math
import random
import csv
import time


# we'll be using 2 files, data and aux, both .bin
class Record:
    FORMAT = "i30sif10s?i?"
    SIZE = struct.calcsize(FORMAT)

    def __init__(
        self,
        id,
        name,
        quantity,
        unit_price,
        date,
        is_deleted=False,
        next=-1,
        is_next_in_aux=False,
    ):
        self.id = id  # i
        self.name = name.ljust(30)[:30]  # 30s
        self.quantity = quantity  # i
        self.unit_price = unit_price  # f
        self.date = date.ljust(10)[:10]  # YYYY-MM-DD, so 8s
        self.is_deleted = is_deleted  # 1 bit, ?
        self.next = next  # i, for linked sequential file
        self.is_next_in_aux = is_next_in_aux  # ?

    def show_data(self):
        print(
            f"ID: {self.id}, Name: {self.name}, Quantity: {self.quantity}, Unit Price: {self.unit_price}, Date: {self.date}"
        )

    def pack(self) -> bytes:
        return struct.pack(
            Record.FORMAT,
            self.id,
            self.name.encode(),
            self.quantity,
            self.unit_price,
            self.date.encode(),
            self.is_deleted,
            self.next,
            self.is_next_in_aux,
        )

    @staticmethod
    def unpack(record_buffer: bytes) -> "Record":
        id, name, quantity, unit_price, date, is_deleted, next, is_next_in_aux = (
            struct.unpack(Record.FORMAT, record_buffer)
        )
        return Record(
            id,
            name.decode().strip(),
            quantity,
            unit_price,
            date.decode().strip(),
            is_deleted,
            next,
            is_next_in_aux,
        )


class Header:
    FORMAT = "i?"  # first_index and is_first_in_aux
    SIZE = struct.calcsize(FORMAT)

    def __init__(self, first_index=0, is_first_in_aux=False):
        self.first_index = first_index
        self.is_first_in_aux = is_first_in_aux

    def pack(self) -> bytes:
        return struct.pack(self.FORMAT, self.first_index, self.is_first_in_aux)

    @staticmethod
    def unpack(header_buffer: bytes) -> "Header":
        first_index, is_first_in_aux = struct.unpack(Header.FORMAT, header_buffer)
        return Header(first_index, is_first_in_aux)


class SequentialFile_LinkedRecords:
    def __init__(self, filename, aux_filename):
        self.filename = filename
        self.aux_filename = aux_filename

        if not os.path.exists(self.filename):
            file = open(self.filename, "wb")
            new_header = Header()
            file.write(new_header.pack())
            file.close()

        if not os.path.exists(self.aux_filename):
            file = open(self.aux_filename, "wb")
            file.close()

    def aux_count(self) -> int:
        with open(self.aux_filename) as file:
            file.seek(0, os.SEEK_END)
            return file.tell() // Record.SIZE

    def count(self) -> int:
        with open(self.filename) as file:
            file.seek(0, os.SEEK_END)
            return (file.tell() - Header.SIZE) // Record.SIZE

    def reset(self):
        with open(self.filename, "rb+") as main_file:
            main_file.seek(0)
            main_file.truncate()
            header = Header()
            main_file.write(header.pack())
        open(self.aux_filename, "wb").close()

    def load_csv(self, csv_filename, randomize=False):
        records = []
        try:
            with open(csv_filename, "r", encoding="utf-8") as csv_file:
                r = csv.DictReader(csv_file)

                for row in r:
                    record = Record(
                        id=int(row["ID de la venta"]),
                        name=row["Nombre producto"],
                        quantity=int(row["Cantidad vendida"]),
                        unit_price=float(row["Precio unitario"]),
                        date=row["Fecha de venta"],
                    )
                    records.append(record)

                # for testing
                if randomize:
                    random.shuffle(records)

                for record in records:
                    self.insert(record)

        except FileNotFoundError:
            print(f"Load CSV error: {csv_filename} not found")

    def rebuild(self):
        print("Rebuild log: Attempting rebuild")
        open("temp.bin", "wb").close()

        with (
            open(self.filename, "rb") as main_file,
            open(self.aux_filename, "rb") as aux_file,
            open("temp.bin", "rb+") as temp_file,
        ):
            # write temp header
            temp_file.seek(0)
            temp_header = Header()
            temp_file.write(temp_header.pack())

            # read original header
            main_file.seek(0)
            main_header = Header.unpack(main_file.read(Header.SIZE))

            # where to start
            curr = main_header.first_index
            read_from_aux = main_header.is_first_in_aux

            counter = 0

            while curr != -1:
                if read_from_aux:
                    aux_file.seek(curr * Record.SIZE)
                    buffer = aux_file.read(Record.SIZE)
                else:
                    main_file.seek(Header.SIZE + curr * Record.SIZE)
                    buffer = main_file.read(Record.SIZE)

                if not buffer:
                    break

                buffer_data = Record.unpack(buffer)

                # retrieve info about next
                curr = buffer_data.next
                read_from_aux = buffer_data.is_next_in_aux

                # after retrieving info about next, skip if deleted
                if buffer_data.is_deleted:
                    continue

                # prepare for copying to temp
                if buffer_data.next != -1:
                    buffer_data.next = counter + 1
                    counter += 1

                buffer_data.is_next_in_aux = False
                temp_file.seek(0, os.SEEK_END)
                temp_file.write(buffer_data.pack())

        os.remove(self.filename)
        os.rename("temp.bin", self.filename)
        open(self.aux_filename, "wb").close()

    def insert(self, record: Record) -> bool:
        if self.count() == 0:
            with open(self.filename, "rb+") as main_file:
                main_file.seek(0, os.SEEK_END)
                main_file.write(record.pack())
                print(f"Insertion log: Item with id {record.id} inserted successfully")
                return True
        pred_index, pred_in_aux = self.find_predecessor(record.id)

        prefetch, _, _ = self.search(record.id)
        if prefetch is not None:
            print(f"Insertion error: Item with id {record.id} already exists")
            return False

        with (
            open(self.filename, "rb+") as main_file,
            open(self.aux_filename, "rb+") as aux_file,
        ):
            if pred_index == -1:
                main_file.seek(0)
                header = Header.unpack(main_file.read(Header.SIZE))
                record.next = header.first_index
                record.is_next_in_aux = header.is_first_in_aux
                # now make header point to this new record
                aux_file.seek(0, os.SEEK_END)
                header.first_index = aux_file.tell() // Record.SIZE
                header.is_first_in_aux = True
                main_file.seek(0)
                main_file.write(header.pack())
                aux_file.write(record.pack())
            else:
                # there is a predecessor, bridge
                if pred_in_aux:
                    aux_file.seek(pred_index * Record.SIZE)
                    predecessor = Record.unpack(aux_file.read(Record.SIZE))
                else:
                    main_file.seek(Header.SIZE + pred_index * Record.SIZE)
                    predecessor = Record.unpack(main_file.read(Record.SIZE))

                # bridge right side
                record.next = predecessor.next
                record.is_next_in_aux = predecessor.is_next_in_aux

                # bridge left side
                aux_file.seek(0, os.SEEK_END)
                predecessor.next = aux_file.tell() // Record.SIZE
                predecessor.is_next_in_aux = True
                aux_file.write(record.pack())
                # write updated predecessor
                if pred_in_aux:
                    aux_file.seek(pred_index * Record.SIZE)
                    aux_file.write(predecessor.pack())
                else:
                    main_file.seek(Header.SIZE + pred_index * Record.SIZE)
                    main_file.write(predecessor.pack())

        print(f"Insertion log: Item with id {record.id} inserted successfully")
        if self.aux_count() > math.log2(self.count()):
            print(
                f"Insertion warning: aux_count={self.aux_count()} exceeds log2(self_count)={math.log2(self.count())}"
            )
            self.rebuild()
            print("Insertion log: Rebuild successful")

        return True

    def show_data(self) -> None:
        with (
            open(self.filename, "rb") as main_file,
            open(self.aux_filename, "rb") as aux_file,
        ):
            main_file.seek(0)
            header = Header.unpack(main_file.read(Header.SIZE))

            curr = header.first_index
            read_from_aux = header.is_first_in_aux

            while curr != -1:
                if read_from_aux:
                    aux_file.seek(curr * Record.SIZE)
                    buffer = aux_file.read(Record.SIZE)
                else:
                    main_file.seek(Header.SIZE + curr * Record.SIZE)
                    buffer = main_file.read(Record.SIZE)

                if not buffer:
                    break

                buffer_data = Record.unpack(buffer)

                if not buffer_data.is_deleted:
                    buffer_data.show_data()

                curr = buffer_data.next
                read_from_aux = buffer_data.is_next_in_aux

    def find_predecessor(self, id: int) -> tuple[int, bool]:
        res_index = -1
        res_in_aux = False

        with (
            open(self.filename, "rb") as main_file,
            open(self.aux_filename, "rb") as aux_file,
        ):
            main_file.seek(0)
            header = Header.unpack(main_file.read(Header.SIZE))

            curr = header.first_index
            read_from_aux = header.is_first_in_aux

            while curr != -1:
                if read_from_aux:
                    aux_file.seek(curr * Record.SIZE)
                    buffer = aux_file.read(Record.SIZE)
                else:
                    main_file.seek(Header.SIZE + curr * Record.SIZE)
                    buffer = main_file.read(Record.SIZE)

                if not buffer:
                    break

                buffer_data = Record.unpack(buffer)

                # if predecessor, update return value
                if (
                    buffer_data.id < id and not buffer_data.is_deleted
                ):  # ignores deleted records when looking for predecessor
                    res_index = curr
                    res_in_aux = read_from_aux

                curr = buffer_data.next
                read_from_aux = buffer_data.is_next_in_aux

        return res_index, res_in_aux

    def remove(self, id: int) -> bool:
        obj, idx, is_in_aux = self.search(id)
        if obj is None or idx == -1:
            print(f"Deletion error: Record with id {id} was not found")
            return
        obj.is_deleted = True

        if is_in_aux:
            with open(self.aux_filename, "rb+") as aux_file:
                aux_file.seek(idx * Record.SIZE)
                aux_file.write(obj.pack())
        else:
            with open(self.filename, "rb+") as main_file:
                main_file.seek(Header.SIZE + idx * Record.SIZE)
                main_file.write(obj.pack())
        print(
            f"Deletion log: Record with id {id} was deleted successfully from {'aux file' if is_in_aux else 'main file'}"
        )

    def search(self, id: int) -> tuple[Record | None, int, bool]:
        # first binary search in data file. if not found, linear search in aux file
        # if found, return record. else, return None
        with open(self.filename, "rb") as file:
            item_count = self.count()
            left = 0
            right = item_count - 1
            while True:
                mid = (left + right) // 2
                file.seek(Header.SIZE + Record.SIZE * mid)
                mid_record = Record.unpack(file.read(Record.SIZE))

                # binary search
                if mid_record.id == id:
                    if mid_record.is_deleted:
                        return None, -1, False
                    return mid_record, mid, False
                elif mid_record.id < id:
                    left = mid + 1
                else:
                    right = mid - 1

                if left > right:
                    break

        # if we are here, there are no more records to check in data file
        # linear search in aux file
        with open(self.aux_filename, "rb") as file:
            file.seek(0)
            idx = 0
            while True:
                record_buffer = file.read(Record.SIZE)
                if not record_buffer:
                    break
                record = Record.unpack(record_buffer)

                if record.id == id:
                    if record.is_deleted:
                        return None, -1, True
                    return record, idx, True
                idx += 1

        return None, -1, False  # objcet, index, is_in_aux

    def range_search(self, low: int, high: int) -> list[Record]:
        # search for lowest in both files
        # then follow linked list until high is reached
        res = []
        lowest_id = -1
        lowest_pos = -1
        low_in_aux = False

        with open(self.filename, "rb") as main_file:
            item_count = self.count()
            left = 0
            right = item_count - 1
            while True:
                mid = (left + right) // 2
                main_file.seek(Header.SIZE + mid * Record.SIZE)

                mid_record = Record.unpack(main_file.read(Record.SIZE))

                if mid_record.id < low:
                    left = mid + 1
                elif high < mid_record.id:
                    right = mid - 1
                # else value is in interval
                else:
                    if lowest_pos == -1 or mid < lowest_pos:
                        lowest_pos = mid
                        lowest_id = mid_record.id
                        right = mid - 1  # try to search in left
                if left > right:
                    break

        with open(self.aux_filename, "rb") as aux_file:
            aux_count = self.aux_count()
            aux_file.seek(0)
            for i in range(aux_count):
                buffer_data = Record.unpack(aux_file.read(Record.SIZE))
                if (
                    buffer_data.id <= high and buffer_data.id >= low
                ):  # if is in interval
                    if lowest_pos == -1 or buffer_data.id < lowest_id:
                        lowest_id = buffer_data.id
                        lowest_pos = i
                        low_in_aux = True

        # now we have index of lowest
        with (
            open(self.filename, "rb") as main_file,
            open(self.aux_filename, "rb") as aux_file,
        ):
            curr = lowest_pos
            read_from_aux = low_in_aux

            while curr != -1:
                if read_from_aux:
                    aux_file.seek(curr * Record.SIZE)
                    buffer = aux_file.read(Record.SIZE)
                else:
                    main_file.seek(Header.SIZE + curr * Record.SIZE)
                    buffer = main_file.read(Record.SIZE)

                if not buffer:
                    break

                buffer = Record.unpack(buffer)

                if buffer.id > high:
                    break

                curr = buffer.next
                read_from_aux = buffer.is_next_in_aux

                if not buffer.is_deleted:
                    res.append(buffer)
        return res


def custom_test():
    s = SequentialFile_LinkedRecords("data.bin", "aux.bin")

    # insertion and rebuilding tests:

    a = Record(1, "Keyboard", 1, 320, "2025-03-27")  # inserted as first record
    b = Record(
        2, "Mouse", 1, 220, "2025-03-27"
    )  # inserted in aux, 1 > log(1), rebuilds
    # now data size is 2, aux size is 0

    c = Record(3, "Mousepad", 1, 60, "2025-03-27")  # inserted in aux
    d = Record(4, "Socks", 1, 12, "2025-03-27")  # inserted in aux, 2 > log(2), rebuilds
    # now data size is 4, aux size is 0

    e = Record(5, "Monitor", 1, 1200, "2025-03-27")  # inserted in aux
    f = Record(6, "Mic", 1, 500, "2025-03-27")  # inserted in aux
    g = Record(
        7, "Headphones", 1, 60, "2025-03-27"
    )  # inserted in aux, 3 > log(4), rebuilds
    # now data size is 7, aux size is 0

    arr = [a, b, c, d, e, f, g]
    random.shuffle(arr)

    for element in arr:
        s.insert(element)

    # deletion tests

    s.remove(6)
    s.remove(3)
    # now real data size is 5

    # insertion tests again, to rebuild without deleted records
    h = Record(8, "Speakers", 1, 220, "2025-03-27")  # inserted in aux
    i = Record(9, "GPU", 1, 1800, "2025-03-27")  # inserted in aux
    j = Record(
        10, "CPU", 1, 1100, "2025-03-27"
    )  # inserted in aux, 3 > log2(7), rebuilds
    # now data size should be 8

    arr = [h, i, j]
    random.shuffle(arr)

    for element in arr:
        s.insert(element)

    print(f"Main file record count should be 8, is {s.count()}")

    s.show_data()

    # range query

    print("Now we do a range query. Looking for values with ids in [2:8]")
    res = s.range_search(2, 8)

    for r in res:
        r.show_data()


def insertion_test():
    output_file = "results.txt"
    # insertion test
    insertion_times = []
    for _ in range(10):
        s = SequentialFile_LinkedRecords("data.bin", "aux.bin")
        start_time = time.time()
        s.load_csv("sales_dataset.csv", True)
        end_time = time.time()
        s.reset()
        elapsed_ns = (end_time - start_time) * 1e9
        insertion_times.append(elapsed_ns)
        print(f"Loading CSV took {elapsed_ns}ns")
    insertion_average = sum(insertion_times) / len(insertion_times)

    with open(output_file, "a") as file:
        file.write("Insertion Tests:\n")
        for i in range(10):
            file.write(f"{i}: Loading CSV took {insertion_times[i]}ns\n")
        file.write(f"Average insertion time: {insertion_average}ns\n")
        file.write("\n")


def search_test():
    output_file = "results.txt"

    # search test
    search_times = []
    for _ in range(10):
        s = SequentialFile_LinkedRecords("data.bin", "aux.bin")
        s.load_csv("sales_dataset.csv", True)
        start_time = time.time()
        s.search(250)
        end_time = time.time()
        s.reset()
        elapsed_ns = (end_time - start_time) * 1e9
        search_times.append(elapsed_ns)
        print(f"Searching took {elapsed_ns:.2f}ns")
    search_average = sum(search_times) / len(search_times)

    with open(output_file, "a") as file:
        file.write("Search Tests:\n")
        for i in range(10):
            file.write(f"{i}: Searching took {search_times[i]}ns\n")
        file.write(f"Average search time: {search_average}ns\n")
        file.write("\n")


def search_by_range_test():
    output_file = "results.txt"
    # search by range test
    range_search_times = []
    for _ in range(10):
        s = SequentialFile_LinkedRecords("data.bin", "aux.bin")
        s.load_csv("sales_dataset.csv", True)
        start_time = time.time()
        s.range_search(250, 750)
        end_time = time.time()
        s.reset()
        elapsed_ns = (end_time - start_time) * 1e9
        range_search_times.append(elapsed_ns)
        print(f"Range searching took {elapsed_ns}ns")
    range_search_average = sum(range_search_times) / len(range_search_times)

    with open(output_file, "a") as file:
        file.write("Range Search Tests:\n")
        for i in range(10):
            file.write(f"{i}: Range searching took {range_search_times[i]}ms\n")
        file.write(f"Average range search time: {range_search_average}ns\n")
        file.write("\n")


def deletion_test():
    output_file = "results.txt"
    # deletion test

    deletion_times = []
    for _ in range(10):
        s = SequentialFile_LinkedRecords("data.bin", "aux.bin")
        s.load_csv("sales_dataset.csv", True)
        start_time = time.time()
        s.remove(250)
        end_time = time.time()
        s.reset()
        elapsed_ns = (end_time - start_time) * 1e9
        deletion_times.append(elapsed_ns)
        print(f"Deletion took {elapsed_ns}ns")
    deletion_average = sum(deletion_times) / len(deletion_times)

    with open(output_file, "a") as file:
        file.write("Deletion Tests:\n")
        for i in range(10):
            file.write(f"{i}: Deletion took {deletion_times[i]}ns\n")
        file.write(f"Average deletion time: {deletion_average}ns\n")
        file.write("\n")


def main():
    insertion_test()
    search_test()
    search_by_range_test()
    deletion_test()


if __name__ == "__main__":
    main()
