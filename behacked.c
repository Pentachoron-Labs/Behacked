#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "behacked.h"

int main(int argc, char **argv){
	int i,n;
	srandom(time(NULL));
	initscr(); //initialize ncurses
	cbreak(); //suspend line buffering
	noecho(); //@echo off
	keypad(stdscr,TRUE); //enable arrows
	start_color(); //enable color
	for(i=1;i<=6;i++) init_pair(i,i,COLOR_BLACK);
	mvprintw(4,7,"Behacked");
	refresh();
	getch();
	clear();
	fill();
	move(5,5);
	refresh();
	while(rmcombs());
	cx = 5; cy = 5;
	while((c = getch()) != 3){
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
			case 'n':
				mvprintw(14,2,"          ");
				n = 0;
				clpendsh();
				ncomb(cx,cy,&n);
				mvprintw(14,2,"(%d,%d) %d*%d",cx,cy,n,grid[cy][cx]);
				refresh();
			default:
				mvprintw(12,5,"%d",c);
		}
		move(cy,cx);
		refresh();
	}
	endwin();
	return 0;
}

/**Generates a random integer in the interval [0,n)*/
int randn(int n){
	int r;
	while((r = random()) >= (RAND_MAX/n*n));
	return (r%n);
}

/**Sleeps for a given number of milliseconds*/
void msleep(int n){
	nanosleep((struct timespec[]){{0,n*1000000}},NULL);
}

/**Fills the grid with random gems*/
void fill(){
	int i,r;
	for(i=0;i<WIDTH*HEIGHT;i++){
		r = randn(6)+1;
		grid[i/WIDTH][i%WIDTH] = r;
		attron(COLOR_PAIR(r));
		mvaddch(i/WIDTH,i%WIDTH,'*');//0x30+r);
		attroff(COLOR_PAIR(r));
	}
}

void redraw(){
	int i;
	clear();
	for(i=0;i<WIDTH*HEIGHT;i++){
		int r = grid[i/WIDTH][i%WIDTH];
		if(r != 0){
			attron(COLOR_PAIR(r));
			mvaddch(i/WIDTH,i%WIDTH,'*');//0x30+r);
			attroff(COLOR_PAIR(r));
		}
	}
	refresh();
}

/**Clears the shadow grid*/
void clshadow(){
	int i;
	for(i=0;i<WIDTH*HEIGHT;i++){
		shadow[i/WIDTH][i%WIDTH] = 0;
	}
}

/**Clears the pending shadow grid*/
void clpendsh(){
	int i;
	for(i=0;i<WIDTH*HEIGHT;i++){
		pendsh[i/WIDTH][i%WIDTH] = 0;
	}
}

/**Copies the pending shadow grid to the actual shadow*/
void acceptpendsh(){
	int i;
	for(i=0;i<WIDTH*HEIGHT;i++){
		shadow[i/WIDTH][i%WIDTH] |= pendsh[i/WIDTH][i%WIDTH];
	}
}

/**Removes combinations (groups of four) in the grid and refills; returns 1 if any were found.*/
int rmcombs(){
	int i,j,hascombs=0;
	clshadow();
	for(j=0;j<HEIGHT;j++){
		for(i=0;i<WIDTH;i++){
			int n = 0;
			clpendsh();
			ncomb(i,j,&n);
			if(n >= 4 && grid[j][i] != 0){
				hascombs = 1;
				acceptpendsh();
			}
		}
	}
	breakcombs();
	refill();
	return hascombs;
}

/**Returns the number of similar gems connected to the specified one (plus one.)*/
void ncomb(int i, int j, int *n){
	int q = grid[j][i];
	pendsh[j][i] = 1;
	if(j-1 >= 0){
		if(pendsh[j-1][i] == 0 && grid[j-1][i] == q) ncomb(i,j-1,n);
	}if(j+1 < HEIGHT){
		if(pendsh[j+1][i] == 0 && grid[j+1][i] == q) ncomb(i,j+1,n);
	}if(i-1 >= 0){
		if(pendsh[j][i-1] == 0 && grid[j][i-1] == q) ncomb(i-1,j,n);
	}if(i+1 < WIDTH){
		if(pendsh[j][i+1] == 0 && grid[j][i+1] == q) ncomb(i+1,j,n);
	}
	++*n;
}

void breakcombs(){
	int k;
	for(k=0;k<WIDTH*HEIGHT;k++){
		if(shadow[k/WIDTH][k%WIDTH] == 1){
			attron(COLOR_PAIR(grid[k/WIDTH][k%WIDTH]));
			mvaddch(k/WIDTH,k%WIDTH,'o');
		}
	}
	refresh();
	msleep(150); //is a nice number
	for(k=0;k<WIDTH*HEIGHT;k++){
		if(shadow[k/WIDTH][k%WIDTH] == 1){
			attron(COLOR_PAIR(grid[k/WIDTH][k%WIDTH]));
			mvaddch(k/WIDTH,k%WIDTH,'O');
		}
	}
	refresh();
	msleep(150);
	for(k=0;k<WIDTH*HEIGHT;k++){
		if(shadow[k/WIDTH][k%WIDTH] == 1){
			attron(COLOR_PAIR(grid[k/WIDTH][k%WIDTH]));
			mvaddch(k/WIDTH,k%WIDTH,' ');
			grid[k/WIDTH][k%WIDTH] = 0;
		}
	}
	refresh();
	msleep(150);
}

void refill(void){
	int hasblank = 0;
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
				grid[0][x] = randn(6)+1;
				hasblank = 1;
			}
		}
		redraw();
		msleep(150);
	}while(hasblank == 1);
}