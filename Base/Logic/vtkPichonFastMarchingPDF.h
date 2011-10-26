/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/
#ifndef __PichonFastMarchingPDF_h
#define __PichonFastMarchingPDF_h

#include "vtkSlicerBaseLogic.h"

#ifdef _WIN32 /// WINDOWS

#include <float.h>
#define isnan(x) _isnan(x)
#define finite(x) _finite(x)

#ifndef min
#define min(a,b) (((a)<(b))?(a):(b))
#endif

#ifndef M_PI
#define M_PI 3.1415926535898
#endif

#include <deque>

#else /// UNIX

#if defined(sun) || defined(__sun)
#include <math.h>
#include <ieeefp.h>
#endif
#include <deque>

#endif

#include <vtkObject.h>


/*

This class is used by vtkFastMarching to estimate the probability density function
of Intensity and Inhomogeneity

*/

class VTK_SLICER_BASE_LOGIC_EXPORT vtkPichonFastMarchingPDF : public vtkObject
{
public:

  double getMean( void ) { return mean; };
  double getSigma2( void ) { return sigma2; };

  
  vtkPichonFastMarchingPDF(){};
  vtkPichonFastMarchingPDF( int realizationMax );
  ~vtkPichonFastMarchingPDF();

  static vtkPichonFastMarchingPDF* New();

  void setMemory( int mem );
  void setUpdateRate( int rate );

  bool willUseGaussian( void );

  void reset( void );  
  void update( void );

  double value( int k );
  void addRealization( int k );

  /*
  bool isUnlikelyGauss( double k );
  bool isUnlikelyBigGauss( double k );
  */

  void show( void );

  const char* GetClassName(void)
    {return "vtkPichonFastMarchingPDF"; };


  friend class vtkFastMarching;

  double sigma2SmoothPDF;

  int realizationMax;

  int counter;
  int memorySize; /// -1=don't ever forget anything
  int updateRate;

  /// the histogram
  int *bins;
  int nRealInBins;

  double *smoothedBins;

  double * coefGauss;  

  std::deque<int> inBins;
  std::deque<int> toBeAdded;

  /// first 2 moments (not centered, not divided by number of samples)  
  double m1;
  double m2;

  /// first 2 moments (centered)
  double sigma2;
  double mean;

  double valueHisto( int k );
  double valueGauss( int k );

};

#endif

