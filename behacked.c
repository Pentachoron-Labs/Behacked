#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <ncurses.h>

#define	WIDTH	10
#define HEIGHT	10
#define DEBUG	1

static int grid[HEIGHT][WIDTH] = {0};

static int _ncomb(int x, int y, int color){
    int n;
    
    if(grid[y][x] != color) return 0;
    
    grid[y][x] = 0;
    n = 1;
    if(y-1 >= 0)     n += _ncomb(x,y-1,color);
    if(y+1 < HEIGHT) n += _ncomb(x,y+1,color);
    if(x-1 >= 0)     n += _ncomb(x-1,y,color);
    if(x+1 < WIDTH)  n += _ncomb(x+1,y,color);
    return n;
}

/**
 * Returns the number of similar gems connected to the specified one,
 * including said gem.
 */
int ncomb(int x, int y){
    int color = grid[y][x];
    int i,ret;
    
    ret = _ncomb(x,y,color);
    for(i=0;i<WIDTH*HEIGHT;i++){
	if(grid[i/WIDTH][i%WIDTH] == 0) grid[i/WIDTH][i%WIDTH] = color;
    }
    return ret;
}

/**
 * Sleeps for a given number of milliseconds
 */
void msleep(int n){
    usleep(n*1000);
}

/**
 * Just redraws the grid
 */
void redraw(void){
    int i;
    for(i=0;i<WIDTH*HEIGHT;i++){
	int r = grid[i/WIDTH][i%WIDTH];
	if(r != 0){
	    attron(COLOR_PAIR(r));
	    mvaddch(i/WIDTH,i%WIDTH,'*');
	    attroff(COLOR_PAIR(r));
	}
    }
    refresh();
}

/**
 * Also redraws the current score
 */
void redrawall(int score){
    clear();
    redraw();
    mvprintw(4,WIDTH+5,"Score: %d",score);
    refresh();
}

void breakcombs(int tobreak[HEIGHT][WIDTH]){
    int k;
    for(k=0;k<WIDTH*HEIGHT;k++){
	if(tobreak[k/WIDTH][k%WIDTH]){
	    attron(COLOR_PAIR(grid[k/WIDTH][k%WIDTH]));
	    mvaddch(k/WIDTH,k%WIDTH,'o');
	}
    }
    refresh();
    msleep(150); /*is a nice number*/
    for(k=0;k<WIDTH*HEIGHT;k++){
	if(tobreak[k/WIDTH][k%WIDTH]){
	    attron(COLOR_PAIR(grid[k/WIDTH][k%WIDTH]));
	    mvaddch(k/WIDTH,k%WIDTH,'O');
	}
    }
    refresh();
    msleep(150);
    for(k=0;k<WIDTH*HEIGHT;k++){
	if(tobreak[k/WIDTH][k%WIDTH]){
	    attron(COLOR_PAIR(grid[k/WIDTH][k%WIDTH]));
	    mvaddch(k/WIDTH,k%WIDTH,' ');
	    grid[k/WIDTH][k%WIDTH] = 0;
	}
    }
    refresh();
    msleep(150);
}

/**
 * Generates a random integer in the interval (0,n]
 */
int randn(int n){
    int r;
    while((r = random()) >= (RAND_MAX/n*n));
    return 1+(r%n);
}

void refill(void){
    int hasblank;
    int x,y;
    do{
	hasblank = 0;
	for(x=0;x<WIDTH;x++){
	    for(y=HEIGHT-1;y>=1;y--){
		if(grid[y-1][x] != 0 && grid[y][x] == 0){
		    grid[y][x] = grid[y-1][x];
		    grid[y-1][x] = 0;
		    hasblank = 1;
		}
	    }
	    if(grid[0][x] == 0){
		grid[0][x] = randn(6);
		hasblank = 1;
	    }
	}
	redraw();
	msleep(150);
    }while(hasblank);
}

/**
 * Removes combinations (groups of four) in the grid and refills.
 * Returns the number of points to be added.
 */
int rmcombs(void){
    int tobreak[HEIGHT][WIDTH] = {0};
    int score=0;
    int x,y;
    for(y=0;y<HEIGHT;y++){
	for(x=0;x<WIDTH;x++){
	    if(ncomb(x,y) >= 4){
		tobreak[y][x] = 1;
		score += 5; /*per gem*/
	    }
	}
    }
    breakcombs(tobreak);
    refill();
    return score;
}

