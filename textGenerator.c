#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>



char* alphabet="acgt";

int main(int argc,char** argv){
	uint32_t i,t;
	int size;
	srand(time(NULL));
	if(argc!=2){
		printf("Error\n");
		printf("Usage = %s <textSize> > text.txt\n",argv[0]);
		exit(EXIT_FAILURE);
	}
	size = atoi(argv[1]);
	for(i=0;i<size;i++){
		t = rand()%4;
		putc(alphabet[t],stdout);
	}	
		putc('$',stdout);
	return(0);
}	
