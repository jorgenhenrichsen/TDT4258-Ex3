#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <signal.h>
#include <unistd.h>
#include <stdint.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <time.h>
#include "draw.h"


#define WHITE (uint16_t)0xFFFF
#define BLACK (uint16_t)0x0000
#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))


FILE* gamepad;


typedef struct Player {
	Rect rect, prevRect;
	/*uint16_t x, y, width, height;
	int prevY;*/
} Player;

typedef struct Ball {
	Rect rect, prevRect;
	int dx, dy;
	/*int x, y, dx, dy, size;
	int prevX, prevY;*/
	bool ignorePlayer;
} Ball;

void sigio_handler(int signo);
void getBinString(int value, char* output);
void setupGamepad();
void clearPlayer(Player player);
void drawPlayer(Player player);
void refreshPlayer(Player player);
void loop();
void update();
void draw();
void timerHandler(int signo);
void moveUp(Player *player);
void moveDown(Player *player);
void clearBall(Ball ball);
void drawBall(Ball ball);
void refreshBall(Ball ball);

Player playerOne = {
	{15, 60, 10, 50}, {15, 60, 10, 50}
	/*15, 60, 10, 50, 60*/
};

Player playerTwo = {
	{295, 60, 10, 50}, {295, 60, 10, 50}
	/*295, 60, 10, 50, 60*/
};

Ball ball = {
	{155, 115, 10, 10}, {155, 115, 10, 10}, 4, 6, false
	/*155, 115, 2, 3, 10, 0, 0, false*/
};


bool playerOneUp = false;
bool playerOneDown = false;
bool playerTwoUp = false;
bool playerTwoDown = false;


int main(int argc, char *argv[])
{
	/*
	TODO:
	Dirtybit player
	Pointers in functions
	tweak framerate

	*/
	printf("Hello World, I'm game!\n");

	setupFramebuffer();
	setupGamepad();

	struct sigaction sa;
	struct itimerval timer;

	memset(&sa, 0, sizeof(sa));

	sa.sa_handler = &timerHandler;
	sigaction( SIGALRM, &sa , NULL);

	timer.it_value.tv_sec = 0;
	timer.it_value.tv_usec = 33333;
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = 33333;

	setitimer(ITIMER_REAL, &timer, NULL);

	//int msec = 0; trigger = 10;

	while(1);

	exit(EXIT_SUCCESS);
}

void timerHandler(int signo) {
	loop();
}

void loop() {
	update();
	draw();
}

void update() {
	/* All state updating should happen here */
	if (playerOneUp) {
		moveUp(&playerOne);
	}
	if (playerOneDown) {
		moveDown(&playerOne);
	}
	if (playerTwoUp) {
		moveUp(&playerTwo);
	}
	if (playerTwoDown) {
		moveDown(&playerTwo);
	}

	ball.prevRect.x = ball.rect.x;
	ball.prevRect.y = ball.rect.y;

	ball.rect.x += ball.dx;
	ball.rect.y += ball.dy;

	if (ball.rect.x + ball.rect.width >= playerTwo.rect.x) {
		if ((ball.rect.y + ball.rect.height) >= playerTwo.rect.y && ball.rect.y < (playerTwo.rect.y + playerTwo.rect.height) && !ball.ignorePlayer) {
			ball.dx *= -1;
		}
		else {
			ball.ignorePlayer = true;
			if (ball.rect.x + ball.rect.width >= 320) {
				ball.ignorePlayer = false;
				ball.rect.x = 155;
				ball.rect.y = 115;
			}
		}
	}
	if (ball.rect.x <= playerOne.rect.x + playerOne.rect.width) {
		if ((ball.rect.y + ball.rect.height) >= playerOne.rect.y && ball.rect.y < (playerOne.rect.y + playerOne.rect.height) && !ball.ignorePlayer) {
			ball.dx *= -1;
		}
		else {
			ball.ignorePlayer = true;
			if (ball.rect.x <= 0) {
				ball.ignorePlayer = false;
				ball.rect.x = 155;
				ball.rect.y = 115;
			}
		}
	}
	if (ball.rect.y <= 0) {
		ball.rect.y = 0;
		ball.dy *= -1;
	}
	if (ball.rect.y + ball.rect.height >= 240) {
		ball.rect.y = 240 - ball.rect.height;
		ball.dy *= -1;
	}



}

void draw() {
	/* All drawing should happen here */

	clearPlayer(playerOne);
	clearPlayer(playerTwo);
	clearBall(ball);

	drawPlayer(playerOne);
	drawPlayer(playerTwo);
	drawBall(ball);

	refreshPlayer(playerOne);
	refreshPlayer(playerTwo);
	refreshBall(ball);

	/*drawPlayer(playerOne);
	drawPlayer(playerTwo);
	drawBall();*/
}

void drawPlayer(Player player) {
	//drawRect(player.x, player.y, player.width, player.height, WHITE);
	drawRect(player.rect, WHITE);
}

void clearPlayer(Player player) {
	drawRect(player.prevRect, BLACK);
	//drawRect(player.x, 0, player.width, player.y, BLACK);
	//drawRect(player.x, player.y + player.height, player.width, 240 - (player.y + player.height), BLACK);
	//drawRect(player.x, player.prevY, player.width, player.height, BLACK);
}

void refreshPlayer(Player player) {
	//refreshRect(player.rect);
	Rect rRect = {
		player.rect.x, min(player.rect.y, player.prevRect.y), player.rect.width, player.rect.height + abs(player.rect.y - player.prevRect.y) + 1
	};
	refreshRect(rRect);
	//refreshRect(player.x, min(player.y, player.prevY), player.width, player.height + abs(player.y - player.prevY));
}

void drawBall(Ball ball) {
	drawRect(ball.rect, WHITE);
	//drawRect(ball.x, ball.y, ball.size, ball.size, WHITE);
}

void clearBall(Ball ball) {
	drawRect(ball.prevRect, BLACK);
	//drawRect(ball.prevX, ball.prevY, ball.size, ball.size, BLACK);
}

void refreshBall(Ball ball) {

	int x = min(ball.rect.x, ball.prevRect.x);
	int y = min(ball.rect.y, ball.prevRect.y);

	Rect rRect =  {
		x,
		y,
		max(ball.rect.x, ball.prevRect.x) + ball.rect.width - x,
		max(ball.rect.y, ball.prevRect.y) + ball.rect.height - y + 1,
	};
	refreshRect(rRect);
}

void moveUp(struct Player *player) {
	if (player->rect.y > 5) {
		player->prevRect.y = player->rect.y;
		player->rect.y -= 6;
	}
}

void moveDown(struct Player *player) {
	if (player->rect.y + player->rect.height < 235) {
		player->prevRect.y = player->rect.y;
		player->rect.y += 6;
	}

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


void sigio_handler(int signo){
	printf("Received signal: %d\n", signo);
	int input = (uint8_t)~(fgetc(gamepad));
	char binary[9];
	getBinString(input, binary);

	int i;
	for (i = 7; i >= 0; i--) {
		int button = 7 - i;
		if (binary[i] == '1'){
			//printf("Button%d pressed \n", button);
			switch (button) {
				case 1: playerOneUp = true; break;
				case 3: playerOneDown = true; break;
				case 5: playerTwoUp = true; break;
				case 7: playerTwoDown = true; break;
				default: break;
			}
		}
		else {
			switch (button) {
				case 1: playerOneUp = false; break;
				case 3: playerOneDown = false; break;
				case 5: playerTwoUp = false; break;
				case 7: playerTwoDown = false; break;
				default: break;
			}
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
