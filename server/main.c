#define _POSIX_SOURCE		`1
#define _BSD_SOURCE		 1
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
#include <strings.h>
#include <sys/types.h>

#define PORT			8080
#define COMMAND			"./run.sh"

static char line_buffer[4096];
static size_t line_buffer_length=0;

static void read_cb(struct ev_loop *loop, struct ev_io *watcher, int revents){
	if(EV_ERROR & revents){
		//Not sure what to do here...
		return;
	}
	char buffer[1024];
	ssize_t len=recv(watcher->fd, buffer, sizeof(buffer), 0);
	if(len<0){
		perror("recv()");
	}else if(len==0){
		fprintf(stderr, "Connection closed\n");
		ev_io_stop(loop, watcher);
		free(watcher);
	}else{
		int times=0;
		for(int i=0;i<len;i++){
			if(buffer[i]=='\n'){
				times++;
			}
		}
		for(;times>0;times--){
			char *ptr=line_buffer;
			int remaining=line_buffer_length;
			do{
				ssize_t amount=send(watcher->fd, ptr, remaining, 0);
				if(amount<0){
					perror("send()");
					return;
				}
				remaining-=amount;
				ptr+=amount;
			}while(remaining>0);
		}
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

static void open_child(const char *cmd, pid_t *child_pid, int *pipe_fd){
	int p_stdout[2];
	if(pipe(p_stdout)!=0){
		perror("pipe()");
		abort();
	}
	pid_t pid=fork();
	if(pid<0){
		perror("fork()");
		close(p_stdout[0]);
		close(p_stdout[1]);
		abort();
	}else if(pid==0){
		close(p_stdout[0]);
		dup2(p_stdout[1], 1);
		execl("/bin/sh", "sh", "-c", cmd, NULL);
		//This should not return. If it does, it's in error.
		perror("execl");
		abort();
	}else{
		*child_pid=pid;
		close(p_stdout[1]);
		*pipe_fd=p_stdout[0];
	}
}
static void close_child(pid_t child_pid, int pipe_fd){
	kill(child_pid, SIGTERM);
	usleep(100000);//Wait 0.1 seconds for it to exit gracefully
	kill(child_pid, SIGKILL);
	close(pipe_fd);
}

static char pipe_buffer[8192];
static size_t pipe_buffer_length=0;

static void pipe_read_cb(struct ev_loop *loop, struct ev_io *watcher, int revents){
	ssize_t len=read(watcher->fd, &pipe_buffer[pipe_buffer_length], sizeof(pipe_buffer)-pipe_buffer_length);
	if(len<0){
		perror("pipe read()");
	}else{
		pipe_buffer_length+=len;
		int start=0;
		for(int i=0;i<pipe_buffer_length;i++){
			if(pipe_buffer[i]=='\n'){
				if(i!=start){
					//TODO: there's a buffer overflow here
					line_buffer_length=i-start;
					memcpy(line_buffer, &pipe_buffer[start], line_buffer_length);
				}
				start=i;
			}
		}
		memmove(pipe_buffer, &pipe_buffer[start], pipe_buffer_length-start);
		pipe_buffer_length-=start;
	}
}

int main(){
	pid_t child_pid;
	int pipe_fd;
	
	open_child(COMMAND, &child_pid, &pipe_fd);
	struct ev_loop *loop=EV_DEFAULT;
	
	struct ev_io w_pipe;
	ev_io_init(&w_pipe, pipe_read_cb, pipe_fd, EV_READ);
	ev_io_start(loop, &w_pipe);
	
	exit_cleanly_on(loop, SIGINT);
	exit_cleanly_on(loop, SIGTERM);
	
	struct sockaddr_in addr;
	int server_sock;
	if((server_sock=socket(PF_INET, SOCK_STREAM, 0)) < 0){
		perror("server_socket()");
		close_child(child_pid, pipe_fd);
		abort();
	}
	bzero(&addr, sizeof(addr));
	addr.sin_family=AF_INET;
	addr.sin_port=htons(PORT);
	addr.sin_addr.s_addr=INADDR_ANY;
	if(bind(server_sock, (void*)&addr, sizeof(addr))!=0){
		perror("bind()");
		close_child(child_pid, pipe_fd);
		abort();
	}
	if(listen(server_sock, 64) < 0){
		perror("listen()");
		close_child(child_pid, pipe_fd);
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
	close_child(child_pid, pipe_fd);
	fprintf(stderr, "...DONE\n");
	exit(0);
	return 0;
}
