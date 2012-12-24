/*
 *  Image.h
 *  BicubicInterpolator
 *
 *  Created by Balajee R.C. on 17/12/12.
 *  Copyright 2012 T. All rights reserved.
 *
 */

#ifndef IMAGE
#define IMAGE

#include <pthread.h>

//Forward declaration
class Pixel;

/*
 Class: Image
 Class representing an image
 Has facilities to read a specified image file into a byte array
 and write a byte array into an image file
 Uses SOIL imaging library for image loading and saving
*/
class Image{
	/*
	 Variable: buffer
	 Holds the buffer containing the image data as an array of unsigned chars
	 */
	unsigned char* buffer;	

	/*
	 Variable: newBuffer
	 Temporary buffer used to hold pixel data during resize operations
	 */
	unsigned char* newBuffer;	
	
	/*
	 Variable: height
	 Height of the texture
	 */ 
	int height;
	
	/*
	 Variable: width
	 Width of the texture
	 */
	int width;
	
	/*
	 Variable: channelCount
	 Number of channels in this image
	 */	
	int channelCount;
	
	enum SamplingDirection{
		ALONG_X,
		ALONG_Y
	};

	/*
	 Struct: PixelCalculationInfo
	 Struct used to encode the data required for pixel interpolation
	 calculations. Sent to threads spawned during resize operation
	 */
	struct PixelCalculationInfo{
		int x;
		int y;
		int maxWidth;
		int maxHeight;		
	};
	
	/*
	 Variable: imageLock
	 Synchronizes WRITES to image data across threads 
	 */
	static pthread_mutex_t imageLock;
	
	/*
	 Variable: resizedBufferLock
	 Mutex synchronizing thread access to the new buffer created during
	 resizing process
	 */		
	static pthread_mutex_t resizedBufferLock;	
	
	/*
	 Variable: iterLock
	 Mutex synchronizing thread access to the new buffer's pixel count
	 */		
	static pthread_mutex_t iterLock;
	
	/*
	 Variable: pixelCounter
	 Counters used to iterate over pixels in the new buffer during multithreaded
	 resize operation
	 */		
	int pixelCounter[2];
	
	/*
	 Variable: newBufferDimensions
	 Dimensions of the new temporary buffer created during multithreaded resize
	 operation
	 */
	int newBufferDimensions[2];
	
	/*
	 Variable: newThreads
	 Number of threads that a resize operation can spawn
	 */
	int numThreads;
	
public:
	/* 
	 Constructor: Image
	 Constructor
	 */ 
	Image();

	/* 
	 Destructor: Image
	 Destructor
	 */ 
	virtual ~Image();
	
	
	/* 
	 Method: loadFromFile
	 Loads an image specified in filepath into this
	 image instance's byte array
	 */ 	
	void loadFromFile(const char* filepath);
	
	/* 
	 Method: saveToFile
	 Writes the specified image to given filepath
	 NOTE: Saving possible only in TGA format
	 */ 	
	void saveToFile(const char* filepath);	
	
	/* 
	 Method: getWidth
	 Get the resolution width of the current texture (in pixels)
	 */ 					
	int getWidth();
	
	/* 
	 Method: getHeight
	 Get the resolution height of the current texture (in pixels)
	 */ 					
	int getHeight();
	
	/* 
	 Method: getNumChannels
	 Get the number of channels in the current texture's buffer
	 */ 					
	int getNumChannels();
	
	/* 
	 Method: getBuffer
	 Get the buffer from the current texture
	 */ 				
	unsigned char* getBuffer();
	
	/*
	 Method: getPixel
	 Returns a single pixel from the current texture
	 */
	void getPixel(int x, int y, Pixel& result);
	
	/*
	 Method: getPixel
	 Returns a single pixel from the current texture
	 Here the arguments are offset ratios (i.e. x offset/width, yoffset/height)
	 */
	void getPixel(double x, double y, Pixel& result);

	/* 
	 Method: setNumThreads
	 Sets the number of threads spawned in a resize operation
	 */ 					
	void setNumThreads(const int numThreads);	
	
	/* 
	 Method: resize
	 Resizes the image using bicubic interpolation
	 */ 					
	void resize(const int newWidth, const int newHeight);

private:
	/* 
	 Method: clamp
	 Clamps val between min and max
	 */		
	double clamp(const double val, const double min, const double max);

	/* 
	 Method: interpolate
	 Interpolates between 4 pixels at the given offset
	 */	
	void interpolate(const Pixel interpolants[], double offset, Pixel& result);

	/* 
	 Method: getSamplingCoords
	 Calculates the 4 surrounding coordinates (1 dimensional)
	 around the specified nearest coordinates. coords array passed
	 as argument must be an integer array of length 4
	 */			
	void getSamplingCoords(const int nearest, const int max, int coords[]);

	/* 
	 Method: interpolateAlongY
	 Given a pixel coordinate (xNear, yNear) and an offset along Y
	 this method returns a pixel by cubically interpolating the values
	 of the nearest 4 pixels in the Y direction
	 */	
	void interpolateAlongY(int xNear, int yNear, int yMax, double yOffset, Pixel& result);	

	/* 
	 Method: interpolateForPixel
	 Interpolates for a single pixel in the new image
	 */		
	void interpolateForPixel(const int i, const int j, const int newWidth, const int newHeight, Pixel& result);

	/* 
	 Method: equals
	 Compares if a is equal to b, allowing for a numerical precision threshold
	 */		
	bool equals(double a, double b, double threshold);

	/*
	 Method: getNextPixelForInterpolation
	 Each thread running interpolation calculations queries this method to acquire
	 the next pixel that needs to be interpolated
	 */
	void getNextPixelForInterpolation(int& xNext, int& yNext, int& maxWidth, int& maxHeight);
	
	/* 
	 Method: RunInterpolations
	 Static method that serves as the kernel for each thread running interpolation
	 calculations
	 */			
	static void* runInterpolations(void *args);
	
};


#endif
