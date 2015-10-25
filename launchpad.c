#include <stdio.h>
#include <math.h>
#include <unistd.h>

//#include <limit.h> 
//#include <stdint.h> 

// ----------------------------------
//         DEMO IMPORTS
// ----------------------------------
extern "C" {
#include <delay.h>
#include <FillPat.h>
#include <I2CEEPROM.h>
#include <LaunchPad.h>
#include <OrbitBoosterPackDefs.h>
#include <OrbitOled.h>
#include <OrbitOledChar.h>
#include <OrbitOledGrph.h>
}

/* ------------------------------------------------------------ */
/*				Global Variables		*/
/* ------------------------------------------------------------ */
extern int xchOledMax; // defined in OrbitOled.c
extern int ychOledMax; // defined in OrbitOled.c

#define L_BITMAP 16
#define LENGTH 128 // maybe set to xchOledMax?
#define HEIGHT 32
char bitmap[LENGTH*HEIGHT/8];
int aliveNow[LENGTH][HEIGHT];
int aliveNext[LENGTH][HEIGHT];
int count = 0;
int current_count = 0;
int total_iter = 0;

int power(int base, int exponent) {
    //assert (exponent>=0);
    int i, result = 1;
    for (i = 0; i < exponent; i++)
        result *= base;
    return result;
 }

int countLivingNeighbours(int i, int j){
	int surroundings = 0, k, l;
	for(k=i-1;k<=i+1;k++){
		for(l=j-1;l<=j+1;l++){
			if (0<=k && k< LENGTH && 0<=l && l< HEIGHT && !(k==i && l==j) && aliveNow[k][l])
                    surroundings++;
		}
	}
	return surroundings;
}

void fillNextArray(){
	int i,j;
	for(i=0;i<LENGTH;i++){
		for(j=0;j<HEIGHT;j++){
			aliveNext[i][j] = countLivingNeighbours(i,j);
		}
	}
}

void updateCurrentArray(){
	int i,j;
	for(i=0;i<LENGTH;i++){
		for(j=0;j<HEIGHT;j++){
			if(aliveNow[i][j]){
				if(aliveNext[i][j]<2) aliveNow[i][j]=0;
				else if(aliveNext[i][j]>3) aliveNow[i][j]=0;
				else current_count++;
			}
			else{
				if(aliveNext[i][j]==3) {
					aliveNow[i][j]=1;
					current_count++;
				}
			}
		}
	}
}

void populate(){
	srand(time(NULL));
	int i,j;
	for(i=0;i<LENGTH;i++){
		for(j=0;j<HEIGHT;j++){
			aliveNow[i][j] = rand()%2;
		}
	}
}

int checkStability(){
	int ret = 0;
	if(current_count==count) ret = 1;
	count = current_count;
	current_count = 0;
	return ret;
}

void convert(){
	int i,j,n=8, ret=0, index =0;
	for(j=0;j<HEIGHT;j++){
		for(i=0;i<LENGTH;i++){
			if(n==0){
				bitmap[index++] = (char)ret;
				n=8;
				ret=0;
			}
			ret+= aliveNow[i][j] * power(2,--n);
		}
	}
}


int main(){
	int q, i,j;
	populate();
	for(q=0;q<2;){
		fillNextArray();
		updateCurrentArray(); 
		convert(); // converts the bool array to bitmap

		// HEX PRINT
		/*for(j=0;j<HEIGHT;j++){
			for(i=0;i<L_BITMAP;i++){
				printf("0x%02hhX ",bitmap[j*L_BITMAP+i]);
			}
			printf("\n");
		}*/

		// CONSOLE PRINT
		/*for(j=0;j<HEIGHT;j++){
			for(i=0;i<LENGTH;i++){
				//printf("%d", aliveNext[i][j]);
				if(aliveNow[i][j])printf("██");
				else printf("  ");
			}
			printf("     %d\n",j);
		}*/

		if(checkStability()){
			printf("STABLE  ");
		}
		else printf("UNSTABLE");
		printf("   %d\n", ++total_iter);

		OrbitOledMoveTo(0,0);
		OrbitOledPutBmp(LENGTH,HEIGHT,bitmap);

		usleep(100000);
	}
}