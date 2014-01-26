#include <ev.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <signal.h>
#include <stdio.h>
#include <pthread.h>

#define PORT			8080

static void read_cb(struct ev_loop *loop, struct ev_io *watcher, int revents){
	if(EV_ERROR & revents){
		//Not sure what to do here...
		return;
	}
	char buffer[1024];
	char *ptr=buffer;
	ssize_t len=recv(watcher->fd, buffer, sizeof(buffer), 0);
	if(len<0){
		perror("recv()");
	}else if(len==0){
		fprintf(stderr, "Connection closed\n");
		ev_io_stop(loop, watcher);
		free(watcher);
	}else{
		do{
			ssize_t amount=send(watcher->fd, ptr, len, 0);
			if(amount<0){
				perror("send()");
				return;
			}
			len-=amount;
			ptr+=amount;
		}while(len>0);
	}
}
static void accept_cb(struct ev_loop *loop, struct ev_io *watcher, int revents){
	int client=accept(watcher->fd, NULL, NULL);
	if(client<0){
		perror("accept()");
		return;
	}
	fprintf(stderr, "Connection Opened\n");
	struct ev_io *w_client=malloc(sizeof(struct ev_io));
	ev_io_init(w_client, read_cb, client, EV_READ);
	ev_io_start(loop, w_client);
}

static void signal_cb(struct ev_loop *loop, ev_signal *w, int revents){
	ev_break(loop, EVBREAK_ALL);
}

static void exit_cleanly_on(struct ev_loop *loop, int sig){
	ev_signal *watcher=malloc(sizeof(ev_signal));
	ev_signal_init(watcher, signal_cb, sig);
	ev_signal_start(loop, watcher);
}

int main(){
	struct ev_loop *loop=EV_DEFAULT;
	
	exit_cleanly_on(loop, SIGINT);
	exit_cleanly_on(loop, SIGTERM);
	
	struct sockaddr_in addr;
	int server_sock;
	if((server_sock=socket(PF_INET, SOCK_STREAM, 0)) < 0){
		perror("server_socket()");
		abort();
	}
	bzero(&addr, sizeof(addr));
	addr.sin_family=AF_INET;
	addr.sin_port=htons(PORT);
	addr.sin_addr.s_addr=INADDR_ANY;
	if(bind(server_sock, (void*)&addr, sizeof(addr))!=0){
		perror("bind()");
		abort();
	}
	if(listen(server_sock, 64) < 0){
		perror("listen()");
		abort();
	}
	fprintf(stderr, "Listening at 0.0.0.0 on port %d\n", (int)PORT);
	struct ev_io w_accept;
	ev_io_init(&w_accept, accept_cb, server_sock, EV_READ);
	ev_io_start(loop, &w_accept);
	ev_run(loop, 0);
	//Cleanup
	fprintf(stderr, "Closing...");
	close(server_sock);
	fprintf(stderr, "...DONE\n");
	exit(0);
	return 0;
}