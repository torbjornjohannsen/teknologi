#include "universal.h"

void SetupKeys(translate_Key **tKeys)
{
    FILE *file = fopen("actions.txt", "r");

    char *buffer;
    buffer = malloc(sizeof(char) * 64);
    size_t sBuffer = sizeof(buffer);
    translate_Key pBuff[64];
    int c = 0;

    while (feof(file) == 0)
    {
        getline(&buffer, &sBuffer, file);
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

    if (sscanf(buffer, "%hd %hd %d %d", &msg->action, &msg->port, &msg->amt, &msg->time) < 1)
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
        return code;
    }

    return rsp_Normal;
}

int Translate(Message *out, char *in, int inLen, translate_Key *keyArr, Port *portArr)
{
    char buff1[32];

    if (sscanf(in, "%s", buff1) < 1)
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
    
    char buff2[32], buff3[32];
    switch (out->action)
    {
    case act_turnOn:
    case act_turnOff:
        sscanf(in, "%s %s", buff1, buff2); 
        break; 
    case act_rotate: 
        sscanf(in, "%s %s %d %d", buff1, buff2, &out->time, &out->amt); 
        break; 
    case act_turnForDuration: 
        sscanf(in, "%s %s %d", buff1, buff2, &out->time); 
        break;
    case act_wait: 
        sscanf(in, "%s %d", buff1, &out->time); 
        return rsp_Normal;  
    case act_get: 
        sscanf(in, "%s %s %s", buff1, buff2, buff3); 
        i = 0; 
        while(portArr[i].num >= 0)
        {
            if (strcmp(buff3, portArr[i].name) == 0)
            {
                out->amt = portArr[i].num;
                c = 0; 
                break;
            }
            i++; 
        }
        break; 
    default:
        return rsp_Invalid; 
        break;
    }
    i = 0; 
    while(portArr[i].num >= 0)
    {
        //printf("%d : %s = %s\n", i, buff2, portArr[i].name);
        if(strcmp(buff2, portArr[i].name) == 0)
        {
            out->port = portArr[i].num; 
            return rsp_Normal; 
        }
        i++; 
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
        if(read <= 0) { break; }
        sscanf(buffer, "%s %hd", pBuff[c].name, &pBuff[c].num);
        c++;
        //printf("%s -> %hd %s\n", buffer, pBuff[c - 1].num, pBuff[c - 1].name);
    }
    Port* portArr = malloc(sizeof(Port) * (c + 1));
    for (int i = 0; i < c; i++)
    {
        strcpy(portArr[i].name, pBuff[i].name);
        portArr[i].num = pBuff[i].num;
        //printf("%d/%d : %hd %s\n", i, c, pBuff[i].num, pBuff[i].name);
    }

    portArr[c].num = -1;
    *ports = portArr; 
}