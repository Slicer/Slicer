/*=auto=========================================================================

(c) Copyright 2001 Massachusetts Institute of Technology 

Permission is hereby granted, without payment, to copy, modify, display 
and distribute this software and its documentation, if any, for any purpose, 
provided that the above copyright notice and the following three paragraphs 
appear on all copies of this software.  Use of this software constitutes 
acceptance of these terms and conditions.

IN NO EVENT SHALL MIT BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, 
INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE 
AND ITS DOCUMENTATION, EVEN IF MIT HAS BEEN ADVISED OF THE POSSIBILITY OF 
SUCH DAMAGE.

MIT SPECIFICALLY DISCLAIMS ANY EXPRESS OR IMPLIED WARRANTIES INCLUDING, 
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR 
A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.

THE SOFTWARE IS PROVIDED "AS IS."  MIT HAS NO OBLIGATION TO PROVIDE 
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================auto=*/
#ifndef __vtkImageSumOverVoxels_h
#define __vtkImageSumOverVoxels_h


#include "vtkImageToImageFilter.h"
#include "vtkTumorGrowth.h"

class VTK_TUMORGROWTH_EXPORT vtkImageSumOverVoxels : public vtkImageToImageFilter
{
  public:
  static vtkImageSumOverVoxels *New();
  vtkTypeMacro(vtkImageSumOverVoxels,vtkImageToImageFilter);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  double GetVoxelSum() {return this->VoxelSum;}

protected:

  vtkImageSumOverVoxels() {VoxelSum = -1; };
  vtkImageSumOverVoxels(const vtkImageSumOverVoxels&) {};
  ~vtkImageSumOverVoxels(){};

  void operator=(const vtkImageSumOverVoxels&) {};
  
  // When it works on parallel machines use : 
  //  void ThreadedExecute(vtkImageData *inData, vtkImageData *outData,int outExt[6], int id);
  // If you do not want to have it multi threaded 
  void ExecuteData(vtkDataObject *);
  void ExecuteInformation(){this->vtkImageToImageFilter::ExecuteInformation();};
  void ExecuteInformation(vtkImageData *inData,vtkImageData *outData);
  void ComputeInputUpdateExtent(int inExt[6], int outExt[6]);
  
  double VoxelSum;
};
#endif



 







