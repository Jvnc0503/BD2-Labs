import struct
import os

class Alumno:
    def __init__(self, codigo, nombre, apellidos, carrera, ciclo, mensualidad):
        self.codigo = codigo
        self.nombre = nombre
        self.apellidos = apellidos
        self.carrera = carrera
        self.ciclo = ciclo
        self.mensualidad = mensualidad

class FixedRecord:
    def __init__(self, filename, delmode = False):
        self.filename = filename
        self.delmode = delmode

        #Check deletion mode
        if (self.delmode):
            self.format = "5s11s20s15sifi"
            #Check if the file doesn't exist or is empty
            if not os.path.exists(self.filename) or os.path.getsize(self.filename) == 0:
                #Add first deleted record and number of records headers
                with open(self.filename, "wb") as file:
                    firstDel = struct.pack("i", -1)
                    file.write(firstDel)
                    records = struct.pack("i", 0)
                    file.write(records)

        else:
            self.format = "5s11s20s15sif"
            #Check if the file doesn't exist or is empty
            if not os.path.exists(self.filename) or os.path.getsize(self.filename) == 0:
                with open(self.filename, "wb") as file:
                    records = struct.pack("i", 0)
                    file.write(records)

        self.size = struct.calcsize(self.format)
    
    def load(self):
        with open (self.filename, "r+b") as file:
            #Check deletion mode
            if (self.delmode):
                #Get number of records
                file.seek(4)
                data = file.read(4)
                records = struct.unpack("i", data)[0]
                file.seek(8)
                
                #Iterate for valid records
                i = 0
                while i < records:
                    data = file.read(self.size)
                    codigo, nombre, apellidos, carrera, ciclo, mensualidad, nextDel = struct.unpack(self.format, data)
                    if (nextDel == 0):
                        print(codigo.decode().strip(), nombre.decode().strip(), apellidos.decode().strip(), carrera.decode().strip(), ciclo, mensualidad)
                        i += 1
            else:
                #Get number of records
                file.seek(0)
                data = file.read(4)
                records = struct.unpack("i", data)[0]

                #Itarate for records in range
                for i in range(records):
                    data = file.read(self.size)
                    codigo, nombre, apellidos, carrera, ciclo, mensualidad = struct.unpack(self.format, data)
                    print(codigo.decode().strip(), nombre.decode().strip(), apellidos.decode().strip(), carrera.decode().strip(), ciclo, mensualidad)

    def add(self, alumno):
        with open(self.filename, "r+b") as file:
            #Check deletion mode
            if (self.delmode):
                #Get first deleted record position
                file.seek(0)
                data = file.read(4)
                firstDel = struct.unpack("i", data)[0]

                #Get number of records
                file.seek(4)
                data = file.read(4)
                records = struct.unpack("i", data)[0]

                #Check if there are deleted records
                if (firstDel != -1):
                    #Get next deleted record
                    file.seek(8 + self.size * firstDel)
                    data = file.read(self.size)
                    nextDel = struct.unpack(self.format, data)[6]

                    #Overwrite with new record
                    file.seek(8 + self.size * firstDel)
                    record = struct.pack(self.format, alumno.codigo.encode(), alumno.nombre.encode(), alumno.apellidos.encode(), alumno.carrera.encode(), alumno.ciclo, alumno.mensualidad, nextDel)
                    file.write(record)

                    #Update first deleted record header
                    file.seek(0)
                    file.write(struct.pack("i", nextDel))

                else:
                    #Add new record at the end of the file
                    file.seek(8 + self.size * records)
                    record = struct.pack(self.format, alumno.codigo.encode(), alumno.nombre.encode(), alumno.apellidos.encode(), alumno.carrera.encode(), alumno.ciclo, alumno.mensualidad, 0)
                    file.write(record)

                #Update number of records header
                records += 1
                file.seek(4)
                file.write(struct.pack("i", records))
                
            else:
                #Get number of records
                file.seek(0)
                data = file.read(4)
                records = struct.unpack("i", data)[0]

                #Add new record at the end of the file
                file.seek(4 + self.size * records)
                record = struct.pack(self.format, alumno.codigo.encode(), alumno.nombre.encode(), alumno.apellidos.encode(), alumno.carrera.encode(), alumno.ciclo, alumno.mensualidad)
                file.write(record)

                #Update number of records header
                records += 1
                file.seek(0)
                file.write(struct.pack("i", records))

            print("Registro agregado exitosamente")

    def readRecord(self, pos):
        with open(self.filename, "rb") as file:
            #Check deletion mode
            if (self.delmode):
                #Get record at position
                file.seek(8 + self.size * pos)
                data = file.read(self.size)

                #Check if record exists
                if data:
                    #Unpack record
                    codigo, nombre, apellidos, carrera, ciclo, mensualidad, nextDel = struct.unpack(self.format, data)
                    
                    #Check if record is valid
                    if (nextDel == 0):
                        print(codigo.decode().strip(), nombre.decode().strip(), apellidos.decode().strip(), carrera.decode().strip(), ciclo, mensualidad)
                    else:
                        print("Registro eliminado")
                else:
                    print("Registro fuera de limite")
            else:
                #Get number of records
                file.seek(0)
                data = file.read(4)
                records = struct.unpack("i", data)[0]

                #Check if records is in range
                if pos < records:
                    #Get record at position
                    file.seek(4 + self.size * pos)
                    data = file.read(self.size)
                    codigo, nombre, apellidos, carrera, ciclo, mensualidad = struct.unpack(self.format, data)

                    print(codigo.decode().strip(), nombre.decode().strip(), apellidos.decode().strip(), carrera.decode().strip(), ciclo, mensualidad)
                else:
                    print("Registro fuera de limite")

    def remove(self, pos):
        with open(self.filename, "r+b") as file:
            #Check deletion mode
            if (self.delmode):
                #Get record at position
                file.seek(8 + self.size * pos)
                data = file.read(self.size)

                #Check if record exists
                if data:
                    #Get next deleted record
                    nextDel = struct.unpack(self.format, data)[6]

                    #Get first deleted record
                    file.seek(0)
                    data = file.read(4)
                    firstDel = struct.unpack("i", data)[0]

                    #Get number of records
                    file.seek(4)
                    data = file.read(4)
                    records = struct.unpack("i", data)[0]

                    #Check if record is valid
                    if (nextDel == 0):
                        #Set next deleted record
                        file.seek(8 + self.size * (pos + 1) -4)
                        nextDel = struct.pack("i", firstDel)
                        file.write(nextDel)

                        #Update first deleted record
                        file.seek(0)
                        firstDel = struct.pack("i", pos)
                        file.write(firstDel)

                        #Update number of records
                        file.seek(4)
                        records -= 1
                        header = struct.pack("i", records)
                        file.write(header)

                        print("Registro eliminado exitosamente")
                    else:
                        print("Registro ya eliminado")
                else:
                    print("Registro fuera de limite")
            else:
                #Get number of records
                file.seek(0)
                data = file.read(4)
                records = struct.unpack("i", data)[0]

                #Check if record is in range
                if pos < records:
                    #Get last record
                    file.seek(4 + self.size * records)
                    lastRecord = file.read(self.size)

                    #Overwrite with last record
                    file.seek(4 + self.size * pos)
                    file.write(lastRecord)

                    #Update number of records
                    file.seek(0)
                    records -= 1
                    header = struct.pack("i", records)
                    file.write(header)

                    print("Registro eliminado exitosamente")
                else:
                    print("Registro fuera de limite")

db = FixedRecord("alumnosTrue.txt", True)
db.add(Alumno("00001", "Juan", "Perez", "Ingenieria", 1, 2000))
db.readRecord(0)
print()
db.add(Alumno("00002", "Maria", "Lopez", "Ingenieria", 2, 2500))
db.add(Alumno("00003", "Pedro", "Gomez", "Ingenieria", 3, 3000))
db.load()
print()
db.remove(0)
db.load()
print()
db.remove(0)
db.load()
print()
db.remove(10)
db.load()
print()