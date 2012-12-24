/*
 *  Interpolator.h
 *  BicubicInterpolator
 *
 *  Created by Balajee R.C. on 17/12/12.
 *  Copyright 2012 T. All rights reserved.
 *
 */

#ifndef INTERPPOLATOR
#define INTERPOLATOR

/*
 Class: Interpolator
 Cubic interpolation math implementation
 Source: http://www.paulinternet.nl/?page=bicubic
*/
class Interpolator{
	//Making constructor private since this class is not supposed
	//to be instantiated. It is simply a collection of static methods
	Interpolator();
public:
	/* 
	 Method: interpolate
	 Interpolates with 4 explicit values
	*/ 		
	static double interpolate(const double p0,const double p1,const double p2,const double p3,const double x);
	
	/* 
	 Method: interpolate
	 Interpolates with 2 values, used at the boundaries of image being interpolated
	 */ 			
	static double interpolate(const double p1, const double p2, const double x);
};


#endif //INTERPOLATOR

