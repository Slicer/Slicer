/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkImageStash.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
///  vtkImageStash - 
///  Store an image data in a compressed form to save memory

#ifndef __vtkImageStash_h
#define __vtkImageStash_h

#include "vtkSlicerBaseLogic.h"

#include "vtkImageData.h"
#include "vtkUnsignedCharArray.h"
#include "vtkZLibDataCompressor.h"
#include "vtkMultiThreader.h"

class VTK_SLICER_BASE_LOGIC_EXPORT vtkImageStash : public vtkObject
{
public:
  static vtkImageStash *New();
  vtkTypeRevisionMacro(vtkImageStash,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  /// 
  /// The stash image: 
  /// This image data will have the Scalars removed
  /// and they will be stored in a local compressed data
  /// array inside this class when the Stash method is called.
  /// You must call Unstash to have the scalars put back into
  /// this image data.
  vtkSetObjectMacro(StashImage, vtkImageData);
  vtkGetObjectMacro(StashImage, vtkImageData);

  /// 
  /// The stashed scalars:
  /// this is the zlib compressed image scalar data
  vtkSetObjectMacro(StashedScalars, vtkUnsignedCharArray);
  vtkGetObjectMacro(StashedScalars, vtkUnsignedCharArray);

  // Description:
  // To keep track of original number of tuples in scalar data
  vtkSetMacro(NumberOfTuples, vtkIdType);
  vtkGetMacro(NumberOfTuples, vtkIdType);

  /// 
  /// The multi-threader used when TreadedStash is called
  vtkSetObjectMacro(MultiThreader, vtkMultiThreader);
  vtkGetObjectMacro(MultiThreader, vtkMultiThreader);
  
  /// 
  /// compress and strip the scalars
  void Stash();

  /// 
  /// compress and strip the scalars in a separate thread
  void ThreadedStash();

  /// 
  /// decompress and restore the scalars
  void Unstash();

  // Description:
  // Get/Set the compression level.
  vtkSetClampMacro(CompressionLevel, int, 0, 9);
  vtkGetMacro(CompressionLevel, int);

  // Description:
  // Get/Set the compressor if needed
  vtkSetObjectMacro(Compressor, vtkZLibDataCompressor);
  vtkGetObjectMacro(Compressor, vtkZLibDataCompressor);

  // Description:
  // Check if compression thread is finished
  vtkSetMacro(Stashing, int);
  vtkGetMacro(Stashing, int);
 
protected:
  vtkImageStash();
  ~vtkImageStash();

  vtkImageData *StashImage;
  vtkUnsignedCharArray *StashedScalars;
  vtkMultiThreader *MultiThreader;
  vtkIdType NumberOfTuples;
  vtkZLibDataCompressor *Compressor;
  int CompressionLevel;
  int Stashing;

private:
  int StashingThreadID;

  vtkImageStash(const vtkImageStash&);  /// Not implemented.
  void operator=(const vtkImageStash&);  /// Not implemented.
};



#endif



