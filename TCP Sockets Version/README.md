# Server-Client-Communication
Built a client/server system for movie information retrieval using TCP sockets, with the server being
able to handle multiple concurrent connections by forking child processes to handle them.
Needs a fix to clean up the child processes.

The client reads responses from the server on one thread, while accepting user requests on the main thread.
