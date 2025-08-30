import struct
import Request
import FilesReader
import socket

server_info = FilesReader.read_server_info()
backup_list = FilesReader.read_backup_info()


def recv_exact(sock, n):
    buffer = b''
    while len(buffer) < n:
        chuck = sock.recv(n - len(buffer))
        if not chuck:
            raise ConnectionError(f"Expected {n} bytes, got {len(buffer)} before connection closed")
        buffer += chuck
    return buffer


def recv_file_payload(sock, file_size):
    # received = 0
    # file_data = b''
    # while received < file_size:
    #     chunk = sock.recv(min(4096, file_size - received))
    #     if not chunk:
    #         raise ConnectionError("Connection closed before receiving full file")
    #     file_data += chunk
    #     received += len(chunk)
    file_payload = b''
    while True:
        chunk = sock.recv(4096)
        if not chunk:
            break
        file_payload += chunk

    return file_payload


def send_request(request_obj):
    request_obj_struct = request_obj.to_bytes()
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)  # define tcp socket
    sock.connect(server_info)
    sock.sendall(request_obj_struct)
    return sock


def request_list_files_from_server():
    list_request = Request.ListRequest(1, 202)
    sock = send_request(list_request)

    version_and_status_data = recv_exact(sock, 3)
    version, status = struct.unpack("<BH", version_and_status_data)
    # version, status = struct.unpack("<BH", version_and_status_data)
    print("Version: ", version, " Status: ", status)
    if status == 211:
        data_name_len = recv_exact(sock, 2)
        name_len = struct.unpack("<H", data_name_len)[0]
        filename = recv_exact(sock, name_len)
        print(filename)

        file_size_data = recv_exact(sock, 4)
        file_size = struct.unpack("<I", file_size_data)[0]
        print(file_size)

        file_payload = recv_file_payload(sock, file_size)
        files_list = (file_payload.decode('utf-8')).splitlines()

        print(files_list)


    elif status == 1002:
        print("No files for user")

    sock.close()


def request_save_files_from_server():
    for file_name in backup_list:
        save_file_request = Request.SaveFileRequest(100, 1, file_name)
        sock = send_request(save_file_request)

        # should wait for a comment from the server
        header = recv_exact(sock, 5)
        version, status, name_len = struct.unpack('<BHH', header)
        filename = recv_exact(sock, name_len).rstrip(b'\x00').decode('ascii')

        if status == 212:
            print(f"Response from server received, file {filename} backup/delete was successful")
        else:
            print(f"An error occurred: {status}")

        sock.close()


def main():
    request_list_files_from_server()


if __name__ == "__main__":
    main()