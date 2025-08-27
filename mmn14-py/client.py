import struct

import Request
import FilesReader
import socket

server_info = FilesReader.read_server_info()
backup_list = FilesReader.read_backup_info()

def define_connect_send_tcp(request_obj):
    request_obj_struct = request_obj.to_bytes()
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)  # define tcp socket
    sock.connect(server_info)
    sock.sendall(request_obj_struct)
    return sock

def request_files_from_server():
    list_request = Request.ListRequest(202, 1)
    define_connect_send_tcp(list_request)

def recv_exact(sock, n):
    buffer = b''
    while len(buffer) < n:
        chuck = sock.recv(n - len(buffer))
        if not chuck:
            raise ConnectionError()
        buffer += chuck
    return buffer


def request_save_files_from_server():
    for file_name in backup_list:
        save_file_request = Request.SaveFileRequest(100, 1, file_name)
        sock = define_connect_send_tcp(save_file_request)

        # should wait for a comment from the server
        header = recv_exact(sock, 5)
        version, status, name_len = struct.unpack('<BHH', header)
        filename = recv_exact(sock, name_len).rstrip(b'\x00').decode('ascii')

        if status == 212:
            print(f"Response from server received, file {filename} backup/delete was successful")
        else:
            print(f"An error occurred: {status}")



def main():
    request_files_from_server()

if __name__ == "__main__":
    main()