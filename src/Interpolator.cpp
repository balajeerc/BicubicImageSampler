/*
 *  Interpolator.cpp
 *  BicubicInterpolator
 *
 *  Created by Balajee R.C. on 17/12/12.
 *  Copyright 2012 T. All rights reserved.
 *
 */

#include <stdio.h>
#include <math.h>

#include "Interpolator.h"

Interpolator::Interpolator(){
	
}

double Interpolator::interpolate(const double p0, const double p1, const double p2, const double p3, const double x){
	return  (-0.5f*p0+1.5f*p1-1.5f*p2+0.5*p3)*pow(x,3)+
			(p0-2.5f*p1+2.f*p2-0.5f*p3)*pow(x,2)+
			(-0.5f*p0+0.5f*p2)*x+
			p1;
	
//	return   (((( -7.f * p0 + 21.f * p1 - 21.f * p2 + 7.f * p3 ) * x +
//				( 15.f * p0 - 36.f * p1 + 27.f * p2 - 6.f * p3 ) ) * x +
//                ( -9.f * p0 + 9.f * p2 ) ) * x + (p0 + 16.f * p1 + p2) ) / 18.0;
	
}

double Interpolator::interpolate(const double p1, const double p2, const double x){
	return interpolate(p1,p1,p2,p2,x);
}