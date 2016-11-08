
#include "draw.h"

int fb;
char * fbmap;
struct fb_var_screeninfo screenInfo;
struct fb_fix_screeninfo fScreenInfo;
struct fb_copyarea rect; // The rect being drawn.

void refreshRect(int x, int y, int width, int height);

void setupFramebuffer(){
	printf("Opening framebufferdriver\n");
	fb = open("/dev/fb0", O_RDWR);

	if (fb == -1){
		printf("Unable to open lcd driver.\n");
	}
	printf("%d\n", fb);

	// Get variable screen information
  if (ioctl(fb, FBIOGET_VSCREENINFO, &screenInfo) == -1) {
      printf("Error reading variable information \n");
  }

	// Get fixed screen information
  if (ioctl(fb, FBIOGET_FSCREENINFO, &fScreenInfo) == -1) {
      printf("Error reading fixed information \n");
  }

	long int screensize = screenInfo.xres * screenInfo.yres * (screenInfo.bits_per_pixel / 8);
	printf("Screensize %ld\n", screensize);

	fbmap = (char*)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0);
	if ((int)fbmap == MAP_FAILED){
		printf("Failed to map the framebuffer\n");
	}

	drawRect(0, 0, screenInfo.xres, screenInfo.yres, rgb(255,255,255));
  drawCircle(50, 50, 10, rgb(0,0,0));
}

void drawPixeltAt(int x,int y,char * color){
  long int location = (x*2) + y*640;
	//long int location = (x * screenInfo.bits_per_pixel/8) + (y*fScreenInfo.line_length);
	fbmap[location] = color[0];
	fbmap[location+1] = color[1];
}

void drawRect(int x,int y,int width,int height, char * color) {
  printf("Drawing rect : x=%d, y=%d, width = %d, height%d\n", x, y, width, height);
	int dx, dy;

	for (dx = x; dx < x + width; dx++) {
		for (dy = y; dy < y + height; dy++) {
      //printf("Pixel: x=%d, y=%d\n", dx, dy);
			drawPixeltAt(dx, dy, color);
		}
	}
  refreshRect(x, y, width, height);
}

void refreshRect(int x, int y, int width, int height) {
  rect.dx = x;
	rect.dy = y;
	rect.width = width;
	rect.height = height;
	ioctl(fb, 0x4680, &rect);
}

// converts rgb to byte
char * rgb(int red, int green, int blue) {
	short int colour16 = (short)(((blue&0xf8)<<8) +
	    ((red&0xfc)<<3) + ((green&0xf8)>>3));

	char * arr = malloc(sizeof(char)*2);
	arr[0] = ((colour16 >> 8) & 0xff);
	arr[1] = ((colour16 >> 0) & 0xff);

	return arr;
}
