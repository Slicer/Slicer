#ifndef __vtkEMSegmentTestUtilities_h
#define __vtkEMSegmentTestUtilities_h

#include <string>
class vtkImageData;
class vtkMRMLScene;

/**
 * Check to see if the image stored in standardFilename differs from
 * resultData.  True is returned if the images differ, false is
 * returned if they are identical.
 */
bool ImageDiff(vtkImageData* resultData, std::string standardFilename);

#endif
