/* udp_client.c */ 
/* Programmed by Adarsh Sethi */
/* Sept. 19, 2019 */

#include <stdio.h>          /* for standard I/O functions */
#include <stdlib.h>         /* for exit */
#include <string.h>         /* for memset, memcpy, and strlen */
#include <netdb.h>          /* for struct hostent and gethostbyname */
#include <sys/socket.h>     /* for socket, sendto, and recvfrom */
#include <netinet/in.h>     /* for sockaddr_in */
#include <unistd.h>         /* for close */

#include "stop_and_wait.h" 
#define STRING_SIZE 1024
int simulateACKLoss(float ratio){ 
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
   
   int sock_client;  /* Socket used by client */ 

   struct sockaddr_in client_addr;  /* Internet address structure that
                                        stores client address */
   unsigned short client_port;  /* Port number used by client (local port) */

   struct sockaddr_in server_addr;  /* Internet address structure that
                                        stores server address */
   struct hostent * server_hp;      /* Structure to store server's IP
                                        address */
   char server_hostname[STRING_SIZE] = "cisc450.cis.udel.edu"; /* Server's hostname */
   unsigned short server_port = 65100;  /* Port number used by server (remote port) */

   char filename[STRING_SIZE];  /* send message */
   char modifiedSentence[STRING_SIZE]; /* receive message */
   unsigned int msg_len;  /* length of message */
   int bytes_sent, bytes_recd; /* number of bytes sent or received */
   
   float ack_loss;
   /* open a socket */
   if (argc != 3){ 
      printf("Usage: ./udpclient filename loss_ratio\n"); 
      exit(1); 
   }
   strcpy(filename, argv[1]); 
   ack_loss = atof(argv[2]); 
   printf("%f\n", ack_loss); 
   if ((sock_client = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
      perror("Client: can't open datagram socket\n");
      exit(1);
   }

   /* Note: there is no need to initialize local client address information
            unless you want to specify a specific local port.
            The local address initialization and binding is done automatically
            when the sendto function is called later, if the socket has not
            already been bound. 
            The code below illustrates how to initialize and bind to a
            specific local port, if that is desired. */

   /* initialize client address information */

   client_port = 0;   /* This allows choice of any available local port */

   /* Uncomment the lines below if you want to specify a particular 
             local port: */
   /*
   printf("Enter port number for client: ");
   scanf("%hu", &client_port);
   */

   /* clear client address structure and initialize with client address */
   memset(&client_addr, 0, sizeof(client_addr));
   client_addr.sin_family = AF_INET;
   client_addr.sin_addr.s_addr = htonl(INADDR_ANY); /* This allows choice of
                                        any host interface, if more than one 
                                        are present */
   client_addr.sin_port = htons(client_port);

   /* bind the socket to the local client port */

   if (bind(sock_client, (struct sockaddr *) &client_addr,
                                    sizeof (client_addr)) < 0) {
      perror("Client: can't bind to local address\n");
      close(sock_client);
      exit(1);
   }

   /* end of local address initialization and binding */

   /* initialize server address information */

   //printf("Enter hostname of server: ");
   //scanf("%s", server_hostname);
   //server_hostname = "cisc450.cis.udel.edu"; 
   if ((server_hp = gethostbyname(server_hostname)) == NULL) {
      perror("Client: invalid server hostname\n");
      close(sock_client);
      exit(1);
   }
   //printf("Enter port number for server: ");
   //scanf("%hu", &server_port);
   //server_port = 65100; 

   /* Clear server address structure and initialize with server address */
   memset(&server_addr, 0, sizeof(server_addr));
   server_addr.sin_family = AF_INET;
   memcpy((char *)&server_addr.sin_addr, server_hp->h_addr,
                                    server_hp->h_length);
   server_addr.sin_port = htons(server_port);

   /* user interface */

   //printf("Please input a file:\n");
   //scanf("%s", filename);
   msg_len = strlen(filename);

   /* send message */
  
   bytes_sent = sendto(sock_client, filename, msg_len, 0,
            (struct sockaddr *) &server_addr, sizeof (server_addr));

   /* get response from server */
   short expectedseqnum = 0; 
   FILE* f = fopen("out", "w"); 
   int eot_recd = 0; 
   
   
   packet_t* result = (packet_t *)malloc(sizeof(packet_t));
   while (1){ 
       printf("Waiting for response from server...\n");
       bytes_recd = recvfrom(sock_client, result, sizeof(packet_t), 0,
                (struct sockaddr *) 0, (int *) 0);
       modifiedSentence[bytes_recd] = '\0'; 
       printf("\nThe response from server is:\n");
       if (result->count == 0){ 
           break; 
       } 
       if (result->seq_num == expectedseqnum){ 
           printf("yes\n"); 
           fprintf(f, "%s", result->data); 
           packet_t* sndpkt = make_pkt(0, expectedseqnum, ""); 
           if (!simulateACKLoss(ack_loss)){ 
           	bytes_sent = sendto(sock_client, sndpkt, sizeof(packet_t), 0, (struct sockaddr *) &server_addr, 
                     sizeof (server_addr)); 
           } 
           expectedseqnum = 1 - expectedseqnum; 
       } 
       else{ 
           packet_t* sndpkt = make_pkt(0, 1 - expectedseqnum, ""); 
           if (!simulateACKLoss(ack_loss)){ 
              bytes_sent = sendto(sock_client, sndpkt, sizeof(packet_t), 0, (struct sockaddr *) &server_addr, 
                     sizeof (server_addr)); 
           } 
       }  
       printf("%d\n", expectedseqnum); 
       //fprintf("%s\n\n", result->data);
   }  
   fclose(f); 
   /* close the socket */

   close (sock_client);
}
