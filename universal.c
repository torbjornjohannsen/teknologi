#include "universal.h"

void SetupKeys(){
    keys = malloc(sizeof(translate_Key) * keyAmt); 
    keys[0].code = act_switchPort; 
    strcpy(keys[0].text, "ON"); 

    keys[1].code = act_switchPort; 
    strcpy(keys[1].text, "OFF"); 

    keys[2].code = act_switchPort; 
    strcpy(keys[2].text, "SWITCH"); 
}

int RecieveAndConfirm(SOCKET conn, Message *msg) {

    char buffer[128];
    int recievedAmt = recv(conn, buffer, sizeof(buffer) - 1, 0);

    if(recievedAmt < 1) { return rsp_Kill; }

    buffer[recievedAmt] = 0; 

    if(sscanf(buffer, "%hd %hd", &msg->action, &msg->port) < 1) { return rsp_Invalid; }

    sprintf(buffer, "%d", act_confirmation); 

    if(send(conn, buffer, strlen(buffer), 0) <= 0) { return err_NoGetOrSend; } 

    return rsp_Normal; 
}

int GetConfirm(SOCKET conn) {
    char buffer[128];
    if(recv(conn, buffer, sizeof(buffer) - 1, 0) < 0) { return err_NoGetOrSend; };
    int code;

    if(sscanf(buffer, "%d", &code) < 1 || code != act_confirmation) { return rsp_Invalid; }

    return rsp_Normal; 
}

int Translate(Message *out, char *in, int inLen) {
    char buff[16]; 
    if(sscanf(in, "%s %hd", buff, &out->port) < 2 || out->port > max_Port) { return rsp_Invalid; }

    for (int i = 0; i < keyAmt; i++)
    {
        if(strcmp(buff, keys[i].text) == 0) {
            out->action = keys[i].code; 
            return rsp_Normal; 
        }
    }
    
    return rsp_Invalid; 
}