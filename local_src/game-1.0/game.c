#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
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
#define RED (uint16_t)0xF800
#define GREEN (uint16_t)0x07E0
#define BLUE (uint16_t)0x001F
#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))
#define NUM_BALLS 5
#define WINNING_SCORE 20
#define SCORE_PANEL_HEIGHT 4
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define CONFETTI_NUM 20
#define PLAYER_SPEED 8
#define FRAME_LENGHT 33333 // The time for one frame. 33333 will be 30 fps.

FILE* gamepad;

typedef struct Player {
	Rect rect, prevRect;
	int score, prevScore;
	bool dirty;
} Player;

typedef struct Ball {
	Rect rect, prevRect;
	int dx, dy;
	bool ignorePlayer;
} Ball;


Player playerOne = {
	{15, 60, 10, 50}, {15, 60, 10, 50}, 0, 0, true
};

Player playerTwo = {
	{295, 60, 10, 50}, {295, 60, 10, 50}, 0, 0, true
};

Ball balls[NUM_BALLS];

bool playerOneUp = false;
bool playerOneDown = false;
bool playerTwoUp = false;
bool playerTwoDown = false;

bool gameFinished = false;
int confettiCounter;
Ball confetti[CONFETTI_NUM];
Ball confetti2[CONFETTI_NUM];
uint16_t confettiColor;


/**
	Returns a random number between -5 and 5, but not 0.
*/
int randomSpeed() {
	int speed = (rand () % 13) - 5;
  if (abs(speed) < 2) {
    return (rand() % 2) == 0 ? -2 : 2;
  }
  return speed;
}

/**
	Handler signal from the gamepad driver.
*/
void sigio_handler(int signo){
	int input = (uint8_t)~(fgetc(gamepad));

	if (input & 1<<1) { playerOneUp   = true;  }
	else {              playerOneUp   = false; }

	if (input & 1<<3) { playerOneDown = true;  }
	else {              playerOneDown = false; }

	if (input & 1<<5) { playerTwoUp   = true;  }
	else {              playerTwoUp   = false; }

	if (input & 1<<7) { playerTwoDown = true;  }
	else {              playerTwoDown = false; }
}

/**
	Load the gamepad driver.
*/
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


//---------------------//
// 		Reset state		   //
//---------------------//

/**
	Respawn a ball.
*/
void resetBall(Ball* ball) {
	ball->ignorePlayer = false;
	ball->rect.x = 155;
	ball->rect.y = 115;
	ball->dx = randomSpeed();
	ball->dy = randomSpeed();
}

/**
	Reset all gamestate.
*/
void resetGame(bool firstGame) {
	gameFinished = false;
	int i;
	for (i = 0; i < NUM_BALLS; i++) {
		Ball* ball = &balls[i];
		resetBall(ball);
		if (firstGame) {
			ball->rect.width = 10;
			ball->rect.height = 10;
			ball->prevRect.x = 155;
			ball->prevRect.y = 115;
			ball->prevRect.width = 10;
			ball->prevRect.height = 10;
		}
	}
	playerOne.score = 0;
	playerTwo.score = 0;
	playerOne.prevScore = 0;
	playerTwo.prevScore = 0;
	playerOne.dirty = true;
	playerTwo.dirty = true;
	clearScreen();

	Rect rect = {
		(SCREEN_WIDTH / 2) - 1,
		0,
		2,
		SCORE_PANEL_HEIGHT
	};

	drawRect(&rect, WHITE);
	refreshRect(&rect);
}


//---------------------//
// 		Confetti    	   //
//---------------------//


