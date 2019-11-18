/* udp_server.c */
/* Programmed by Adarsh Sethi */
/* Sept. 19, 2019 */

#include <ctype.h>          /* for toupper */
#include <stdio.h>          /* for standard I/O functions */
#include <stdlib.h>         /* for exit */
#include <string.h>         /* for memset */
#include <sys/socket.h>     /* for socket, sendto, and recvfrom */
#include <netinet/in.h>     /* for sockaddr_in */
#include <unistd.h>         /* for close */
#include <math.h> 
#include "stop_and_wait.h" 
#define STRING_SIZE 1024

/* SERV_UDP_PORT is the port number on which the server listens for
   incoming messages from clients. You should change this to a different
   number to prevent conflicts with others in the class. */

#define SERV_UDP_PORT 65100

int simulateLoss(float ratio){ 
     float r = rand() / (double)RAND_MAX; 
     if (r < ratio){ 
        printf("LOSS\n"); 
        return 1; 
     } 
     else{ 
        return 0; 
     } 
} 
int main(int argc, char* argv[]) {

   int sock_server;  /* Socket on which server listens to clients */

   struct sockaddr_in server_addr;  /* Internet address structure that
                                        stores server address */
   unsigned short server_port;  /* Port number used by server (local port) */

   struct sockaddr_in client_addr;  /* Internet address structure that
                                        stores client address */
   unsigned int client_addr_len;  /* Length of client address structure */

   char filename[STRING_SIZE];  /* receive message */
   char modifiedSentence[STRING_SIZE]; /* send message */
   unsigned int msg_len;  /* length of message */
   int bytes_sent, bytes_recd; /* number of bytes sent or received */
   unsigned int i;  /* temporary loop variable */
   
   int n_timeout; 
   float packet_loss_ratio; 
   /* open a socket */
   if (argc != 3){ 
      printf("Usage: ./udpserver timeout packet_loss_ratio\n"); 
      exit(1); 
   } 
   
   n_timeout = atoi(argv[1]); 
   if (n_timeout < 1 || n_timeout > 10){ 
      printf("timeout should be between 1-10\n"); 
      exit(1); 
   } 
   packet_loss_ratio = atof(argv[2]); 
   printf("%f\n", packet_loss_ratio); 
   if ((sock_server = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
      perror("Server: can't open datagram socket\n");
      exit(1);
   }

   /* initialize server address information */

   memset(&server_addr, 0, sizeof(server_addr));
   server_addr.sin_family = AF_INET;
   server_addr.sin_addr.s_addr = htonl (INADDR_ANY);  /* This allows choice of
                                        any host interface, if more than one
                                        are present */
   server_port = SERV_UDP_PORT; /* Server will listen on this port */
   server_addr.sin_port = htons(server_port);

   /* bind the socket to the local server port */

   if (bind(sock_server, (struct sockaddr *) &server_addr,
                                    sizeof (server_addr)) < 0) {
      perror("Server: can't bind to local address\n");
      close(sock_server);
      exit(1);
   }

   /* wait for incoming messages in an indefinite loop */

   printf("Waiting for incoming messages on port %hu\n\n", 
                           server_port);

   client_addr_len = sizeof (client_addr);

   for (;;) {

      bytes_recd = recvfrom(sock_server, &filename, STRING_SIZE, 0,
                     (struct sockaddr *) &client_addr, &client_addr_len);
      printf("%d\n", bytes_recd); 
      
      filename[bytes_recd] = '\0'; 
      printf("Received Sentence is: %s\n     with length %d\n\n",
                         filename, bytes_recd);
       
      FILE* f = fopen(filename, "r"); 
      
      if (f == NULL){ 
          perror("Error opening file:"); 
	  char resp[19] = "Error opening file"; 
          resp[19] = '\0'; 
          bytes_sent = sendto(sock_server, resp, strlen(resp), 0, 
		   (struct sockaddr*) &client_addr, client_addr_len); 
          //close(sock_server); 
          continue; 
      } 
      /* prepare the message to send */
      
      msg_len = bytes_recd;
      short nextseqnum = 0;
      packet_t* pkt = (packet_t *)malloc(sizeof(packet_t));  
      while (fgets(modifiedSentence, 80, f)){ 

          // fgets(modifiedSentence, STRING_SIZE, f); 
      /* send message */
          packet_t* new_pkt = make_pkt(strlen(modifiedSentence), nextseqnum, modifiedSentence); 
	  
          printf("%s\n", modifiedSentence); 
          
          if (!simulateLoss(packet_loss_ratio)){ 
          	bytes_sent = sendto(sock_server, new_pkt, sizeof(packet_t), 0,
                   (struct sockaddr*) &client_addr, client_addr_len);
          } 
          int wait_for_ack = 1; 
          while (wait_for_ack){ 
              struct timeval timeout; 
              timeout.tv_sec = (int)pow(10, n_timeout) / (int)pow(10, 6); 
              timeout.tv_usec = (int)pow(10, n_timeout) % (int)pow(10, 6); 
              setsockopt(sock_server, SOL_SOCKET, SO_RCVTIMEO, 
                  (const void *) &timeout, sizeof(timeout)); 
          
              bytes_recd = recvfrom(sock_server, pkt, sizeof(packet_t), 
                              0, (struct sockaddr*) &client_addr, &client_addr_len); 
              if (bytes_recd <= 0){ 
                  // timeout occurred
                  printf("Here?\n"); 
                  if (!simulateLoss(packet_loss_ratio)){ 
                  	bytes_sent = sendto(sock_server, new_pkt, sizeof(packet_t), 0, 
                        	(struct sockaddr*) &client_addr, client_addr_len); 
                  } 
                  // then loop around and wait for ack again
              }   
              else{ 
                  // packet received 
                  printf("%hu\n", pkt->seq_num); 
                  if (nextseqnum == pkt->seq_num){ 
                      printf("yes\n");
                      wait_for_ack = 0; 
                  } 
                  
              }
          } 
          // wait for ack has finished
          nextseqnum = 1 - nextseqnum;  
      } 
      packet_t* EOT = make_pkt(0, nextseqnum, ""); 
      bytes_sent = sendto(sock_server, EOT, sizeof(packet_t), 0, (struct sockaddr*)&client_addr, client_addr_len); 
      
      fclose(f); 
      exit(0);  
      //close(sock_server); 
   }
}
