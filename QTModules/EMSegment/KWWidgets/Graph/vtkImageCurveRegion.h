/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkImageCurveRegion.h,v $
  Date:      $Date: 2006/04/13 19:29:02 $
  Version:   $Revision: 1.8 $

=========================================================================auto=*/
// .NAME vtkImageCurveRegion - Abstract Filter used in slicer to plot graphs
// .SECTION Description
// vtkImageCurveRegion
//

#ifndef __vtkImageCurveRegion_h
#define __vtkImageCurveRegion_h


#include "vtkImageAlgorithm.h"

#include "vtkImageSource.h"
#include "vtkEMSegmentStep.h"


// Copied from vtkImageGeneral.h
// Abuse the type system.
#define COERCE(x, type) (*((type *)(&(x))))
// Some constants having to do with the way single
// floats are represented on alphas and sparcs
#define EMSEGMENT_MANTSIZE (23)
#define EMSEGMENT_SIGNBIT (1 << 31)
#define EMSEGMENT_EXPMASK (255 << EMSEGMENT_MANTSIZE)
#define EMSEGMENT_MENTMASK ((~EMSEGMENT_EXPMASK)&(~EMSEGMENT_SIGNBIT))
#define EMSEGMENT_PHANTOM_BIT (1 << EMSEGMENT_MANTSIZE)
#define EMSEGMENT_EXPBIAS 127
#define EMSEGMENT_SHIFTED_BIAS (EMSEGMENT_EXPBIAS << EMSEGMENT_MANTSIZE)
#define EMSEGMENT_SHIFTED_BIAS_COMP ((~ EMSEGMENT_SHIFTED_BIAS) + 1)
#define EMSEGMENT_ONE_OVER_2_PI 0.5/3.14159265358979
#define EMSEGMENT_ONE_OVER_ROOT_2_PI sqrt(EMSEGMENT_ONE_OVER_2_PI)
#define EMSEGMENT_MINUS_ONE_OVER_2_LOG_2 ((float) -.72134752)

class VTK_EMSEGMENT_EXPORT vtkImageCurveRegion : public vtkImageAlgorithm 
{
public:
  static vtkImageCurveRegion *New();
  vtkTypeRevisionMacro(vtkImageCurveRegion,vtkImageAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

void SetDimension(int value);
  vtkGetMacro(Dimension, int);
  vtkSetMacro(Xmin, float);
  vtkGetMacro(Xmin, float);
  vtkSetMacro(Xmax, float);
  vtkGetMacro(Xmax, float);
  // Description:
  // Define space between two points on the X-Axis to be calculated, e.g. 0.1, or define Xlength 
  vtkSetMacro(Xunit, float);
  vtkGetMacro(Xunit, float);
  vtkSetMacro(Xlength, int);
  vtkGetMacro(Xlength, int);
  // Description:
  // Ymin, Ymax, Yunit can only be set for Dimension = 2 (=> to calculate reange in second dimension, like Xmin Xmax for first dimension)
  vtkSetMacro(Ymin, float);
  vtkGetMacro(Ymin, float);
  vtkSetMacro(Ymax, float);
  vtkGetMacro(Ymax, float);
  // Description:
  // Define space between two points on the Y-Axis to be calculated, e.g. 0.1, or define Ylength 
  vtkSetMacro(Yunit, float);
  vtkGetMacro(Yunit, float);

  vtkSetMacro(Ylength, int);
  vtkGetMacro(Ylength, int);

  // Description:
  // Maximum and Minimum value of the function
  vtkGetMacro(FctMax, float);
  vtkGetMacro(FctMin, float);

  // Description:
  // Function to be executed. Currently implemented
  // 1 = Gausian
  // 2 = Log Gausian, where *min, *max, *unit correspond to the non log values (e.g. 0, 100, 1.0) 
  //     and Covariance and mean to log values  (3.65, 0.2)
  // 3 = Data will be read from file (FileName has to be defined)
  void SetFunction(int val);
  vtkGetMacro(Function, int);
  // Description:
  // Define first dimension before defining mean and covariance 
  // Also if you choose Function 2 Mean and Covirance correpsond to their log values , e.g. Mean = 50 => for function 2 enter 
  // Mean = log(50+1) !!
  void  SetMean(float value, int x) { if (this->Mean[x] != value) {this->Mean[x] = value; this->Modified();}} 
  float GetMean(int x) {return this->Mean[x];} 

  void  SetCovariance(float value, int y, int x) {if (this->Covariance[y][x] != value) {this->Covariance[y][x] = value; this->Modified();}} 
  float GetCovariance(int y, int x) {return this->Covariance[y][x];} 

  // Description:
  // Global probability, necessary if you have different curves with different weights 
  vtkSetMacro(Probability, float);
  vtkGetMacro(Probability, float);

  vtkGetStringMacro(FileName);
  vtkSetStringMacro(FileName);

protected:
  vtkImageCurveRegion();

   void DeleteVariables();
  ~vtkImageCurveRegion() {this->DeleteVariables();};

  int Dimension;
  int Function;
  float Xmin;
  float Xmax;
  float Xunit;
  int Xlength;

  float Ymin;
  float Ymax;
  float Yunit;
  int Ylength;

  float FctMax;
  float FctMin;

  float* Mean;
  float** Covariance;
  float Probability;

  char* FileName;

  void ExecuteDataReadFile(vtkDataObject *output) ;
  void ExecuteDataGauss(vtkDataObject *output);
  virtual void ExecuteData(vtkDataObject *data);
  virtual int RequestInformation (vtkInformation *, vtkInformationVector**, vtkInformationVector *);

private:
  vtkImageCurveRegion(const vtkImageCurveRegion&);  // Not implemented.
  void operator=(const vtkImageCurveRegion&);  // Not implemented.
};


#endif

  