/**
	Create the confetti.
*/
void startConfetti() {
	clearScreen();
	confettiCounter = 150;
	int i;
	for (i = 0; i < CONFETTI_NUM; i++) {
		Ball* ball = &confetti[i];
		resetBall(ball);
		ball->rect.width = 5;
		ball->rect.height = 5;
		ball->prevRect.x = 155;
		ball->prevRect.y = 115;
		ball->prevRect.width = 5;
		ball->prevRect.height = 5;

		Ball* ball2 = &confetti2[i];
		resetBall(ball2);
		ball2->rect.x = 100;
		ball2->rect.y = 90;
		ball2->rect.width = 5;
		ball2->rect.height = 5;
		ball2->prevRect.x = 100;
		ball2->prevRect.y = 90;
		ball2->prevRect.width = 5;
		ball2->prevRect.height = 5;
	}
}

/**
	Update the positions of the confetti.
*/
void updateConfetti() {
	confettiCounter -= 1;
	if (confettiCounter <= 0) {
		resetGame(false);
	}
	int i;
	for (i = 0; i < CONFETTI_NUM; i++) {
		Ball *ball = &confetti[i];

		ball->prevRect.x = ball->rect.x;
		ball->prevRect.y = ball->rect.y;

		ball->rect.x += ball->dx;
		ball->rect.y += ball->dy;

		Ball *ball2 = &confetti2[i];

		ball2->prevRect.x = ball2->rect.x;
		ball2->prevRect.y = ball2->rect.y;

		ball2->rect.x += ball2->dx;
		ball2->rect.y += ball2->dy;
	}
}


//---------------------//
// 		Update players   //
//---------------------//


/**
	Move a player up.
*/
void moveUp(struct Player *player) {
	if (player->rect.y > PLAYER_SPEED - 1 + SCORE_PANEL_HEIGHT) {
		player->prevRect.y = player->rect.y;
		player->rect.y -= PLAYER_SPEED;
		player->dirty = true;
	}
}

/**
	Move a player down.
*/
void moveDown(struct Player *player) {
	if (player->rect.y + player->rect.height < SCREEN_HEIGHT - PLAYER_SPEED + 1) {
		player->prevRect.y = player->rect.y;
		player->rect.y += PLAYER_SPEED;
		player->dirty = true;
	}
}

/**
	Update the players state.
*/
void updatePlayers() {
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

	playerOne.prevScore = playerOne.score;
	playerTwo.prevScore = playerTwo.score;
}


//---------------------//
// 		Update balls	   //
//---------------------//

/**
	If the ball is in the upper half of the paddle, angle the ball upwards.
 	If it is in the lowe, angle the ball downwards.
*/
void angleBall(Ball* ball, Player* player){
	int diff = abs(ball->rect.y - player->rect.y);
	int half = player->rect.height / 2;
	if (abs(ball->dy - ball->dx) <= 3) {
		if (diff <= half) {
			ball->dy -= 1;
		} else {
			ball->dy += 1;
		}
	}
}