/**
 * Returns the number of potential combinations.
 */
int ncombs(void){
    int x,y;
    int temp;
    int n;
    
    /*test vertical swaps first...*/
    for(x=0;x<WIDTH;x++){
	for(y=0;y<HEIGHT-1;y++){
	    /*swap (x,y) and (x,y+1)*/
	    temp = grid[y][x];
	    grid[y][x] = grid[y+1][x];
	    grid[y+1][x] = temp;
	    /*count the combinations*/
	    n += (ncomb(x,y) >= 4);
	    n += (ncomb(x,y+1) >= 4);
	    /*swap back*/
	    grid[y+1][x] = grid[y][x];
	    grid[y][x] = temp;
	}
    }
    /*...and then horizontal*/
    for(x=0;x<WIDTH-1;x++){
	for(y=0;y<HEIGHT;y++){
	    /*swap (x,y) and (x+1,y)*/
	    temp = grid[y][x];
	    grid[y][x] = grid[y][x+1];
	    grid[y][x+1] = temp;
	    /*count the combinations*/
	    n += (ncomb(x,y) >= 4);
	    n += (ncomb(x+1,y) >= 4);
	    /*swap back*/
	    grid[y][x+1] = grid[y][x];
	    grid[y][x] = temp;
	}
    }
    return n;
}

int main(int argc, char **argv){
    int score = 0;
    int cx,cy; /*cursor x,y*/
    int sx,sy; /*selected x,y*/
    int i;
    int c;
    
    srandom(time(NULL)); /*seed random*/
    initscr(); /*initialize ncurses*/
    cbreak(); /*suspend line buffering*/
    noecho(); /*@echo off*/
    keypad(stdscr,TRUE); /*enable arrows*/
    start_color(); /*enable color*/
    for(i=1;i<=6;i++) init_pair(i,i,COLOR_BLACK);
    
    mvprintw(4,7,"Behacked");
    refresh();
    msleep(2000);
    clear();

    refill();
    redrawall(score);
    
    move(5,5);
    refresh();
    while((i = rmcombs())){
	score += i;
	redrawall(score);
    }
    cx = cy = 5;
    sx = sy = -1;
    while((c = getch()) != 'q'){
	switch(c){
	case KEY_LEFT:
	case 'h':
	    if(cx > 0) cx--;
	    break;
	case KEY_RIGHT:
	case 'l':
	    if(cx < WIDTH-1) cx++;
	    break;
	case KEY_UP:
	case 'k':
	    if(cy > 0) cy--;
	    break;
	case KEY_DOWN:
	case 'j':
	    if(cy < HEIGHT-1) cy++;
	    break;
#ifdef DEBUG
	case 'n':
	    redraw();
	    mvprintw(HEIGHT+4,2,"(%d,%d) %d*%d",cx,cy,ncomb(cx,cy),grid[cy][cx]);
	    refresh();
	    break;
	case 'N':
	    redraw();
	    mvprintw(2,WIDTH+5,"%d",ncombs());
	    refresh();
	    break;
	case 'r':
	    redrawall(score);
	    break;
#endif
	case ' ':
	    if(sx == -1){
		sx = cx;
		sy = cy;
		attron(COLOR_PAIR(grid[cy][cx]));
		mvaddch(cy,cx,'*'|A_BOLD);
		attroff(COLOR_PAIR(grid[cy][cx]));
	    }else{
		redraw(); /*gets rid of bold*/
		if((sx-cx == 1 || sx-cx == -1) ^ (sy-cy == 1 || sy-cy == -1)){
		    int r = grid[sy][sx];
		    grid[sy][sx] = grid[cy][cx];
		    grid[cy][cx] = r;
		    redrawall(score);
		    msleep(150);
		    if((i = rmcombs())){
			do{
			    score += i;
			    redrawall(score);
			}while((i = rmcombs()));
			
			if(ncombs() == 0){
			    mvprintw(4,(WIDTH/2)-5,"GAME OVER");
			    getch();
			    endwin();
			    exit(0);
			}
		    }else{
			r = grid[sy][sx];
			grid[sy][sx] = grid[cy][cx];
			grid[cy][cx] = r;
			redraw();
		    }
		}
		sy = sx = -1;
	    }
	    break;
#ifdef DEBUG
	default:
	    mvprintw(HEIGHT+2,5,"%d",c);
#endif
	}
	move(cy,cx);
	refresh();
    }
    endwin();
    return 0;
}
