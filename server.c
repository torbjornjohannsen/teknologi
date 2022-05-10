#include "universal.h"
#include <pthread.h>

int runloops = 1; 

typedef struct {
    int port; 
    int interval;
} argStruct; 

void *RunLoop(void *args) 
{
    argStruct* info = args; 
    char action1[32], action2[32]; 
    sprintf(action1, "raspi-gpio set %d dh", info->port); 
    sprintf(action2, "raspi-gpio set %d dl", info->port); 
    printf("%s\n", action1);
    printf("%s\n", action2);
    while(runloops)
    {
        system(action1); 
        usleep(info->interval * 1000); 
        system(action2);
        usleep(info->interval * 1000); 
        printf("Completed cycle\n");
    }
}

int main(void)
{
    SOCKET listenfd = INVALID_SOCKET, connfd = INVALID_SOCKET;

    struct sockaddr_in serv_addr;

    char sendBuff[1500]; // to fit better within the Ethernet MTU

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
    while(1)
    {
        runloops = 1; 
        connfd = accept(listenfd, (struct sockaddr*)NULL ,NULL); // accept awaiting request
    
        send(connfd, "2", sizeof("2"), 0); 

        Message* msg;
        msg = malloc(sizeof(Message));
        while(1) {
            if(RecieveAndConfirm(connfd, msg) == rsp_Kill) {
                printf("Connection closed\n"); 
                break; 
            } 

            pthread_t tId; 
            argStruct args; 
            args.interval = 500; 
            args.port = 18; 
            pthread_create(&tId, NULL, RunLoop, &args); 
            /* char action[32];
            switch (msg->action)
            {
            case act_turnOn:
                sprintf(action, "raspi-gpio set %d dh", msg->port); 
                break;
            case act_turnOff:
                sprintf(action, "raspi-gpio set %d dl", msg->port); 
                break;
            case act_disconnect: 
                sprintf(action, "./gpiosetup.sh"); 
                break;
            default:
            // send this to the client too 
                printf("Invalid action requested\n");
                break;
            }
            system(action); 
            printf("Executing: \"%s\"\n", action);  */
            if(msg->action == act_disconnect) { break; }
            sleep(1);
        }

        shutdown(connfd, 2);  // shutdown input and output streams, i.e. close TCP connection
        closesocket(connfd);  // free fd
        runloops = 0; 
    }
    return 0;
}