/**
	Iterates through all the balls and updates their state.
	Also checks if the balls collides with walls or the players.
	Increments players score if the ball goes off screen on one of the sides.
*/
void updateBalls(){
			int i;

			for (i = 0; i < NUM_BALLS; i++) {
				Ball *ball = &balls[i];

				// Set the prevRect = the current rect of the ball.
				ball->prevRect.x = ball->rect.x;
				ball->prevRect.y = ball->rect.y;

				// Apply the speed to the position of the ball
				ball->rect.x += ball->dx;
				ball->rect.y += ball->dy;

				// Check if the current or previous pos of the ball hits the player.
				if (ball->rect.x + ball->rect.width >= playerTwo.rect.x ||
						ball->prevRect.x + ball->prevRect.width >= playerTwo.rect.x) {
					//If it does, set the player dirty bit.
					playerTwo.dirty = true;
				}
				// Same, just with playerOne.
				if (ball->rect.x <= playerOne.rect.x + playerOne.rect.width ||
						ball->prevRect.x <= playerOne.rect.x + playerOne.rect.width) {
					playerOne.dirty = true;
				}


				// Check if the ball is to the right of playerTwo
				if (ball->rect.x + ball->rect.width >= playerTwo.rect.x + 3) {
					// Check if the ball is within the paddle of the player.
					if ((ball->rect.y + ball->rect.height) >= playerTwo.rect.y &&
					 		 ball->rect.y < (playerTwo.rect.y + playerTwo.rect.height) &&
							 !ball->ignorePlayer)
					{
						// Flip the x-acceleration
						ball->dx *= -1;
						angleBall(ball, &playerTwo);
					}
					else {
						// The ball is past the player, but no within the paddle.
						ball->ignorePlayer = true;
						if (ball->rect.x + ball->rect.width >= SCREEN_WIDTH) {
							// If the ball has moved out to the edge of screen,
							// reset the ball and give the other player a point.
							resetBall(ball);
							playerOne.score += 1;
						}
					}
				}

				// Check if the ball is to the left of playerOne
				if (ball->rect.x <= playerOne.rect.x + playerOne.rect.width - 3) {
					// Check if the ball is within the paddle of the player.
					if ((ball->rect.y + ball->rect.height) >= playerOne.rect.y
						&& ball->rect.y < (playerOne.rect.y + playerOne.rect.height)
						&& !ball->ignorePlayer)
					{
						// Flip the x-acceleration
						ball->dx *= -1;
						angleBall(ball, &playerOne);
					}
					else {
						// The ball is past the player, but no within the paddle.
						ball->ignorePlayer = true;
						// If the ball has moved out to the edge of screen,
						// reset the ball and give the other player a point.
						if (ball->rect.x <= 0) {
							resetBall(ball);
							playerTwo.score += 1;
						}
					}
				}
				// Check if the ball is touching the scorepanels
				if (ball->rect.y <= SCORE_PANEL_HEIGHT) {
					// Fix the position, and flip the y accel
					ball->rect.y = SCORE_PANEL_HEIGHT;
					ball->dy *= -1;
				}
				// Check if the ball is outside the bottom of the screen.
				if (ball->rect.y + ball->rect.height >= 240) {
					// Fix the position and flip the y accel.
					ball->rect.y = 240 - ball->rect.height;
					ball->dy *= -1;
				}
			}
}

/**
	Check if one of the players has won.
	Start the confetti if yes.
*/
void  checkForWinner() {
	if (playerOne.score >= WINNING_SCORE) {
		printf("Player One WINS\n");
		gameFinished = true;
		confettiColor = BLUE;
		startConfetti();
	}
	else if (playerTwo.score >= WINNING_SCORE) {
		printf("Player Two WINS\n");
		gameFinished = true;
		confettiColor = RED;
		startConfetti();
	}
}


/*
	The main update function called from the main gameloop.
*/
void update() {
	/* All state updating should happen here */
	if (!gameFinished) {
		updatePlayers();
		updateBalls();
		checkForWinner();
	}
	else {
		// If the game is finsished call the confetti update instead.
		updateConfetti();
	}
}

//---------------------//
// 			Drawing    	   //
//---------------------//

/**
	Calculates the rect to be refreshed based on a players current and previous rect.
*/
void refreshPlayer(Player* player) {
	Rect rRect = {
		player->rect.x,
		min(player->rect.y, player->prevRect.y),
		player->rect.width,
		player->rect.height + abs(player->rect.y - player->prevRect.y) + 1
	};
	refreshRect(&rRect);
}

/**
	Calculates the rect to be refreshed based on the ball's previous rect and current rect.
*/
void refreshBall(Ball* ball) {

	int x = min(ball->rect.x, ball->prevRect.x);
	int y = min(ball->rect.y, ball->prevRect.y);

	Rect rRect =  {
		x,
		y,
		max(ball->rect.x, ball->prevRect.x) + ball->rect.width - x,
		max(ball->rect.y, ball->prevRect.y) + ball->rect.height - y + 1,
	};
	refreshRect(&rRect);
}

/**
	Calculates the Rect of the Score of playerTwo, based on the players score.
*/
void createPlayerOneScoreRect(Rect* rect) {
	float widthPerScore = (float)(SCREEN_WIDTH / 2) / (float)WINNING_SCORE;
	rect->x = (int)(widthPerScore * playerOne.prevScore);
	rect->y = 0;
	rect->width = (playerOne.score - playerOne.prevScore) * widthPerScore;
	rect->height = SCORE_PANEL_HEIGHT;
}

