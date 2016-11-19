
#include "draw.h"

#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))

int fb;
char * fbmap;
struct fb_var_screeninfo screenInfo;
struct fb_fix_screeninfo fScreenInfo;
struct fb_copyarea rRect; // The rect being refreshed.

void drawPixelAt(int x, int y, uint16_t color);

void setupFramebuffer(){
	printf("Opening framebufferdriver\n");
	fb = open("/dev/fb0", O_RDWR);

	if (fb == -1){
		printf("Unable to open lcd driver.\n");
	}

	// Get variable screen information
  if (ioctl(fb, FBIOGET_VSCREENINFO, &screenInfo) == -1) {
      printf("Error reading variable information \n");
  }

	// Get fixed screen information
  if (ioctl(fb, FBIOGET_FSCREENINFO, &fScreenInfo) == -1) {
      printf("Error reading fixed information \n");
  }

	// Map the framebuffer to memory
	long int screensize = screenInfo.xres * screenInfo.yres * (screenInfo.bits_per_pixel / 8);
	fbmap = (char*)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0);
	if (fbmap == MAP_FAILED){
		printf("Failed to map the framebuffer\n");
	}

	clearScreen();
}

void clearScreen() {
	/*
		Draw the screen black.
	*/
	Rect rect = {
		0,0, screenInfo.xres, screenInfo.yres
	};
	drawRect(&rect, 0x0000);
	refreshRect(&rect);
}

void drawPixelAt(int x,int y, uint16_t color){
	/*
		Sets a single pixel to the specified color.
	*/
  long int location = (x*2) + y*640;
	fbmap[location] = (uint8_t)color;
	fbmap[location+1] = (uint8_t)(color>>8);
}



void drawRect(Rect* rect, uint16_t color) {
	/*
		Draws a rectangle on the screen in the specified color
		Does not refresh the screen, only sets the values in the framebuffer.
	*/

	// Clamp the given rect to the screens bounds.
	int xStart = min(max(0, rect->x), screenInfo.xres);
	int yStart = min(max(0, rect->y), screenInfo.yres);
	int xEnd   = min(max(0, rect->x + rect->width ), screenInfo.xres);
	int yEnd   = min(max(0, rect->y + rect->height), screenInfo.yres);

	int x, y;
	for (x = xStart; x < xEnd; x++) {
		for (y = yStart; y < yEnd; y++) {
			drawPixelAt(x, y, color);
		}
	}
}

void refreshRect(Rect* rect) {
	/*
		Refresh a rectangle of the screen
		Will clamp the rect so it only refreshes the parts that are inside the screen bounds.
	*/
	int xEnd   = min(max(0, rect->x + rect->width ), screenInfo.xres);
	int yEnd   = min(max(0, rect->y + rect->height), screenInfo.yres);

	rRect.dx = min(max(0, rect->x), screenInfo.xres);
	rRect.dy = min(max(0, rect->y), screenInfo.yres);
	rRect.width = xEnd - rRect.dx;
	rRect.height = yEnd - rRect.dy;
	ioctl(fb, 0x4680, &rRect);
}
