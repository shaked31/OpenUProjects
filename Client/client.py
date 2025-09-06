import struct
import os
import socket
from copyreg import constructor

import Request
import FilesReader




def create_id():
    id = os.urandom(4)
    i = struct.unpack('<I', id)[0]  # little endian unsigned int
    return i

def recv_exact(sock, n):
    buffer = b''
    while len(buffer) < n:
        chuck = sock.recv(n - len(buffer))
        if not chuck:
            raise ConnectionError("Connection closed with extra bytes to receive")
        buffer += chuck
    return buffer


def recv_file_payload(sock, file_size):
    file_payload = b''
    total_bytes = 0
    while total_bytes < file_size:
        chunk_size = min(4096, file_size - total_bytes)
        chunk = sock.recv(chunk_size)
        if not chunk:
            raise ConnectionError("Connection closed with extra bytes to receive")
        total_bytes += chunk_size
        file_payload += chunk
    return file_payload



def send_request(request_obj):
    server_info = FilesReader.read_server_info()
    request_obj_struct = request_obj.to_bytes()
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)  # define tcp socket
    sock.connect(server_info)
    sock.sendall(request_obj_struct)
    return sock


def request_list_files(uid):
    list_request = Request.ListRequest(uid,1, 202)
    sock = send_request(list_request)

    version_and_status_data = recv_exact(sock, 3)
    version, status = struct.unpack("<BH", version_and_status_data)
    print("Version: ", version, " Status: ", status)
    if status == 211:
        data_name_len = recv_exact(sock, 2)
        name_len = struct.unpack("<H", data_name_len)[0]
        filename = recv_exact(sock, name_len)
        # print(filename)

        file_size_data = recv_exact(sock, 4)
        file_size = struct.unpack("<I", file_size_data)[0]
        # print(file_size)

        file_payload = recv_file_payload(sock, file_size)
        files_list = (file_payload.decode('utf-8')).splitlines()

        for file in files_list:
            print(f"The file '{file}' is saved on the server")

    elif status == 1002:
        print("No files for user")

    sock.close()


def request_save_files(uid, filename):
    req = Request.ListRequest(uid, 1, 100)
    save_file_request = Request.SaveFileRequest(request=req, filename=filename)
    sock = send_request(save_file_request)

    # wait for a comment from the server
    header = recv_exact(sock, 5)
    version, status, name_len = struct.unpack('<BHH', header)
    filename = recv_exact(sock, name_len).rstrip(b'\x00').decode('utf-8')

    if status == 212:
        print(f"Response from server received, file {filename} backup was successful")
    else:
        print(f"An error occurred: {status}")

    # sock.close()

def request_retrieve_files(uid, filename):
    req = Request.ListRequest(uid, 1, 200)
    fileOpsReq = Request.FileOpsRequest(filename=filename, other=req)
    sock = send_request(fileOpsReq)

    header = recv_exact(sock, 3)
    version, status = struct.unpack('<BH', header)
    if status == 1002:
        print("No files for this user")
        return
    else:
        name_len_data = recv_exact(sock, 2)
        name_len = struct.unpack('<H', name_len_data)[0]
        filename = recv_exact(sock, name_len).rstrip(b'\x00').decode('utf-8')
        if status == 1001:
            print(f"file {filename} does not exist on the server")
            return
        file_size_data = recv_exact(sock, 4)
        file_size = struct.unpack("<I", file_size_data)[0]

        file_payload = recv_file_payload(sock, file_size)

        folder_path = f"Retrieved Files\\{uid}"
        os.makedirs(folder_path, exist_ok=True)

        with open(f"Retrieved Files\\{uid}\\{filename}", 'wb') as f:
            f.write(file_payload)

        print(f"File {filename} was retrieved from the server")

def request_delete_files(uid, filename):
    req = Request.ListRequest(uid, 1, 201)
    fileOpsReq = Request.FileOpsRequest(filename=filename, other=req)
    sock = send_request(fileOpsReq)

    header = recv_exact(sock, 5)
    version, status, name_len = struct.unpack('<BHH', header)
    filename = recv_exact(sock, name_len).rstrip(b'\x00').decode('utf-8')

    if status == 212:
        print(f"Response from server received, file {filename} delete was successful")
    else:
        print(f"An error occurred: {status}")


def main():
    uid = create_id()

    backup_list = FilesReader.read_backup_info()

    request_list_files(uid)
    for filename in backup_list:
        request_save_files(uid, filename)

    request_list_files(uid)

    request_retrieve_files(uid, backup_list[0])
    request_delete_files(uid, backup_list[0])
    request_retrieve_files(uid, backup_list[0])


if __name__ == "__main__":
    main()