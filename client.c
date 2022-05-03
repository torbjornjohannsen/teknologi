#include "universal.h"

int main(void)
{
    SOCKET sockfd = INVALID_SOCKET;
    int n = 0;
    char recvBuff[64]; // to fit better within the Ethernet MTU
    struct sockaddr_in serv_addr;
    SetupKeys(); 

    memset(recvBuff, 0, sizeof(recvBuff));
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(gangport);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)))
    {
        printf("\n Error : Connect Failed \n");
        return 1;
    }

    char msgBuf[64]; 
    Message* msg;
    msg = malloc(sizeof(Message));
    int bytesLeft = 0, t_bytesSent = 0; 
    while(1)
    {
        int rsp = GetConfirm(sockfd);
        if(rsp != rsp_Normal) {
            printf("Instead of normal confirmation rsp, got %d response\n", rsp); 
            break; 
        }
        printf("Server ready to recieve, type your command:\n"); 

GetMsg: 
        gets(msgBuf); 

        if(Translate(msg, msgBuf, sizeof(msgBuf)) != rsp_Normal) {
            printf("Invalid input you dingus, try again:\n"); 
            goto GetMsg; 
        } 

        sprintf(recvBuff, "%d %d", msg->action, msg->port); 

        int sentBytes = send(sockfd, recvBuff, sizeof(recvBuff), 0);
        printf("Sent: %d\n", sentBytes);

        sleep(1);
    } 

    shutdown(sockfd, 2);  // shutdown input and output streams, i.e. close TCP connection
    closesocket(sockfd);  // free fd

    return 0;
}