
#include "draw.h"

#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))

int fb;
char * fbmap;
struct fb_var_screeninfo screenInfo;
struct fb_fix_screeninfo fScreenInfo;
struct fb_copyarea rRect; // The rect being drawn.

void drawPixelAt(int x, int y, uint16_t color);

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

	clearScreen();
}

void clearScreen() {
	Rect rect = {
		0,0, screenInfo.xres, screenInfo.yres
	};

	drawRect(rect, 0x0000);
	refreshRect(rect);
}

void drawPixeltAt(int x,int y, uint16_t color){
  long int location = (x*2) + y*640;
	//long int location = (x * screenInfo.bits_per_pixel/8) + (y*fScreenInfo.line_length);
	fbmap[location] = (uint8_t)color;
	fbmap[location+1] = (uint8_t)(color>>8);
}



void drawRect(Rect rect, uint16_t color) {
  //printf("Drawing rect : x=%d, y=%d, width = %d, height%d\n", x, y, width, height);
	uint16_t dx, dy;
	int maxX = rect.x + rect.width;//min((x + width), screenInfo.xres);
	int maxY =rect.y + rect.height;//min((y + height), screenInfo.yres);
	for (dx = rect.x; dx < maxX; dx++) {
		for (dy = rect.y; dy < maxY; dy++) {
      //printf("Pixel: x=%d, y=%d\n", dx, dy);
			drawPixeltAt(dx, dy, color);
		}
	}
}

void refreshRect(Rect rect) {
  rRect.dx = min((rect.x + rect.width), screenInfo.xres) - rect.width;
	rRect.dy = min((rect.y + rect.height), screenInfo.yres) - rect.height;
	rRect.width = rect.width;
	rRect.height = rect.height;
	ioctl(fb, 0x4680, &rRect);
}
