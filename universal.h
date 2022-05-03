#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "netinet/in.h"
#include "netdb.h"
#include "unistd.h"
#include "errno.h"
#include "arpa/inet.h"
typedef int SOCKET;
#define INVALID_SOCKET -1
#define closesocket(s) close(s)
#define gangport 4200
#define max_Port 20

typedef struct 
{
    int16_t action; 
    int16_t port; 
} Message;

typedef struct {
    char text[16]; 
    int16_t code;  
} translate_Key;

//Actions
// a message has the following format: 
// ACTION PORT_NUM 
#define act_switchPort 0 // changes a port from low to high or vice versa
#define act_disconnect 1 // tells the other side youre closing the connection, port num is irrelevent
#define act_confirmation 2 // tells the other side you got the last msg and ready for another, port ignored

// ERROR CODES 
#define err_MsgTooLong -1 
#define err_NoGetOrSend -2

// RESPONSE CODES 
#define rsp_Normal 0 
#define rsp_Kill 1 
#define rsp_Invalid 2

int RecieveAndConfirm(SOCKET conn, Message *msg); 

int GetConfirm(SOCKET conn); 

// caveman method
#define keyAmt 3
translate_Key* keys; 
void SetupKeys(); 

int Translate(Message *out, char *in, int inLen); 

