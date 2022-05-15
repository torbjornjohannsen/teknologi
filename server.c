#include "universal.h"
#include <pthread.h>

typedef struct {
    int port; 
    int interval;
    int rotations;
} rotArgs; 

void *RotationLoop(void *args) 
{
    rotArgs* info = args; 
    char action1[32], action2[32]; 
    sprintf(action1, "raspi-gpio set %d dh", info->port); 
    sprintf(action2, "raspi-gpio set %d dl", info->port); 
    printf("Starting rotation of %d: %d times with %d interval\n", info->port, info->rotations, info->interval); 
    for (int i = 0; i < info->rotations; i++)
    {
        system(action1); 
        usleep(1000); 
        system(action2);
        usleep(info->interval * 1000); 
    }
    printf("Stopping rotation of %d\n", info->port); 
    free(info); 
    printf("Rotator dead\n");
}

typedef struct {
    int port; 
    int time; 
} turnDurArgs; 

void *TurnDurationLoop(void *args)
{
    turnDurArgs* info = args; 
    char action1[32], action2[32]; 
    sprintf(action1, "raspi-gpio set %d dh", info->port); 
    sprintf(action2, "raspi-gpio set %d dl", info->port); 
    printf("Turning on port %d w time %d\n", info->port, info->time); 
    system(action1);
    usleep(info->time * 1000); 
    printf("Turning off port %d\n", info->port); 
    system(action2);
    free(info); 
    printf("Turner dead\n");
}

typedef struct {
    int port; 
    int actPort;
    int* on; 
    int* changed; 
} monitorArgs; 

void *MonitorPort(void *args)
{
    monitorArgs* info = args;  
    char action[32];
    sprintf(action, "raspi-gpio get %d", info->port); 
    while(1)
    {
        FILE* input = popen(action, "r");
        while(feof(input) == 0)
        {
            if(fgetc(input) == '=') 
            {
                char state = fgetc(input);
                int numState = atoi(&state);
                if(numState != *info->on) {
                    printf("ST: Port %d changed to %d from %d\n", info->port, numState, *info->on); 
                    *info->on = numState; 
                    sprintf(action, "raspi-gpio set %d %s", info->actPort, *info->on == 0 ? "dl" : "dh");
                    printf("Executing: %s cause event happened\n", action);
                    system(action); 
                }
                break; 
            }
        }
        usleep(timeBtwPolls); 
    }
    free(info); 
    printf("Monitor dead\n");
}

int ExecuteCommand(Message *msg, int* amtMPorts, pthread_t* tNum, int *monitoredPorts)
{
    int qwe = 123; 
    char action[32];
    
    switch (msg->action)
    {
    case act_turnOn:
        sprintf(action, "raspi-gpio set %d dh", msg->port); 
        system(action); 
        printf("Executing: \"%s\"\n", action);  
        break;
    case act_turnOff:
        sprintf(action, "raspi-gpio set %d dl", msg->port); 
        system(action); 
        printf("Executing: \"%s\"\n", action);  
        break;
    case act_disconnect: 
        sprintf(action, "./gpiosetup.sh"); 
        break;
    case act_turnForDuration: ;
    {
        turnDurArgs *args = malloc(sizeof(args)); 
        args->port = msg->port; 
        args->time = msg->time; 
        pthread_t *sTNum = malloc(sizeof(pthread_t));
        *sTNum = *tNum; 
        pthread_create(sTNum, NULL, TurnDurationLoop, args); 
        (*tNum)++;
        break;
    } 
    case act_rotate: ;
    {
        rotArgs *rargs = malloc(sizeof(rotArgs)); 
        rargs->port = msg->port; 
        rargs->interval = msg->time; 
        rargs->rotations = msg->amt; 
        pthread_t *sTNum = malloc(sizeof(pthread_t));
        *sTNum = *tNum; 
        pthread_create(sTNum, NULL, RotationLoop, rargs); 
        (*tNum)++; 
        break; 
    }
    case act_kill: 
        printf("Killing server\n");
        return -1; 
    case act_wait:  
        printf("Waiting for %d millisec\n", msg->time);
        usleep(msg->time * 1000); 
        break;
    case act_get:
        for (int i = 0; i < *amtMPorts; i++)
        {
            if(monitoredPorts[i] == msg->port) {
                goto END; 
            }
        }
        printf("Reading input from port %d", msg->port);
        monitorArgs* margs = malloc(sizeof(monitorArgs)); 
        margs->port = msg->port; 
        margs->actPort = msg->amt; 
        monitoredPorts[*amtMPorts] = margs->port; 
        margs->on = malloc(sizeof(int)); 
        (*amtMPorts)++; 

        pthread_t *sTNum = malloc(sizeof(pthread_t)); 
        *sTNum = *tNum; 
        pthread_create(sTNum, NULL, MonitorPort, margs); 
        (*tNum)++;    
    END: 
        break;
    default:
        // send this to the client too 
        printf("Invalid action requested\n");
        break;
    }
    return 0; 
}

int main(int argc, char* argv[])
{
    int connect = 1; 
    if(argc == 2) 
    {
        if(strcmp(argv[1], "-i") == 0) {
            connect = 0; 
        }
    }
    system("./gpiosetup.sh");

    SOCKET listenfd = INVALID_SOCKET, connfd = INVALID_SOCKET;

    struct sockaddr_in serv_addr;

    char sendBuff[1500]; // to fit better within the Ethernet MTU

    translate_Key* tKeys; 
    SetupKeys(&tKeys);
    Port* ports; 
    SetupPorts(&ports);

    int amtMPorts = 0; 
    pthread_t tNum = 0;
    int* monitoredPorts; 
    monitoredPorts = malloc(sizeof(int) * 64);  
    

    if(connect)
    {
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
    }

    int loop = 1;
    Message* msg = malloc(sizeof(Message)); 

    while(loop && connect)
    {
        connfd = accept(listenfd, (struct sockaddr*)NULL ,NULL); // accept awaiting request
    
        send(connfd, "2", sizeof("2"), 0); 
        
        while(1) {
            if(RecieveAndConfirm(connfd, msg) == rsp_Kill) {
                printf("Connection closed\n"); 
                break; 
            } 
            if(ExecuteCommand(msg, &amtMPorts, &tNum, monitoredPorts) != 0) {
                loop = 0; 
            }
            //printf("Got: %d %d %d %d\n", msg->action, msg->port, msg->amt, msg->time);
            
            sleep(1);
        }

        shutdown(connfd, 2);  // shutdown input and output streams, i.e. close TCP connection
        closesocket(connfd);  // free fd
    }
    while(loop)
    {
        printf("qwe\n");
        FILE *file = fopen("instructions.txt", "r");
        char *buffer = malloc(sizeof(char) * 128);
        size_t sBuffer = sizeof(buffer);
    
        while(feof(file) == 0) 
        {
            int read = getline(&buffer, &sBuffer, file);
            if(read <= 0) { break; }
            if (Translate(msg, buffer, sizeof(buffer), tKeys, ports) != rsp_Normal)
            {
                printf("Wrong input from file: %s\n", buffer); 
                continue;
            }
            if(ExecuteCommand(msg, &amtMPorts, &tNum, monitoredPorts) != 0) {
                loop = 0; 
            }
            
        }
    }
    return 0;
}