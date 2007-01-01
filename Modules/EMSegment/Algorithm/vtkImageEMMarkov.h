/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkImageEMMarkov.h,v $
  Date:      $Date: 2006/12/08 23:28:23 $
  Version:   $Revision: 1.1 $

=========================================================================auto=*/
// .NAME vtkImageEMMarkov

#ifndef __vtkImageEMMarkov_h
#define __vtkImageEMMarkov_h

#include "vtkEMSegment.h"
#include "vtkImageData.h"
#include "vtkImageToImageFilter.h"
//!!!bcd!!!#include "vtkSlicer.h"

#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <string.h>


// Error Definiton  
// 1  = eveything correct
// -1 = Number of Classes not defined correctly
// -2 = Not all Mu defined correctly
// -3 = Not all Sigma defined correctly
// -4 = Not all Prob defined correctly
// -6 = Wrong index when in GetProbability
// -7 = SetMu: Incorrect index (" << index << ") or mu ("<<mu<<") < 0 !" << endl;
// -8 = SetSigma: Incorrect index (" << index << ") or sigma ("<<mu<<") < 0 !" << endl;
// -10 = Minimum Brightness value below 0 !



class VTK_EMSEGMENT_EXPORT vtkImageEMMarkov : public vtkImageToImageFilter
{
  public:
  static vtkImageEMMarkov *New();
  vtkTypeMacro(vtkImageEMMarkov,vtkImageToImageFilter);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Changing NumClasses re-defines also all the arrays which depon the number of classes e.g. prob

  void SetNumClasses(int Num);
  vtkGetMacro(NumClasses,int);

  vtkSetMacro(StartSlice, int);
  vtkGetMacro(StartSlice, int);

  vtkSetMacro(EndSlice, int);
  vtkGetMacro(EndSlice, int);

  vtkSetMacro(Error, int);
  vtkGetMacro(Error, int);

  // Description:
  // Setting the ImgTestNo > 0 executes the EM algorithm with a test picture
  // test = 1 => Creates horizontal stripped matrix with image values from 0 to 255 
  // test = 2 => Creates squared matrix with image values from 0 to 255 
  // test = 3 => Creates vertical stripped matrix with image values from 0 to 255
  vtkSetMacro(ImgTestNo,int);       
  vtkGetMacro(ImgTestNo,int);       
  // Description:
  // ImgTestDivision = the number of colors that the pixture should be devided in 
  //            if pixel == -1 => the number of divisions of the picture
  vtkSetMacro(ImgTestDivision, int); 
  vtkGetMacro(ImgTestDivision, int); 
  // Description:
  // ImgTestPixel = pixel length of one divison
  vtkSetMacro(ImgTestPixel, int); 
  vtkGetMacro(ImgTestPixel, int); 


  double GetProbability(int index);

  void SetLabelNumber(int index, int num);
  void SetLabel(int index, int Label);

  // Description:
  // Creates a Tissue Class Interation Matrix form the given image
  void TrainMarkovMatrix(int ***Image,int Ydim, int Xdim, float * outPtr);

  // Description:
  // Setting up Test Matrix to test 
  // test = 1 => Creates horizontal stripped matrix with image values from 0 to 255 
  // test = 2 => Creates squared matrix with image values from 0 to 255 
  // test = 3 => Creates vertical stripped matrix with image values from 0 to 255 
  // division = the number of colors that the pixture should be devided in 
  //            if pixel == -1 => the number of divisions of the picture
  // pixel = pixel length of one divison
  void setMatrix3DTest(int ***mat3D,int maxZ, int maxY, int maxX, int test,int division,int pixel);

protected:

  vtkImageEMMarkov();
  ~vtkImageEMMarkov();

  
  // When it works on parallel machines use : 
  //  void ThreadedExecute(vtkImageData *inData, vtkImageData *outData,int outExt[6], int id);
  // If you do not want to have it multi threaded 
  void ExecuteData(vtkDataObject *);
  void ExecuteInformation(){this->vtkImageToImageFilter::ExecuteInformation();};
  void ExecuteInformation(vtkImageData *inData,vtkImageData *outData);
  void ComputeInputUpdateExtent(int inExt[6], int outExt[6]);

  // Description:
  // Calculates the Maximum Class Probability for each brightness value  
  void CalculateMaxClassProb(double *MaxClass, int &ImageMin,int &ImageMax);

  void DeleteVariables();

  // Description:
  // Setting up Test Matrix to test 
  // test = 1 => Creates horizontal stripped matrix with image values from 0 to 255 
  // test = 2 => Creates squared matrix with image values from 0 to 255 
  // test = 3 => Creates vertical stripped matrix with image values from 0 to 255 
  // division = the number of colors that the pixture should be devided in 
  //            if pixel == -1 => the number of divisions of the picture
  // pixel = pixel length of one divison
  void setMatrixTest(int **mat, int maxY, int maxX, int test,int division, int pixel, int offset);
  void setVectorTest(int *vec, int maxX,int test,int division,int pixel, int offset);

  void setMatrix(int **mat, int maxY, int maxX, int val);

  int Error;           // Error Flag while executing 
  int NumClasses;      // Number of Classes

  int StartSlice;      // First Slide to be segmented
  int EndSlice;        // Last Slide to be segmented 

  int ImgTestNo;       // Segment an image test picture (-1 => No, > 0 =>certain Test pictures)
  int ImgTestDivision; // Number of divisions/colors of the picture
  int ImgTestPixel;    // Pixel lenght on one diviosn (pixel == -1 => max pixel length for devision)


  int **Label;            // Intensity distribution of the classes
  int *LabelNumber;            // Intensity distribution of the classes
  double *ClassProbability;  // Prior Probability of the classes
private:
  vtkImageEMMarkov(const vtkImageEMMarkov&);
  void operator=(const vtkImageEMMarkov&);
};
#endif



 







