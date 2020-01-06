/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkImageStash.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkImageStash.h"

#include "vtkPointData.h"
#include "vtkObjectFactory.h"

vtkStandardNewMacro(vtkImageStash);

//----------------------------------------------------------------------------
vtkImageStash::vtkImageStash()
{
  this->StashImage = nullptr;
  this->StashedScalars = nullptr;
  this->MultiThreader = vtkMultiThreader::New();
  this->Compressor = vtkZLibDataCompressor::New();
  this->CompressionLevel = 1; // corresponds to Z_BEST_SPEED
  this->Stashing = 0;
  this->StashingThreadID = 0;
  this->StashingSucceeded = 0;
}

//----------------------------------------------------------------------------
vtkImageStash::~vtkImageStash()
{

  if (this->Stashing && this->MultiThreader)
    {
    this->MultiThreader->TerminateThread(this->StashingThreadID);
    }
  if (this->StashImage)
    {
    this->StashImage->Delete();
    }
  if (this->StashedScalars)
    {
    this->StashedScalars->Delete();
    }
  if (this->MultiThreader)
    {
    this->MultiThreader->Delete();
    }
  if (this->Compressor)
    {
    this->Compressor->Delete();
    }
}

//----------------------------------------------------------------------------
static void *vtkImageStash_ThreadFunction( vtkMultiThreader::ThreadInfo *genericData )
{
  ThreadInfoStruct *info = static_cast<ThreadInfoStruct*>(genericData);
  vtkImageStash *self = static_cast<vtkImageStash *>(info->UserData);
  self->Stash();
  self->SetStashing(0);
  return nullptr;
}

//----------------------------------------------------------------------------
void vtkImageStash::ThreadedStash()
{
  this->SetStashing(1);
  this->StashingThreadID = this->MultiThreader->SpawnThread(
                                        (vtkThreadFunctionType) &vtkImageStash_ThreadFunction,
                                        static_cast<void *>(this));
}


//----------------------------------------------------------------------------
void vtkImageStash::Stash()
{
  //
  // put a compressed version of the scalars into the compressed
  // buffer, and then set the scalar size to zero
  //
  if (!this->GetStashImage())
    {
    vtkErrorWithObjectMacro (this, "Cannot stash - no image data");
    return;
    }

  vtkDataArray *scalars = this->GetStashImage()->GetPointData()->GetScalars();
  if (!scalars)
    {
    vtkErrorWithObjectMacro (this, "Cannot stash - image has no scalars");
    return;
    }

  this->SetNumberOfTuples(scalars->GetNumberOfTuples());
  vtkIdType numPrims = this->GetNumberOfTuples() * scalars->GetNumberOfComponents();
  vtkIdType size = vtkDataArray::GetDataTypeSize(scalars->GetDataType());
  vtkIdType scalarSize = size * numPrims;

  unsigned char *p = static_cast<unsigned char *>(scalars->WriteVoidPointer(0, numPrims));
  this->GetCompressor()->SetCompressionLevel(this->GetCompressionLevel());
  vtkUnsignedCharArray* compressedBuffer=this->GetCompressor()->Compress(p, scalarSize); // returns a new buffer that has to be deleted
  // The compressor allocates space that has the size of an uncompressed volume
  // and even if it uses less memory the buffer size is not reduced.
  // Call squeeze on the buffer to reclaim the unused memory space
  // (typically reduces memory consumption from hundreds of megabytes to under a megabyte)
  if (compressedBuffer)
    {
    compressedBuffer->Squeeze();
    this->SetStashedScalars(compressedBuffer);
    compressedBuffer->Delete();

    // this will realloc a zero sized buffer
    scalars->SetNumberOfTuples(0);
    scalars->Squeeze();
    this->SetStashingSucceeded(1);
    }
  else
    {
    // couldn't really compress
    this->SetStashedScalars(nullptr);
    this->SetStashingSucceeded(0);
    }
}

//----------------------------------------------------------------------------
void vtkImageStash::Unstash()
{
  //
  // put the decompressed values back into the scalar array
  //  - note: image data cannot have been touched since calling Stash
  //
  if (!this->StashImage)
    {
    vtkErrorMacro ("Cannot unstash - no image data");
    return;
    }

  if (this->GetStashing())
    {
    vtkErrorMacro ("Cannot unstash - stashing is still underway in a thread");
    return;
    }

  vtkDataArray *scalars = this->StashImage->GetPointData()->GetScalars();
  if (!scalars)
    {
    vtkErrorMacro ("Cannot unstash - image has no scalars");
    return;
    }

  if (!this->StashedScalars)
    {
    vtkErrorMacro ("Cannot unstash - nothing in the stash");
    return;
    }

  // we saved the original number of tuples before squeezing
  //   - the number of components and the datatype are unchanged from before
  //     so we know the right size for the output buffer
  vtkIdType numPrims = this->GetNumberOfTuples() * scalars->GetNumberOfComponents();
  vtkIdType size = vtkDataArray::GetDataTypeSize(scalars->GetDataType());
  vtkIdType scalarSize = size * numPrims;

  // setting the number of tuples reallocates the right amount of data
  // so we can uncompress directly into the buffer
  scalars->SetNumberOfTuples(this->GetNumberOfTuples());
  vtkIdType stashedSize = this->StashedScalars->GetNumberOfTuples();
  unsigned char *scalar_p =
      static_cast<unsigned char *>(scalars->WriteVoidPointer(0, numPrims));
  unsigned char *stash_p =
      static_cast<unsigned char *>(this->StashedScalars->WriteVoidPointer(0, stashedSize));
  this->GetCompressor()->Uncompress(stash_p, stashedSize, scalar_p, scalarSize);
}

//----------------------------------------------------------------------------
void vtkImageStash::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "StashImage: " << this->GetStashImage() << "\n";
  os << indent << "Stashing: " << this->GetStashing() << "\n";
  os << indent << "Stashed Scalars: " << this->GetStashedScalars() << "\n";
  if ( this->GetStashedScalars()) this->GetStashedScalars()->PrintSelf(os,indent.GetNextIndent());
  os << indent << "CompressionLevel: " << this->GetCompressionLevel() << "\n";
  os << indent << "Compressor: \n";
  this->GetCompressor()->PrintSelf(os,indent.GetNextIndent());
}

