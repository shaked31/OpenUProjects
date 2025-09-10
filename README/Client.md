Client

Short Explanation:
	This Python client communicates with the Backup Server using little endian represented values to perform file backup operations such as:
		- List files stored on the server
		- Save files (upload) to the server
		- Retrieve files (download) from the server
		- Delete files from the server

The client and server communicate over a custom binary protocol with a stateless design.

Requirements
	- Python 3.8+
	-Standard library modules only (socket, struct, os)

Configuration:
	The client reads two configuration files:
		1. server.info
		This file will contain the server’s IP and port in the format:
		127.0.0.1:5000

		2. backup.info
		This file will contain a list of local filenames (one per line) to back up, for example:
		notes.txt
		report.pdf
		image.png

Running the Client:
	python client.py

Running the program will:
	1. Generate a random user ID.
	2. Send a LIST request to the server.
	3. Upload all files listed in backup.info.
	4. List files again to confirm they were saved.
	5. Retrieve the first file in the list into the folder Retrieved Files/<uid>/.
	6. Delete the same file on the server.
	7. Attempt to retrieve it again to confirm deletion.

Output:

	Retrieved files are saved under:
		Retrieved Files/<uid>


Console output shows the status of each operation (success, error, file not found, etc.).

Protocol Notes:

All requests start with:
	- UID (4 bytes)
	- Version (1 byte)
	- OpCode (1 byte)

FileOpsRequest adds:
	Filename length (2 bytes)
	Filename (raw bytes)

SaveFileRequest adds:
	File size (4 bytes)
	File payload (binary file content)