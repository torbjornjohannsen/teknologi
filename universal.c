#include "universal.h"

void SetupKeys(translate_Key **tKeys)
{
    FILE *file = fopen("actions.txt", "r");

    char *buffer;
    buffer = malloc(sizeof(char) * 64);
    size_t sBuffer = sizeof(buffer);
    translate_Key pBuff[64];
    int read, c = 0;

    while (feof(file) == 0)
    {
        read = getline(&buffer, &sBuffer, file);
        sscanf(buffer, "%s %hd", pBuff[c].text, &pBuff[c].code);
        c++;
        //printf("%s -> %hd %s\n", buffer, pBuff[c - 1].num, pBuff[c - 1].name);
    }
    translate_Key* portArr = malloc(sizeof(translate_Key) * (c + 1));
    for (int i = 0; i < c; i++)
    {
        portArr[i] = pBuff[i - 1];
    }

    portArr[c].code = -1;
    *tKeys = portArr; 
}

int RecieveAndConfirm(SOCKET conn, Message *msg)
{

    char buffer[128];
    int recievedAmt = recv(conn, buffer, sizeof(buffer) - 1, 0);

    if (recievedAmt < 1)
    {
        return rsp_Kill;
    }

    buffer[recievedAmt] = 0;

    if (sscanf(buffer, "%hd %hd", &msg->action, &msg->port) < 1)
    {
        return rsp_Invalid;
    }

    sprintf(buffer, "%d", act_confirmation);

    if (send(conn, buffer, strlen(buffer), 0) <= 0)
    {
        return err_NoGetOrSend;
    }

    return rsp_Normal;
}

int GetConfirm(SOCKET conn)
{
    char buffer[128];
    if (recv(conn, buffer, sizeof(buffer) - 1, 0) < 0)
    {
        return err_NoGetOrSend;
    };
    int code;

    if (sscanf(buffer, "%d", &code) < 1 || code != act_confirmation)
    {
        return rsp_Invalid;
    }

    return rsp_Normal;
}

int Translate(Message *out, char *in, int inLen, translate_Key *keyArr, Port *portArr)
{
    char buff1[16], buff2[16];
    
    //printf("keys: %d and ports: %d\n", keyArr, portArr); 

    if (sscanf(in, "%s %s", buff1, buff2) < 1)
    {
        return rsp_Invalid;
    }
    int c = 1, i = 0;
    while(keyArr[i].code >= 0)
    {
        if (strcmp(buff1, keyArr[i].text) == 0)
        {
            out->action = keyArr[i].code;
            c = 0; 
            break;
        }
        i++; 
    }
    if(c > 0) { return rsp_Invalid; }
    while (portArr[c].num >= 0)
    {
        if (strcmp(buff2, portArr[c].name) == 0)
        {
            out->port = portArr[c].num;
            return rsp_Normal;
        }
        c++; 
    }

    return rsp_Invalid;
}

void SetupPorts(Port **ports)
{
    FILE *file = fopen("ports.txt", "r");

    char *buffer;
    buffer = malloc(sizeof(char) * 64);
    size_t sBuffer = sizeof(buffer);
    Port pBuff[64];
    int read, c = 0;

    while (feof(file) == 0)
    {
        read = getline(&buffer, &sBuffer, file);
        sscanf(buffer, "%s %hd", pBuff[c].name, &pBuff[c].num);
        c++;
        //printf("%s -> %hd %s\n", buffer, pBuff[c - 1].num, pBuff[c - 1].name);
    }
    Port* portArr = malloc(sizeof(Port) * (c + 1));
    for (int i = 0; i < c; i++)
    {
        portArr[i] = pBuff[i - 1];
    }

    portArr[c].num = -1;
    *ports = portArr; 
}