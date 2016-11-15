#include <stdint.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <unistd.h>

typedef struct Rect {
	int x, y, width, height;
} Rect;

void setupFramebuffer();
void drawRect(Rect rect, uint16_t color);
void refreshRect(Rect rect);
void clearScreen();
