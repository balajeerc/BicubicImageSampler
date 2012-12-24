#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "Interpolator.h"
#include "Image.h"

int main (int argc, char * const argv[]) {	
	//Usage programname inputimage newWidth newHeight	
	if(argc<4){
		printf("Usage: ./BicubicInterpolator inputImagePath newWidth newHeight\n");
		printf("Example: ./BicubicInterpolator inputImage.png  2048 1024\n");
		printf("NOTE: Please use absolute paths or images in the same directory only!\n");
		exit(1);
	}
	Image image;
	image.loadFromFile(argv[1]);
	int newWidth = atoi(argv[2]);
	int newHeight = atoi(argv[3]);
	if(newWidth<=0 || newHeight<=0){
		printf("Invalid dimensions for newHeight or newWidth specified! Must be integers.");
		assert(false);
	}
	//Optionally, set the maximum number of threads spawned
	//By default, 4 threads are spawned
	image.setNumThreads(6);
	image.resize(newWidth, newHeight);
	image.saveToFile("output.tga");
	return 0;
}

