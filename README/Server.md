Backup Server (C++ / Boost.Asio)

Short Explanation:
	Here, you can see an implementation of a multithreaded backup server.
	Clients can upload, list, download, and delete files using a custom TCP protocol.
	The server is built with Boost.Asio for networking and uses threads to handle multiple clients concurrently.
	I use little endian for my protocol

How the Server Works?

1. Entry Point (main.cpp)
	Creates a Boost.Asio I/O context
	Initializes a Server listening on port 5000
	Calls run(), which accepts incoming clients in a loop

2. The Server
	Listens on all interfaces (0.0.0.0) at the given port
	For each new connection:
		Accepts a TCP socket
		Starts a new thread to handle the client using ClientHandler
		This makes the server stateless: each request is handled independently, no global session storage.

3. Client Handling
ClientHandler::start_session(socket) runs in a separate thread for each client.
Steps inside:
	Read the request header:
	uid (user ID, 4 bytes)
	version (1 byte)
	opcode (1 byte, defines the action)
	Dispatch to the right handler:
	handle_list_req → generate a file list
	handle_save_req → receive a file and store it
	handle_retrieve_req → send a file back
	handle_delete_req → delete a file
	Send back a response using the Serializer helper.

4. Stateless Protocol
	Every request includes the user ID (uid) → no session is stored between requests.
	The server uses that uid to find the user’s backup folder (C:\backupsrv\<uid>\).
	Each request stands on its own → if the client disconnects, nothing breaks.

5. File Management
	Save (Upload):
		The client sends filename, size, and payload.
		The server writes the file under C:\backupsrv\<uid>\.
	List:
		The server creates a temporary file with all filenames.
		Temporary files are named with a random 32-character alphanumeric string.
		TempFile struct ensures the file is deleted automatically when done.
	Retrieve (Download):
		The server sends back the requested file’s size and content.
	Delete:
		The server deletes the file from the user’s backup folder.

6. Responses
	Responses are built with the Serializer utility:
	Response → contains protocol version and status code
	PartialFileResponse → adds filename info
	FullFileResponse → adds file size
	The client uses these to understand if the request succeeded and what data follows.

Key Features:

Multithreading: each client runs in its own thread
Stateless design: no long-lived sessions, everything comes from request data
Temporary files: used safely with RAII to ensure cleanup
Filesystem safety: prevents directory traversal with safe_path()
Custom protocol: efficient binary protocol using struct.pack / struct.unpack (Python) and Serializer (C++)

Example Flow:
	1. Client sends a LIST_FILES request.
	Server:
		Scans C:\backupsrv\<uid>\
		Writes filenames to a temp file
		Sends the temp filename + its content back
		Deletes the temp file automatically
		Client prints the list of backed-up files.


2. Client sends a SAVE_FILE request with: filename, file size, and file content.
	Server:
		Creates C:\backupsrv\<uid>\ if it doesn’t exist
		Writes the file there
		Sends back a success response
		Client prints confirmation that the file was saved.


3. Client sends a RETRIEVE_FILE request with the filename.
	Server:
		Looks for the file under C:\backupsrv\<uid>\
		If file exists: sends file size + file content
		If not: sends “file does not exist” error
		Client saves the retrieved file into Retrieved Files\<uid>\.


4. Client sends a DELETE_FILE request with the filename.
	Server:
		Deletes the file if it exists
		Sends back status: success or error
		Client prints whether the deletion succeeded.



Notes & Limitations
	Runs only on Windows (uses WIN32_FIND_DATAA).
	No authentication → UID is the only identifier.
	Plain TCP (no encryption).


Compile & Run:
	run in cmd:
		1. cd "path/to/project"

		2. g++ -std=c++20 -D_WIN32_WINNT=0x0601 -fexec-charset=UTF-8 -I ".\include" .\main.cpp .\Exceptions\GeneralException.cpp .\Network\Server.cpp .\Network\ClientHandler.cpp .\Utils\Serializer.cpp -o Server.exe -lws2_32
	
		3. .\Server.exe

	
