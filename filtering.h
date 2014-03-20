#ifndef FILTERING_H
#define FILTERING_H
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"{
#endif

extern uint16_t g_min_s;
extern uint16_t g_hue;
extern uint16_t g_hue_variance;

typedef uint16_t edge_number_t;

typedef struct FilteringSystem* filtering_system_t;

filtering_system_t FilteringSystemNew(void);
void FilteringSystemFilter(filtering_system_t, int width, int height, uint8_t* in, edge_number_t *out);
void FilteringSystemClose(filtering_system_t); 

#ifdef __cplusplus
}
#endif
#endif
