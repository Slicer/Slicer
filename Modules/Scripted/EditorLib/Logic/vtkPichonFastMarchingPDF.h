/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/
#ifndef __PichonFastMarchingPDF_h
#define __PichonFastMarchingPDF_h

#include "vtkSlicerEditorLibModuleLogicExport.h"

/*

This class is used by vtkFastMarching to estimate the probability density function
of Intensity and Inhomogeneity

*/

#include <deque>

class PichonFastMarchingPDF
{
public:
  double getMean() { return mean; };
  double getSigma2() { return sigma2; };

  PichonFastMarchingPDF( int realizationMax );
  ~PichonFastMarchingPDF();

  void setMemory( int mem );
  void setUpdateRate( int rate );

  bool willUseGaussian();

  void reset();
  void update();

  double value( int k );
  void addRealization( int k );

  /*
  bool isUnlikelyGauss( double k );
  bool isUnlikelyBigGauss( double k );
  */

  void show();

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
