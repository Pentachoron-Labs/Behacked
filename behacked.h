#define	WIDTH	20
#define HEIGHT	10

int randn(int);
void msleep(int);
void fill(void);
void redraw(void);
void clshadow(void);
void clpendsh(void);
void acceptpendsh(void);
int rmcombs(void);
int ncombs(void);
void ncomb(int x,int y,int*);
void breakcombs(void);
void refill(void);

int score;
int grid[HEIGHT][WIDTH];
int shadow[HEIGHT][WIDTH];
int pendsh[HEIGHT][WIDTH]; //pending shadow grid
int c; //character last typed
int cx,cy; //cursor x/y
int sx,sy; //selected x/y