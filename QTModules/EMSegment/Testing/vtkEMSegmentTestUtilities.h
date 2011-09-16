#ifndef __vtkEMSegmentTestUtilities_h
#define __vtkEMSegmentTestUtilities_h

#include <string>
class vtkImageData;
class vtkMRMLScene;
class vtkImageMathematics;
class vtkImageAccumulate;

/**
 * Check to see if the image stored in standardFilename differs from
 * resultData.  True is returned if the images differ, false is
 * returned if they are identical.
 */
bool ImageDiff(vtkImageData* resultData, std::string standardFilename);
double CompareTwoVolumes ( vtkImageData* Volume1, vtkImageData* Volume2 , int Flag );
double* GenerateHistogram ( vtkImageAccumulate* Histogram, vtkImageData* InputVolume, int bins=-1 );

#endif
