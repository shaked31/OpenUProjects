import os
import struct
import sys

class ListRequest:
    def __init__(self, op, version):
        self.uid = self.create_id()
        self.op = op
        self.version = version


    def create_id(self):
        id = os.urandom(4)
        i = struct.unpack('<I', id)[0]  # little endian unsigned int
        return i


    def to_bytes(self):
        # I is a 4-byte unsigned int, B is 1-byte unsigned int
        return struct.pack('<IBB', self.uid, self.op, self.version)


class FileOpsRequest(ListRequest):
    def __init__(self, op, version, filename):
        super().__init__(op, version)
        self.filename = filename.encode('ascii')
        self.name_len = len(filename)

    def to_bytes(self):
        # I is a 4-byte unsigned int, B is 1-byte unsigned int, H is 2-byte unsigned short int
        return super().to_bytes() + struct.pack('<H',self.name_len) + self.filename

class SaveFileRequest(FileOpsRequest):
    def __init__(self, op, version, filename):
        super().__init__(op, version, filename)
        with open(filename, 'rb') as f:
            self.payload = f.read()
        self.size = os.path.getsize(filename)
    def to_bytes(self):
        return super().to_bytes() + struct.pack('<I', self.size) + self.payload