#include "stop_and_wait.h" 


packet_t* make_pkt(short count, short seq_num, char* data){ 
    packet_t* new_pkt = (packet_t *)malloc(sizeof(packet_t));
    new_pkt->count = count; 
    new_pkt->seq_num = seq_num; 
    strcpy(new_pkt->data, data); 
    return new_pkt;     
} 
void delete_pkt(packet_t* pkt){ 
    free(pkt->data); 
    free(pkt); 
} 
