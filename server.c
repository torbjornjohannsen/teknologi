#include "universal.h"

int main(void)
{
    SOCKET listenfd = INVALID_SOCKET, connfd = INVALID_SOCKET;

    struct sockaddr_in serv_addr;

    char sendBuff[1500]; // to fit better within the Ethernet MTU

    SetupKeys(); 
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    printf("socket retrieve success\n");

    memset(&serv_addr, 0, sizeof(serv_addr));
    memset(sendBuff, 0, sizeof(sendBuff));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(gangport);

    if(bind(listenfd, &serv_addr, sizeof(serv_addr)) == -1)
    {
        printf("Failed to bind: %d \n", errno);
        return 1;
    }

    if(listen(listenfd, 10) == -1)
    {
        printf("Failed to listen\n");
        return 1;
    }
    connfd = accept(listenfd, (struct sockaddr*)NULL ,NULL); // accept awaiting request
    
    send(connfd, "2", sizeof("2"), 0); 

    Message* msg;
    msg = malloc(sizeof(Message));
    while(1) {
        if(RecieveAndConfirm(connfd, msg) == rsp_Kill) {
            printf("Connection closed\n"); 
            break; 
        } 

        printf("Got msg: %d %d\n", msg->action, msg->port); 
        if(msg->action == act_disconnect) { break; }

        sleep(1);
    }

    shutdown(connfd, 2);  // shutdown input and output streams, i.e. close TCP connection
    closesocket(connfd);  // free fd

    return 0;
}