#ifndef METAL_H
#define METAL_H

void metal_init(void);
int metal_create_pipeline(const char* shader_source);
void metal_cleanup(void);

#endif