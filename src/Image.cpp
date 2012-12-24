/*
 *  Image.cpp
 *  BicubicInterpolator
 *
 *  Created by Balajee R.C. on 17/12/12.
 *  Copyright 2012 T. All rights reserved.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>


#include "SOIL.h"
#include "Pixel.h"
#include "Interpolator.h"

#define ERROR_THRESHOLD 0.0000001
#define MAX(a, b) ((a)>(b) ? (a) : (b))
#define MIN(a, b) ((a)<(b) ? (a) : (b))

#include "Image.h"

pthread_mutex_t Image::imageLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t Image::resizedBufferLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t Image::iterLock = PTHREAD_MUTEX_INITIALIZER;

Image::Image(){
	buffer = NULL;
	width = 0;
	height = 0;
	channelCount = 0;
	pixelCounter[0] = -1;
	pixelCounter[1] = -1;
	numThreads = 4;
}

Image::~Image(){
	if(buffer)
	{	
		delete [] buffer;
		buffer = NULL;
	}
	
	if(newBuffer)
	{	
		delete [] newBuffer;
		newBuffer = NULL;
	}
}


int Image::getWidth(){
	return width;
}

int Image::getHeight(){
	return height;
}

int Image::getNumChannels(){
	return channelCount;
}

unsigned char* Image::getBuffer(){
	return buffer;
}

void Image::setNumThreads(const int numThreads){
	pthread_mutex_lock(&imageLock);
	this->numThreads = numThreads;
	pthread_mutex_unlock(&imageLock);
}

void Image::loadFromFile(const char* filename)
{
	pthread_mutex_lock(&imageLock);
	int imgWidth, imgHeight, imgChannels;
	unsigned char *ht_map = SOIL_load_image(filename,
						&imgWidth,
						&imgHeight,
						&imgChannels,
						SOIL_LOAD_AUTO);
	
	//Check for an error during the load process
	if(!ht_map)
	{
		printf( "ERROR: SOIL failed to load image. Reason: '%s'\n", SOIL_last_result() );
		assert(false);
	}
	
	height = imgHeight;
	width = imgWidth;
	channelCount = imgChannels;
	
	//If we have a previously loaded image already, we discard the
	//memory allocated for it
	if (buffer)
		delete [] buffer;
	
	int numBytes = height*width*channelCount;
	buffer = new unsigned char[numBytes];
	memcpy( (void*)buffer, (void*)ht_map, numBytes*sizeof(unsigned char));
		
	//Deallocate the memory taken up by SOIL
	SOIL_free_image_data(ht_map);
	
	pthread_mutex_unlock(&imageLock);
}

void Image::saveToFile(const char* filename){
	pthread_mutex_lock(&imageLock);	
	int result = SOIL_save_image(filename,
					SOIL_SAVE_TYPE_TGA,
					this->width,
					this->height,
					this->channelCount,
					this->buffer);
	if(result!=1){
		printf("ERROR: Soil failed to save image! Reason: '%s'\n", SOIL_last_result());
		assert(false);
	}
	pthread_mutex_unlock(&imageLock);
}

//threadsafe
void Image::getPixel(int x, int y, Pixel& result){
	size_t offsetTot = (y*width+x)*channelCount;
	result.r = *(buffer+offsetTot);
	result.g = *(buffer+offsetTot+1);
	result.b = *(buffer+offsetTot+2);			
	if (channelCount == 4)
		result.a = *(buffer+offsetTot+3);
	else
		result.a = 1.f;	
}

//threadsafe
void Image::getPixel(double x, double y, Pixel& result){
	getPixel((int)x*width, (int)y*height, result);
}

void Image::resize(const int newWidth, const int newHeight){
	//Prevent any other resize operation from running when
	//this resize opertion is in progress
	pthread_mutex_lock(&imageLock);
	
	//Ensure that we have a valid buffer already
	if(buffer==NULL){
		printf("ERROR: Must load an image before resizing it!");
		assert(false);
	}
	//We first need to create a new buffer with the new dimensions
	newBuffer = new unsigned char[newWidth*newHeight*channelCount];
	newBufferDimensions[0] = newWidth;
	newBufferDimensions[1] = newHeight;
	//Reset pixel counter
	pixelCounter[0]=0;
	pixelCounter[1]=0;
	//Create handles for the required number of threads
	pthread_t* threads = (pthread_t*)malloc(sizeof(pthread_t)*numThreads);	
	//Spawn the threads
	for(int i=0; i<numThreads; ++i){
		pthread_create(threads+i, NULL, runInterpolations, (void*)this);
	}		
	//Wait till all threads finish running
	for(int i=0; i<numThreads; ++i)
		pthread_join(threads[i], NULL);	
	//Deallocate memory for thread handles
	free(threads);	
	
	//Now we can deallocate the memory of our current buffer
	delete [] buffer;
	//Reassign our newly sampled buffer to our own
	buffer = newBuffer;
	newBuffer = NULL;
	//Reset our image dimensions
	height = newHeight;
	width = newWidth;	
	//Unlock image lock so subsequent operations can be called on this image instance
	pthread_mutex_unlock(&imageLock);
}

//reentrant
bool Image::equals(double a, double b, double threshold){
	if(fabs(a-b)<=threshold)
		return true;
	return false;
}

//reentrant
double Image::clamp(const double val, const double min, const double max){
	if(val<min)
		return min;
	if(val>max)
		return max;
	return val;
}

//reentrant
void Image::interpolate(const Pixel p[], double offset, Pixel& result){
	result.r = Interpolator::interpolate(p[0].r,p[1].r,p[2].r,p[3].r,offset);	
	result.g = Interpolator::interpolate(p[0].g,p[1].g,p[2].g,p[3].g,offset);
	result.b = Interpolator::interpolate(p[0].b,p[1].b,p[2].b,p[3].b,offset);
	result.a = Interpolator::interpolate(p[0].a,p[1].a,p[2].a,p[3].a,offset);
	//Clamp the resultant values to [0,1]
	result.r = clamp(result.r,0.f,255.f);
	result.g = clamp(result.g,0.f,255.f);
	result.b = clamp(result.b,0.f,255.f);
	result.a = clamp(result.a,0.f,255.f);	
}

//reentrant
void Image::getSamplingCoords(const int nearest, const int max, int coords[]){
	
	coords[0] = nearest-1;
	if(coords[0]<0)
		coords[0] = nearest;
	coords[1] = nearest;
	coords[2] = nearest+1;
	if(coords[2]>=max)
		coords[2] = nearest;	
	coords[3] = nearest+2;
	if(coords[3]>=max)
		coords[3] = nearest-1;
}

//reentrant
void Image::interpolateAlongY(int x, int y, int yMax, double yOffset, Pixel& result){
	
	if(equals(yOffset,0.f,ERROR_THRESHOLD)){
		//No interpolation required
		getPixel(x,y,result);
		return;
	}
	
	int yCoords[4];
	getSamplingCoords(y, yMax, yCoords);	
	Pixel interpolants[4];
	for(int i=0; i<4; ++i){
		getPixel(x, yCoords[i], interpolants[i]);
	}	
	interpolate(interpolants, yOffset, result);
}

void Image::getNextPixelForInterpolation(int& xNext, int& yNext, int& maxWidth, int& maxHeight){
	pthread_mutex_lock(&iterLock);

	pixelCounter[0] += 1;
	if(!(pixelCounter[0]<newBufferDimensions[0])){
		pixelCounter[0] = 0;
		pixelCounter[1]++;
	}
	
	if(!(pixelCounter[1]<newBufferDimensions[1])){
		//Send negative dimensions to the thread requesting the data
		//This will tell the thread that all pixel computations are done and that it
		//can end
		xNext = -1;
		yNext = -1;
		maxWidth = -1;
		maxHeight = -1;
	}else{		
		xNext = pixelCounter[0];
		yNext = pixelCounter[1];
		maxWidth = newBufferDimensions[0];
		maxHeight = newBufferDimensions[1];
	}
	
	pthread_mutex_unlock(&iterLock);	
}

//reentrant
void Image::interpolateForPixel(const int i, const int j, const int newWidth, const int newHeight, Pixel& result){
	//For this pixel in the target image we
	//a) Find the nearest pixel in the source image
	//b) Find the offset from the aforementioned nearest pixel
	int xNear,yNear;
	double xOffset,yOffset;
	double x = ((double)width/(double)newWidth)*i;
	double y = ((double)height/(double)newHeight)*j;	
	xNear = floor(x);
	yNear = floor(y);	
	xOffset = x-xNear;
	yOffset = y-yNear;
	
	//We make a check that xNear and yNear obtained above
	//are always smaller than the edge pixels at the extremeties
	if(xNear>=width || yNear>=height){
		printf("ERROR: Nearest pixel computation error!");
		assert(false);
	}			
	//Next we find four pixels along the x direction around this
	//nearest pixel
	int xCoords[4];
	getSamplingCoords(xNear,width,xCoords);
	
	//For each of these sampling xCoords, we interpolate 4 nearest points
	//along Y direction
	Pixel yInterps[4];
	for(int k=0; k<4; k++){
		interpolateAlongY(xCoords[k], yNear, height, yOffset, yInterps[k]);
	}			
	//Finally, the resultant pixel is a cubic interpolation
	//on the 4 obtained pixels above
	if(equals(xOffset,0.f,ERROR_THRESHOLD)){
		//We note the resultant colour and also
		//clamp the values of the resultant pixel
		//between 0 and 1
		result.r = clamp(yInterps[1].r,0.f,255.f);
		result.g = clamp(yInterps[1].g,0.f,255.f);
		result.b = clamp(yInterps[1].b,0.f,255.f);
		result.a = clamp(yInterps[1].a,0.f,255.f);
	}else{
		interpolate(yInterps, xOffset, result);
	}
}

void* Image::runInterpolations(void *args){
	int x=0,y=0;
	int newWidth, newHeight;		
	Image* image = (Image*)args;
	while(true){
		image->getNextPixelForInterpolation(x, y, newWidth, newHeight);
		//If the values of x and y we obtain are negative, it means that
		//all pixel computations have been completed. We can just exit the thread
		if(x<0 || y<0)
			break;
		size_t newIndexOffset = (y*newWidth+x)*(image->channelCount);
		Pixel result;
		image->interpolateForPixel(x, y, newWidth, newHeight, result);
		
		//Lock new buffer before writing to it
		pthread_mutex_lock(&resizedBufferLock);
		*((image->newBuffer)+newIndexOffset) = result.r;
		*((image->newBuffer)+newIndexOffset+1) = result.g;
		*((image->newBuffer)+newIndexOffset+2) = result.b;
		if(image->channelCount==4)
			*((image->newBuffer)+newIndexOffset+3) = result.a;
		pthread_mutex_unlock(&resizedBufferLock);
	}
	return 0;
}