/**
	Calculates the Rect of the Score of playerOne, based on the players score.
*/
void createPlayerTwoScoreRect(Rect* rect) {
	float widthPerScore = (float)(SCREEN_WIDTH / 2) / (float)WINNING_SCORE;
	rect->x = SCREEN_WIDTH - (int)(widthPerScore * playerTwo.score);
	rect->y = 0;
	rect->width = (playerTwo.score - playerTwo.prevScore) * widthPerScore;
	rect->height = SCORE_PANEL_HEIGHT;
}

/**
	Draw confetti
*/
void drawConfetti() {

	int i;
	// Clear the confettis previous rects.
	for (i = 0; i < CONFETTI_NUM; i++) {
		drawRect(&confetti[i].prevRect, BLACK);
		drawRect(&confetti2[i].prevRect, BLACK);
	}
	// Draw the current rects.
	for (i = 0; i < CONFETTI_NUM; i++) {
		drawRect(&confetti[i].rect, confettiColor);
		drawRect(&confetti2[i].rect, confettiColor);
	}

	// Refresh the screen where needed.
	for (i = 0; i < CONFETTI_NUM; i++) {
		refreshBall(&confetti[i]);
		refreshBall(&confetti2[i]);
	}
}

/**
	All drawing and refreshing happens here.
*/
void draw() {
	if (!gameFinished) {
		// Clear the players previous positions.
		drawRect(&playerOne.prevRect, BLACK);
		drawRect(&playerTwo.prevRect, BLACK);

		// Clear the previous positions of all balls.
		int i;
		for (i = 0; i < NUM_BALLS; i++) {
			drawRect(&balls[i].prevRect, BLACK);
		}

		// Draw the players, if they are dirty.
		if(playerOne.dirty) {
			drawRect(&playerOne.rect, BLUE);
		}
		if (playerTwo.dirty) {
			drawRect(&playerTwo.rect, RED);
		}

		// Draw all balls.
		for (i = 0; i < NUM_BALLS; i++) {
			drawRect(&balls[i].rect, WHITE);
		}

		Rect playerOneScoreRect;
		createPlayerOneScoreRect(&playerOneScoreRect);
		drawRect(&playerOneScoreRect, BLUE);

		Rect playerTwoScoreRect;
		createPlayerTwoScoreRect(&playerTwoScoreRect);
		drawRect(&playerTwoScoreRect, RED);

		// Refresh players, if they are dirty. Set the dirtybits to false when refreshed-
		if (playerOne.dirty) {
			refreshPlayer(&playerOne);
			playerOne.dirty = false;
		}
		if (playerTwo.dirty) {
			refreshPlayer(&playerTwo);
			playerTwo.dirty = false;
		}

		// Refresh balls
		for (i = 0; i < NUM_BALLS; i++) {
			refreshBall(&balls[i]);
		}

		refreshRect(&playerOneScoreRect);
		refreshRect(&playerTwoScoreRect);
	}
	else {
		// If the game is finished draw confetti!
		drawConfetti();
	}

}

int main(int argc, char *argv[])
{

	setupFramebuffer();
	setupGamepad();

	resetGame(true);

	while(1) {

		struct timeval tv;
		gettimeofday(&tv, NULL);
		unsigned long frameStartTime = (unsigned long)(tv.tv_sec) * 1000000 + (unsigned long)(tv.tv_usec);

		update();
		draw();

		gettimeofday(&tv, NULL);
		unsigned long frameEndTime = (unsigned long)(tv.tv_sec) * 1000000 + (unsigned long)(tv.tv_usec);
		unsigned long difference  = frameEndTime - frameStartTime;

		// Sleep the amount of time we have left of the frame.
		if (difference < FRAME_LENGHT) {
			usleep(FRAME_LENGHT - difference);
		}

	}
	exit(EXIT_SUCCESS);
}
