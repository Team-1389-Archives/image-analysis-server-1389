#ifndef FILTERING_H
#define FILTERING_H
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"{
#endif

typedef struct FilteringSystem* filtering_system_t;

filtering_system_t FilteringSystemNew(void);
void FilteringSystemFilter(filtering_system_t, int width, int height, uint8_t* in, uint8_t *out);
void FilteringSystemClose(filtering_system_t); 

#ifdef __cplusplus
}
#endif
#endif
