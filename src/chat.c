#include "chat.h"
int rx_fd;

void chat_tx(){
        int chat_socket = -1;
        struct sockaddr_in chat_addr;
        char str[BUFSZ];
        getlin("Send chat:", str);
        if(isspace(str[0]) || str[0] == '\033' || str[0] == '\n' || str[0] == 0){ /* user cancel */
                pline("Fine, chat aborted. They probably didn't want to talk to you anyway.");
        }
        else{
                if( (chat_socket = socket(PF_INET, SOCK_DGRAM, 0)) < 0 ) { /* error: can't create socket */
                        pline("You try and call out, but your voice catches in your throat.");
                }
                memset(&chat_addr, 0, sizeof(chat_addr));
                chat_addr.sin_family = AF_INET;
                chat_addr.sin_port = htons(chat_port);
                chat_addr.sin_addr.s_addr = inet_addr(chat_ip_address);
                if(sendto(chat_socket, str, strlen(str), 0, (const struct sockaddr *) &chat_addr, sizeof(chat_addr)) < 0){ /* error: couldn't transmit */
                        pline("You call out, but you have a nagging feeling nobody can hear you...");
                }
                else{
                        pline("Message sent.");
                }
        }
}

void setup_chat_rx(){
	struct sockaddr_in chat_addr;
	struct ip_mreq mreq;

	u_int u_one = 1;
        memset(&chat_addr, 0, sizeof(chat_addr));
        addr.sin_family = AF_INET;
        addr.sin.port = htons(chat_port);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
	mreq.imr_multiaddr.s_addr=inet_addr(chat_ip_address);
	mreq.imr_interface.s_addr=htonl(INADDR_ANY);

	if( 
		((rx_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) && 
		(setsockopt(rx_fd,SOL_SOCKET,SO_REUSEADDR,&u_one,sizeof(u_one)) < 0) && 
		(bind(rx_fd,(struct sockaddr *) &chat_addr,sizeof(chat_addr)) < 0) &&
		(setsockopt(rx_fd,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq)) < 0)
	){
		pline("Error while setting up chat system recieve socket.");
	}
}

//TODO: Implement non-blocking message queue system.
void chat_rx(){
	char str[BUFSZ];
	recvfrom(rx_fd, str, BUFSZ, 0, (struct sockaddr *) &chat_addr, sizeof(chat_addr)){
		pline(str); //ideally, enqueue here
	}
}
