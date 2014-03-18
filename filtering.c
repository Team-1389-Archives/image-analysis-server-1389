#include <pthread.h>
#include <ck_md.h>
#include <ck_spinlock.h>
#include <ck_ring.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <semaphore.h>
#include <assert.h>
#include "filtering.h"

#define NUM_CORES		(4)

enum{
    OPERATION_THRESHOLD,
    OPERATION_EDGE_DETECT
};

struct request{
	uint32_t num_finished;
	int num_pixels;
	uint8_t *data;
	uint8_t *out;
	int width, height;
	uint8_t operation;
	sem_t notify;
};

#define QUEUE_LENGTH		(4)//Power of 2 >=4

struct Thread{
	pthread_t        thread;
	int              idx;
	ck_spinlock_t    producer_lock;
	ck_ring_t        queue;
	ck_ring_buffer_t queue_buffer[QUEUE_LENGTH];
	sem_t            work_ready;
} __attribute__ ((aligned (CK_MD_CACHELINE)));

struct FilteringSystem{
	struct Thread threads[NUM_CORES];
};

struct __attribute__ ((__packed__)) pixel{
	uint8_t r, g, b;
};

#define BATCH_SIZE		(CK_MD_CACHELINE*3)
#define INT16(x)		((int16_t)(x))

#define SATURATION_MIN		(250)
#define TARGET_HUE			(1000)
#define MAX_HUE_VARIANCE	(1000)

#define MIN_HUE				(TARGET_HUE-MAX_HUE_VARIANCE)
#define MAX_HUE				(TARGET_HUE+MAX_HUE_VARIANCE)

static inline bool threshold(struct pixel pix){
	uint8_t max, min;
	max=((pix.r > pix.g) && (pix.r > pix.b))?pix.r:(
		((pix.g > pix.r) && (pix.g > pix.b)) ? 
			pix.g : pix.b
	);
	min=((pix.r < pix.g) && (pix.r < pix.b))?pix.r:(
		((pix.g < pix.r) && (pix.g < pix.b)) ? 
			pix.g : pix.b
	);
	if(max==0){
		return false;
	}
	uint8_t delta=max-min;
	uint16_t saturation=(delta*1000)/max;
	if(saturation<SATURATION_MIN){
		return false;
	}
	int16_t hue;
#define PARTIAL(a, b)	((600)*(INT16(a)-INT16(b)))/INT16(delta)
	if(pix.r==max){
		hue=PARTIAL(pix.g, pix.b);
	}else if(pix.g==max){
		hue=(600)*2+PARTIAL(pix.b, pix.r);
	}else{
		hue=(600)*4+PARTIAL(pix.r, pix.g);
	}
	if(hue<0){
		hue+=INT16_MAX;
	}
	return (hue>=MIN_HUE && hue<=MAX_HUE);
}

static inline void threshold_operation(struct pixel *pix, int i, struct request *req){
	if(threshold(*pix)){
	    pix->g=255;
	}else{
		pix->g=0;
	}
}

static inline bool edge_detect_predicate(struct pixel *pix, int i, struct request *req){
    int x=i%req->width;
    int y=i/req->width;
#define image(x,y,chan)     (req->data[(((x)+(y)*(req->width))*3)+1])
    bool isEdge=false;
    if (image(x,y,0) > 127){///********** > 127 because after blur this is area we want;
                if (x != 0){
                    if (image(x-1,y,0) <= 127)//means its black pixel Why do black pixels automatically mean an edge?
                        isEdge = true;
                }
                if (x != req->width - 1){
                    if (image(x+1,y,0) <= 127)
                        isEdge = true;
                }
                if (y != 0){
                    if (image(x,y-1,0) <= 127)
                        isEdge = true;
                }
                if (y != req->height - 1){
                    if (image(x,y+1,0) <= 127)
                        isEdge = true;
                }
            }
            return isEdge;
#undef image
}

static inline void edge_detect_operation(struct pixel *pix, int i, struct request *req){
    req->out[i]=edge_detect_predicate(pix, i, req)?255:0;
}

#define PER_PIXEL_OPERATION(func)   for(int idx=thread->idx;true;idx+=NUM_CORES){   \
			int end=(idx+1)*BATCH_SIZE; \
			int i;  \
			for(i=idx*BATCH_SIZE;i<end && i<req->num_pixels; i++){  \
				struct pixel *pix=(struct pixel*)&req->data[i*3];   \
				func(pix, i, req);   \
			}   \
			if(i>=req->num_pixels){ \
				break;  \
			}   \
		}

static void* working_thread(void* arg){
	struct Thread *thread=arg;
	while(true){
		struct request *req;
		sem_wait(&thread->work_ready);
		while(!ck_ring_dequeue_spsc(&thread->queue, thread->queue_buffer, &req)){
			sched_yield();
		}
		if(req->operation==OPERATION_THRESHOLD){
		    PER_PIXEL_OPERATION(threshold_operation);
		}else if(req->operation==OPERATION_EDGE_DETECT){
		    PER_PIXEL_OPERATION(edge_detect_operation);
		}
		sem_post(&req->notify);
	}
	pthread_exit(NULL);
	return NULL;
}

filtering_system_t FilteringSystemNew(){
    assert(sizeof(struct pixel)==3*sizeof(uint8_t));
    
	filtering_system_t sys=malloc(sizeof(struct FilteringSystem));
	for(int i=0;i<NUM_CORES;i++){
		struct Thread *thread=&sys->threads[i];
		thread->idx=i;
		ck_ring_init(&thread->queue, QUEUE_LENGTH);
		ck_spinlock_init(&thread->producer_lock);
		sem_init(&thread->work_ready, 0, 0);
		pthread_create(&thread->thread, NULL, working_thread, thread);
	}
	return sys;
}
#define EXECUTE_REQUEST()   do{ \
    for(int i=0;i<NUM_CORES;i++){   \
		struct Thread *thread=&sys->threads[i]; \
		ck_spinlock_lock(&thread->producer_lock);   \
		ck_ring_enqueue_spsc(&thread->queue, thread->queue_buffer, &req);   \
		ck_spinlock_unlock(&thread->producer_lock); \
		sem_post(&thread->work_ready);  \
	}   \
	for(int i=0;i<NUM_CORES;i++){   \
	    sem_wait(&req.notify);  \
	}   \
}while(0)
void FilteringSystemFilter(filtering_system_t sys, int width, int height, uint8_t* img, uint8_t *out){
	struct request req={
		.num_pixels=width*height,
		.data=img,
		.out=out,
		.operation=OPERATION_THRESHOLD,
		.width=width,
		.height=height
	};
	sem_init(&req.notify, 0, 0);
	EXECUTE_REQUEST();
	req.operation=OPERATION_EDGE_DETECT;
	EXECUTE_REQUEST();
	sem_destroy(&req.notify);
}
void FilteringSystemClose(filtering_system_t sys){
	for(int i=0;i<NUM_CORES;i++){
		struct Thread *thread=&sys->threads[i];
		pthread_cancel(thread->thread);
		sem_destroy(&thread->work_ready);
	}
	free(sys);
}

//NOTE the g is displaying as red, and the r is displaying as green
