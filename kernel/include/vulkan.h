#ifndef VULKAN_H
#define VULKAN_H

#include <stdint.h>

void vulkan_init(void);
int vulkan_create_surface(uint32_t width, uint32_t height);
void vulkan_cleanup(void);

#endif