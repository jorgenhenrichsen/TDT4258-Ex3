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
void drawRect(int x, int y, int width, int height, char * color);
char * rgb(int red, int green, int blue);
