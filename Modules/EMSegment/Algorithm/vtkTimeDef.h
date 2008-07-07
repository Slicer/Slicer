/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkTimeDef.h,v $
  Date:      $Date: 2006/12/08 23:28:23 $
  Version:   $Revision: 1.1 $

=========================================================================auto=*/
#ifndef __vtkTimeDef_h
#define __vtkTimeDef_h


#include "vtkEMSegment.h"
#include "vtkObject.h"

#ifndef _WIN32
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <errno.h>
#include <sys/time.h>

// ----------------------------------------------------------------------------------------------
// This file includes alll Data and time Structures  
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
// Time Stop Measure up to msec  
// ----------------------------------------------------------------------------------------------
// Do not forget to define in cxx
// extern "C" {
//  struct timeval preciseTimeEnd;
//  struct timeval preciseTimeStart;
// }
extern time_t timeToComputeTime; /* In seconds */
extern time_t timeCount; /* In seconds */
extern time_t timer_Start;
extern time_t timer_Stop;

extern "C" struct timeval preciseTimeStart;
extern "C" struct timeval preciseTimeEnd;
extern long milliSecondDifferences;

#define START_PRECISE_TIMING \
do { \
  gettimeofday(&preciseTimeStart, NULL); \
} while (0)

#define END_PRECISE_TIMING \
do { \
  gettimeofday(&preciseTimeEnd, NULL); \
} while (0)

 
#define SHOW_ELAPSED_PRECISE_TIME \
do { \
  double uend = 1e-06*(double)preciseTimeEnd.tv_usec; \
  double ustart = 1e-06*(double)preciseTimeStart.tv_usec; \
  double end = preciseTimeEnd.tv_sec + uend;\
  double start = preciseTimeStart.tv_sec + ustart;\
  printf("Elapsed time: %g\n", (end-start) ); \
} while(0)

#define START_TIMING \
do { \
  findTimeToComputeTime(); \
  timer_Start = time((time_t *)NULL); \
} while (0)

#define END_TIMING \
do { \
  timer_Stop = time((time_t *)NULL); \
  timeCount = timer_Stop - timer_Start - timeToComputeTime; \
} while(0)


#define SHOW_ELAPSED_TIME(fp) \
do { \
  fprintf(fp,"Elapsed time: %d\n",(int)timeCount); \
} while(0)


#endif // _WIN32




// ----------------------------------------------------------------------------------------------
// Dummy class 
// ----------------------------------------------------------------------------------------------/ 
class VTK_EMSEGMENT_EXPORT vtkTimeDef { 
public:
  static vtkTimeDef *New() {return (new vtkTimeDef);}
protected:

};

#endif
