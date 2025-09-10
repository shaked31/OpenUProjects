import struct
import os
import socket
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



def send_request(request_obj, server_info):
    request_obj_struct = request_obj.to_bytes()
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)  # define tcp socket
    sock.connect(server_info)
    sock.sendall(request_obj_struct)
    return sock


def request_list_files(uid, server_info):
    list_request = Request.ListRequest(uid,1, 202)
    sock = send_request(list_request, server_info)

    version_and_status_data = recv_exact(sock, 3)
    version, status = struct.unpack("<BH", version_and_status_data)

    if status == 211:
        data_name_len = recv_exact(sock, 2)
        name_len = struct.unpack("<H", data_name_len)[0]
        filename = recv_exact(sock, name_len)

        file_size_data = recv_exact(sock, 4)
        file_size = struct.unpack("<I", file_size_data)[0]

        file_payload = recv_file_payload(sock, file_size)
        files_list = (file_payload.decode('utf-8')).splitlines()

        for file in files_list:
            print(f"[INFO] Response no. {status}: The file '{file}' is saved on the server")

    elif status == 1002:
        print(f"[ERROR] Response no. {status}: No files for user")

    sock.close()


def request_save_files(uid, filename, server_info):
    req = Request.ListRequest(uid, 1, 100)
    save_file_request = Request.SaveFileRequest(request=req, filename=filename)
    sock = send_request(save_file_request, server_info)

    # wait for a comment from the server
    header = recv_exact(sock, 3)
    version, status = struct.unpack('<BH', header)
    if status == 1002:
        print(f"[ERROR] Response no. {status}: No files for this user")
        return
    elif status == 1003:
        print(f"[ERROR] Response no. {status}: A general error on the server occurred")
        return
    else:
        name_len_data = recv_exact(sock, 2)
        name_len = struct.unpack('<H', name_len_data)[0]
        filename = recv_exact(sock, name_len).rstrip(b'\x00').decode('utf-8')
        print(f"[INFO] Response no. {status}: Saving the file '{filename}' was successful")
    sock.close()

def request_retrieve_files(uid, filename, server_info):
    req = Request.ListRequest(uid, 1, 200)
    file_ops_req = Request.FileOpsRequest(filename=filename, other=req)
    sock = send_request(file_ops_req, server_info)

    header = recv_exact(sock, 3)
    version, status = struct.unpack('<BH', header)
    if status == 1002:
        print(f"[ERROR] Response no. {status}: No files for this user on the server")
        return
    else:
        name_len_data = recv_exact(sock, 2)
        name_len = struct.unpack('<H', name_len_data)[0]
        filename = recv_exact(sock, name_len).rstrip(b'\x00').decode('utf-8')
        if status == 1001:
            print(f"[ERROR] Response no. {status}: File '{filename}' does not exist on the server")
            return
        file_size_data = recv_exact(sock, 4)
        file_size = struct.unpack("<I", file_size_data)[0]

        file_payload = recv_file_payload(sock, file_size)

        folder_path = f"Retrieved Files\\{uid}"
        os.makedirs(folder_path, exist_ok=True)

        with open(f"Retrieved Files\\{uid}\\{filename}", 'wb') as f:
            f.write(file_payload)
        with open(f"Retrieved Files\\{uid}\\tmp.py", 'wb') as f:
            pass

        print(f"[INFO] Response no. {status}: File '{filename}' was retrieved from the server successfully")
    sock.close()


def request_delete_files(uid, filename, server_info):
    req = Request.ListRequest(uid, 1, 201)
    file_ops_req = Request.FileOpsRequest(filename=filename, other=req)
    sock = send_request(file_ops_req, server_info)

    header = recv_exact(sock, 5)
    version, status, name_len = struct.unpack('<BHH', header)
    filename = recv_exact(sock, name_len).rstrip(b'\x00').decode('utf-8')

    if status == 212:
        print(f"[INFO] Response no. {status}: File {filename} delete was successful")
    else:
        print(f"[ERROR] Response no. {status}: An error occurred: {status}")

    sock.close()


def main():
    uid = create_id()

    server_info = FilesReader.read_server_info()
    backup_list = FilesReader.read_backup_info()

    request_list_files(uid, server_info)

    for filename in backup_list:
        request_save_files(uid, filename, server_info)

    request_list_files(uid, server_info)

    request_retrieve_files(uid, backup_list[0], server_info)
    request_delete_files(uid, backup_list[0], server_info)
    request_retrieve_files(uid, backup_list[0], server_info)


if __name__ == "__main__":
    main()