gcc -o client.out client.c universal.c
echo "//////////////////////////
/                        /
/                        /
/   Server Compilation:  /
/                        /
/                        /
//////////////////////////"
gcc -pthread -o server.out server.c universal.c
echo "\"./server.out\" for server script \"./client.out server_IP\" for client script"