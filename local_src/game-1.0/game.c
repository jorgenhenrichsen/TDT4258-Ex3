#include <stdio.h>
#include <stdlib.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <signal.h>
#include <unistd.h>
#include <stdint.h>
#include <linux/fb.h>

FILE* gamepad;
FILE* fb;

void sigio_handler(int signo);
void getBinString(int value, char* output);
void setupFramebuffer();
void setupGamepad();

int main(int argc, char *argv[])
{
	printf("Hello World, I'm game!\n");

	setupFramebuffer();
	setupGamepad();

	while (1) {
		
	}

	exit(EXIT_SUCCESS);
}


void setupGamepad(){
	gamepad = fopen("dev/gamepad", "rb");

	if (!gamepad){
		printf("Unable to open gamepad driver.\n");
	}
	
	if (signal(SIGIO, &sigio_handler) == SIG_ERR) {
		printf("Error occured with the signal handler.\n");
	}
	
	if (fcntl(fileno(gamepad), F_SETOWN, getpid()) == -1) {
		printf("Error setting owner\n");
	}
	
	long oflags = fcntl(fileno(gamepad), F_GETFL);
	if (fcntl(fileno(gamepad), F_SETFL, oflags | FASYNC) == -1) {
		printf("Error setting FASYNC flag.\n");
	}
}

void setupFramebuffer(){

	fb = fopen("dev/fb0", O_RDWR);
	
	if (fb == -1){
		printf("Unable to open lcd driver.\n");
	}
}


void sigio_handler(int signo){
	printf("Received signal: %d\n", signo);
	int input = (uint8_t)~(fgetc(gamepad));
	char binary[9];
	getBinString(input, binary);
	
	int i;
	for (i = 7; i >= 0; i--) {
		if (binary[i] == '1'){
			int button = 7 - i;
			printf("Button%d pressed \n", button);
		}
	}
	
} 

void getBinString(int value, char* output)
{
    int i;
    output[8] = '\0';
    for (i = 7; i >= 0; --i, value >>= 1)
    {
        output[i] = (value & 1) + '0';
    }
}






