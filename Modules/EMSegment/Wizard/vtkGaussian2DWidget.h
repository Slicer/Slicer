/*=auto==============================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All
Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkGaussian2DWidget.h,v $
Date:      $Date: 2006/01/06 17:56:51 $
Version:   $Revision: 1.6 $
Author:    $Sylvain Jaume (MIT)$

==============================================================auto=*/
// .NAME vtkGaussian2DWidget - a render widget
// .SECTION Description
// This class encapsulates a render window, a renderer and several other
// objects inside a single widget. Actors and props can be added,
// annotations can be set.
// .WARNING
// This widget set the camera to be in parallel projection mode.
// You can change this default (after Create()) by calling:
// renderwidget->GetRenderer()->GetActiveCamera()->ParallelProjectionOff();

#ifndef __vtkGaussian2DWidget_h
#define __vtkGaussian2DWidget_h

#include "vtkEMSegment.h"
#include "vtkKWRenderWidget.h"

class vtkDoubleArray;

class VTK_EMSEGMENT_EXPORT vtkGaussian2DWidget : public vtkKWRenderWidget
{
public:
  static vtkGaussian2DWidget* New();
  vtkTypeRevisionMacro(vtkGaussian2DWidget, vtkKWRenderWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Define the range of intensities in image 1 (X axis)
  vtkSetVector2Macro(XAxisRange, double);
  vtkGetVector2Macro(XAxisRange, double);

  // Description:
  // Define the range of intensities in image 2 (Y axis)
  vtkSetVector2Macro(YAxisRange, double);
  vtkGetVector2Macro(YAxisRange, double);

  // Description:
  // Set the mean and standard deviation to define an additional
  // gaussian distribution
  void AddGaussian(double meanX, double meanY, double varianceX, double varianceY, double covariance,
      double rgb[3]);
  
  // Description:
  // Remove all the gaussians without deleting the widget  
  void RemoveAllGaussians();

  // Description:
  // Create the gaussians.
  virtual void CreateGaussian2D();
/*
  // Description:
  // Set/Get the upper left element of the covariance matrix.
  vtkSetMacro(VarianceX,double);
  vtkGetMacro(VarianceX,double);

  // Description:
  // Set/Get the upper right element of the covariance matrix.
  // Note that the lower left element has the same value.
  vtkSetMacro(Covariance,double);
  vtkGetMacro(Covariance,double);

  // Description:
  // Set/Get the lower right element of the covariance matrix.
  vtkSetMacro(VarianceY,double);
  vtkGetMacro(VarianceY,double);
  
  // Description:
  // Set/Get the lower right element of the covariance matrix.
  vtkSetMacro(MeanX,double);
  vtkGetMacro(MeanX,double);
  
  // Description:
  // Set/Get the lower right element of the covariance matrix.
  vtkSetMacro(MeanY,double);
  vtkGetMacro(MeanY,double);*/

protected:
  vtkGaussian2DWidget();
  ~vtkGaussian2DWidget();

  double XAxisRange[2];
  double YAxisRange[2];

  /*double VarianceX;
  double VarianceY;
  double Covariance;
  
  double MeanX;
  double MeanY;*/
  
  int NbOfGaussians;

  vtkDoubleArray *MeanXArray;
  vtkDoubleArray *MeanYArray;
  vtkDoubleArray *VarianceXArray;
  vtkDoubleArray *VarianceYArray;
  //vtkDoubleArray *CovarianceArray;
  vtkDoubleArray *RotationAngleArray;

  vtkDoubleArray *RGBArray;

private:
  vtkGaussian2DWidget(const vtkGaussian2DWidget&);  // Not implemented
  void operator=(const vtkGaussian2DWidget&);  // Not implemented
};

#endif

