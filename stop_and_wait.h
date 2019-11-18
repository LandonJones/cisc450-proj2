#include <stdio.h> 
#include <stdlib.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <string.h> 
typedef struct packet{ 
    short count; 
    short seq_num; 
    char data[80]; 
} packet_t; 

packet_t* make_pkt(short, short, char*); 
void delete_pkt(packet_t*); 
 
