import os
import struct


class ListRequest:
    def __init__(self, uid=None, version=None, op=None, other=None):
        if other is not None and isinstance(other, ListRequest):
            self.uid = other.uid
            self.version = other.version
            self.op = other.op
        else:
            self.uid = uid
            self.version = version
            self.op = op

    def to_bytes(self):
        # I is a 4-byte unsigned int, B is 1-byte unsigned int
        return struct.pack('<IBB', self.uid, self.version, self.op)


class FileOpsRequest(ListRequest):
    def __init__(self, uid=None, version=None, op=None, filename="", other=None):
        if other is not None and isinstance(other, ListRequest):
            super().__init__(other=other)
        else:
            super().__init__(uid, version, op)
        self.filename = filename.encode('utf-8')
        self.name_len = len(filename)

    def to_bytes(self):
        # I is a 4-byte unsigned int, B is 1-byte unsigned int, H is 2-byte unsigned short int
        return super().to_bytes() + struct.pack('<H', self.name_len) + self.filename


class SaveFileRequest(FileOpsRequest):
    def __init__(self, uid=None, version=None, op=None, filename="", request=None):
        if request is not None and isinstance(request, ListRequest):
            super().__init__(other=request, filename=filename)
        else:
            super().__init__(uid, version, op, filename)

        self.size = os.path.getsize(filename)
        with open(filename, 'rb') as f:
            self.payload = f.read()

    def to_bytes(self):
        return super().to_bytes() + struct.pack('<I', self.size) + self.payload
