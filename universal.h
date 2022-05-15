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
#define max_Port 28
#define timeBtwPolls 50000

typedef struct 
{
    int16_t action; 
    int16_t port;
    int time; 
    int amt;  
} Message;


typedef struct {
    char text[16]; 
    int16_t code;  
} translate_Key;

typedef struct {
    char name[16];
    int16_t num; 
} Port; 

//Actions
// a message has the following format: 
// ACTION PORT_NUM 
#define act_turnOn 0 // changes a port to high
#define act_disconnect 1 // tells the other side youre closing the connection, port num is irrelevent
#define act_confirmation 2 // tells the other side you got the last msg and ready for another, port ignored
#define act_turnOff 3 // changes a port to low
#define act_rotate 4 // sendes alternating high/low signals, given # steps and time per step  
#define act_turnForDuration 5 // turn on port then turn it off again after given time 
#define act_refuse 6 // tells client that the action could not be completed 
#define act_kill 7 // tells the server to shutdown
#define act_wait 8 // tells the server to sleep x time in main thread. Other threads keep executing like normal 
#define act_get 9 // tells the server to read the input of some port 
#define act_runInstructions 10 // tells the server to follow the instructions file 

// ERROR CODES 
#define err_MsgTooLong -1 
#define err_NoGetOrSend -2

// RESPONSE CODES 
#define rsp_Normal 0 
#define rsp_Kill 1 
#define rsp_Invalid 2

int RecieveAndConfirm(SOCKET conn, Message *msg); 

int GetConfirm(SOCKET conn); 

void SetupKeys(translate_Key** tKeys); 

void SetupPorts(Port** ports); 

int Translate(Message *out, char *in, int inLen, translate_Key *keyArr, Port *portArr); 

