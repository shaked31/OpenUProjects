def read_server_info():
    with open('server.info', 'r') as f:
        line = f.read()
        ip, port = line.split(':')
        return ip, int(port)


def read_backup_info():
    file_names = []
    with open('backup.info', 'r') as f:
        for line in f:
            file_names.append(line.strip('\n'))
        return file_names
