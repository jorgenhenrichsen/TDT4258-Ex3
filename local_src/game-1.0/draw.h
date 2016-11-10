#include <stdint.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <unistd.h>


void setupFramebuffer();
void drawRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);
void refreshRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
