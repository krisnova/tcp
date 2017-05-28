#include "client.h"
#include "../libnetrep/ip.h"
#include "../libnetrep/tcp.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <linux/in.h>
#include <arpa/inet.h>

int main(int argc , char *argv[]){


    // Create a new raw TCP socket
    int s = socket(PF_INET, SOCK_RAW, IPPROTO_TCP);
        if(s == -1){
            printf("Unable to create socket\n");
            exit(1);
        }

    // Allocate our variables
    char packet[4096];
    char source_ip[32];
    char remote_ip[32];
    strcpy(source_ip, localip());
    strcpy(remote_ip, remoteip());
    char *body;
    char *packet_ptr;
    memset (packet, 0, 4096);
    struct iphdr *iph = (struct iphdr *) packet;
    struct tcphdr *tcph = (struct tcphdr *) (packet + sizeof (struct ip));
    struct sockaddr_in sin;
    struct tcphdr ps;

    // Define the packet
    body = packet + sizeof(struct iphdr) + sizeof(struct tcphdr);
    strcpy(body , "todo make this a flag");


    // Build the packet
    sin.sin_family = AF_INET;
    sin.sin_port = htons(80); // HTTP
    sin.sin_addr.s_addr = inet_addr (remote_ip);

    printf("\n");
    printf("[Local]   IP Address [%s]\n", source_ip);
    printf("[Remote]  IP Address [%s]\n", remote_ip);


    // IP Header
    iph->version = 4;
    iph->ihl = 5;
    iph->tos = 0;
    iph->tot_len = sizeof (struct iphdr) + sizeof (struct tcphdr) + strlen(body);
    iph->id = htonl (1);
    iph->frag_off = 0;
    iph->ttl = 255;
    iph->protocol = IPPROTO_TCP;
    iph->check = csum ((unsigned short *) packet, iph->tot_len);
    iph->saddr = inet_addr(source_ip);
    iph->daddr = sin.sin_addr.s_addr;
    print_iphdr(iph);


    //TCP Header
    tcph->source = htons (1234);
    tcph->dest = htons (80);
    tcph->seq = 0;
    tcph->ack_seq = 0;
    tcph->doff = 5;  //tcp header size
    tcph->fin=0;
    tcph->syn=1;
    tcph->rst=0;
    tcph->psh=0;
    tcph->ack=0;
    tcph->urg=0;
    tcph->window = htons (5840); /* maximum allowed window size */
    tcph->check = 0; //leave checksum 0 now, filled later by pseudo header
    tcph->urg_ptr = 0;
    print_tcphdr(tcph);


    return 0;


}

char* remoteip() {
    return NIVENLY_IP;
}

char* localip() {
    int file_descriptor;
    struct ifreq ifr;
    file_descriptor = socket(AF_INET, SOCK_DGRAM, 0);
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);
    ioctl(file_descriptor, SIOCGIFADDR, &ifr);
    close(file_descriptor);
    return inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
}

unsigned short csum(unsigned short *ptr, int nbytes){
	register long sum;
	unsigned short oddbyte;
	register short answer;
	sum = 0;
	while (nbytes > 1)
	{
		sum += *ptr++;
		nbytes -= 2;
	}
	if (nbytes == 1)
	{
		oddbyte = 0;
		*((u_char*) &oddbyte) = *(u_char*) ptr;
		sum += oddbyte;
	}
	sum = (sum >> 16) + (sum & 0xffff);
	sum = sum + (sum >> 16);
	answer = (short) ~sum;
	return (answer);
}