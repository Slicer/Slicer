/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkIndirectLookupTable.cxx,v $
  Date:      $Date: 2006/09/22 18:43:46 $
  Version:   $Revision: 1.27 $

=========================================================================auto=*/
#include "vtkIndirectLookupTable.h"

#include "vtkObjectFactory.h"
#include "vtkUnsignedShortArray.h"
#include "vtkLookupTable.h"

#include <math.h>

vtkCxxSetObjectMacro(vtkIndirectLookupTable,LookupTable,vtkLookupTable);

//------------------------------------------------------------------------------
vtkIndirectLookupTable* vtkIndirectLookupTable::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkIndirectLookupTable");
  if(ret)
    {
    return (vtkIndirectLookupTable*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkIndirectLookupTable;
}

// Construct
//----------------------------------------------------------------------------
vtkIndirectLookupTable::vtkIndirectLookupTable()
{
  this->WinLvlMap = vtkUnsignedShortArray::New();
  this->DirectMap = vtkUnsignedShortArray::New();
  this->Map = vtkUnsignedShortArray::New();

  this->MapRange[0] = VTK_SHORT_MIN;
  this->MapRange[1] = VTK_SHORT_MAX;
  this->SetMapRange(this->MapRange[0], this->MapRange[1]);

  this->Window = 256;
  this->Level = 128;
  this->LowerThreshold = VTK_SHORT_MIN;
  this->UpperThreshold = VTK_SHORT_MAX;
  this->ApplyThreshold = 1;

  this->inLoPrev = 0;
  this->inHiPrev = 0;
  this->outLoPrev = 0;
  this->outHiPrev = 0;

  this->Direct = 0;
  this->DirectDefaultIndex = 1;

  this->FMRIMapping = 0;

  this->LookupTable = NULL;
}

//----------------------------------------------------------------------------
vtkIndirectLookupTable::~vtkIndirectLookupTable()
{
  // Delete allocated arrays
  this->WinLvlMap->Delete();
  this->DirectMap->Delete();
  this->Map->Delete();

  // Signal that we're no longer using it
  if (this->LookupTable != NULL)
    {
    this->LookupTable->UnRegister(this);
    }
}


//----------------------------------------------------------------------------
void vtkIndirectLookupTable::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Range:         " << this->Range[0] << " thru " << this->Range[1] << "\n";
  os << indent << "Window:        " << this->Window    << "\n";
  os << indent << "Level:         " << this->Level     << "\n";
  os << indent << "LowerThreshold:" << this->LowerThreshold << "\n";
  os << indent << "UpperThreshold:" << this->UpperThreshold << "\n";
  os << indent << "ApplyThreshold:" << this->ApplyThreshold << "\n";
  os << indent << "Direct:        " << this->Direct    << "\n";
  os << indent << "inLoPrev       " << this->inLoPrev    << "\n";
  os << indent << "inHiPrev       " << this->inHiPrev    << "\n";
  os << indent << "outHiPrev      " << this->outHiPrev    << "\n";
  os << indent << "outLoPrev      " << this->outLoPrev    << "\n";
  os << indent << "Build Time:    " << this->BuildTime.GetMTime() << "\n";

  if (this->LookupTable)
    {
    this->LookupTable->PrintSelf(os,indent.GetNextIndent());
    }
}

//----------------------------------------------------------------------------
// For now, don't allow users to alter the MapRange
void vtkIndirectLookupTable::SetMapRange(long /*min*/, long /*max*/)
{
  this->MapRange[0] = VTK_SHORT_MIN;
  this->MapRange[1] = VTK_SHORT_MAX;
  this->Range[0] = (vtkFloatingPointType)this->MapRange[0];
  this->Range[1] = (vtkFloatingPointType)this->MapRange[1];
  this->MapOffset = -VTK_SHORT_MIN;

  // Allocate arrays
  this->Map->Allocate(65536);
  this->DirectMap->Allocate(65536);
  this->WinLvlMap->Allocate(65536);
}

// Init to the default (Note: multiply index by 4 for rgba).
//------------------------------------------------------------------------------
void vtkIndirectLookupTable::InitDirect()
{
  long i;
  unsigned short idx = this->DirectDefaultIndex * 4;
  unsigned short *map = this->DirectMap->GetPointer(0);

  for (i=this->MapRange[0]; i <= this->MapRange[1]; i++)
    {
    map[this->MapOffset + i] = idx;
    }
}

//------------------------------------------------------------------------------
void vtkIndirectLookupTable::MapDirect(vtkFloatingPointType scalar, int index)
{
  unsigned short *map = this->DirectMap->GetPointer(0);

  // Boundary check
  if (scalar < this->Range[0])
    {
    scalar = this->Range[0];
    }
  else if (scalar > this->Range[1])
    {
    scalar = this->Range[1];
    }
  long s = (long)scalar;

  // Map it
  map[this->MapOffset + s] = (index * 4);
}

// Consider the Lookup Table when getting the modified time
// Kilian - This M function does not do anything good, because LookupTable->Build 
// will not do anything. This is due to the way how this->Build is written
// (it uses the function LookupTable->SetTableValue which disables LookupTable->Build)
// Therefore to update a LookupTable you have to delete it and recreate it and assign it to this!  
//----------------------------------------------------------------------------
unsigned long vtkIndirectLookupTable::GetMTime()
{
  unsigned long t1, t2;

  t1 = this->vtkScalarsToColors::GetMTime();
  if (this->LookupTable)
  {
    t2 = this->LookupTable->GetMTime();
    if (t2 > t1)
    {
      t1 = t2;
    }
  }
  return t1;
}

//----------------------------------------------------------------------------
void vtkIndirectLookupTable::WindowLevel()
{
  double delta;
  short outLo, outHi, window, level;
  short i, inLo, inHi, inFirst, inLast;
  short min = this->MapRange[0];
  short max = this->MapRange[1];
  long offset = this->MapOffset;
  unsigned short *map = this->WinLvlMap->GetPointer(0);

  window = (short)this->Window;
  level  = (short)this->Level;

  // The range of input scalar values that will be mapped is [inLo, inHi]
  inLo = level - window / 2;
  inHi = inLo + window - 1;
  if (inHi <= inLo)
    {
    inHi = inLo + 1;
    }

  // The scalars are mapped to indices into the LookupTable.
  // These indices are in the range [outLo, outHi].
  // Assumes the first entry in the LookupTable is for thresholding.
  //
  outLo = 1;
  outHi = this->LookupTable->GetNumberOfColors() - 1;

  // Only update the portion of the map that changes since the last
  // time we ran this routine.  The range between [inLo, inHi] always
  // gets updated.  If inLo is greater than the previous inLo, then update
  // the portion between them.  If inHi is less than the previous inHi,
  // then update the portion between them.  If outHi changed since last time,
  // then update from inHi through the end of the map.
  // Update the portion from 'inFirst' through 'inLast', inclusive.
  //
  if (this->outLoPrev == outLo)
    {
    inFirst = this->inLoPrev;
    }
  else
    {
    inFirst = min;
    }

  if (this->outHiPrev == outHi)
    {
    inLast = this->inHiPrev;
    }
  else
    {
    inLast = max;
    }

  // Only update he portion of the map between inLo and inHi, inclusive.
  // Store for next time.
  //
  this->inLoPrev  = inLo;
  this->inHiPrev  = inHi;
  this->outLoPrev = outLo;
  this->outHiPrev = outHi;

  // Delta is the step in output corresponding to each step in input
  //
  delta = (double)(outHi - outLo + 1) / (double)(inHi - inLo + 1);

  // Integers between 'inLo' and 'inHi' are mapped linearly into the
  // range between 'outLo' and 'outHi'
  //
  for (i = inLo; i < inHi; i++)
    {
    map[offset + i] =
      (unsigned short)(outLo + delta * (double) (i - inLo)) << 2;
    }

  // Save time by shifting now instead of later
  outLo = outLo << 2;
  outHi = outHi << 2;

  // All integer values below 'inLo' are mapped to 'outLo'
  //
  for (i = inFirst; i <= inLo; i++)
    {
    map[offset + i] = outLo;
    }

  // All integer values above 'inHi' are mapped to 'outHi'
  // We have to stop before 'max', or i rolls over.
  //
  for (i = inHi; i < inLast; i++)
    {
    map[offset + i] = outHi;
    }

  // Need this line because if inLast=32767, then i rolls over to -32768
  // if the for loop says (i <= inLast)
  //
  if (inLast > inHi)
    {
    map[offset + inLast] = outHi;
    }

  // We have to do i=inHi in chase iLast < inHi
  //
  map[offset + inHi] = outHi;
}


// Generate lookup table from window and level.
// Table is built as a linear ramp, centered at Level and of width Window.
//----------------------------------------------------------------------------
void vtkIndirectLookupTable::Build()
{
  int numColors;
  short min   = this->MapRange[0];
  short max   = this->MapRange[1];
  long offset = this->MapOffset;
  short lower = (short)this->GetLowerThreshold();
  short upper = (short)this->GetUpperThreshold();
  unsigned short *map       = this->Map->GetPointer(0);
  unsigned short *directMap = this->DirectMap->GetPointer(0);
  unsigned short *winLvlMap = this->WinLvlMap->GetPointer(0);

  // If no changes since last build, then do nothing.
  if (this->BuildTime > this->GetMTime())
    {
    return;
    }

  // If there is no lookup table, create one
  // 256 bins grayscale.
  if (this->LookupTable == NULL)
    {
    this->LookupTable = vtkLookupTable::New();
    this->LookupTable->Register(this);
    this->LookupTable->Delete();
    this->LookupTable->SetNumberOfColors(256);
    this->LookupTable->SetSaturationRange(0, 0);
    this->LookupTable->SetValueRange(0, 1);
    this->LookupTable->SetRampToLinear();
    this->LookupTable->Build();
    }

  // Set the first color to be transparent for use when a pixel is
  // outside the threshold

  // Kilian - This Function disables LookupTable->Build()
  // Therefore to update a LookupTable you have to delete it, recreate it and assign it to this !  
  this->LookupTable->SetTableValue(0, 0, 0, 0, 0);

  // Check that the LookupTable has the right number of colors
  numColors = this->LookupTable->GetNumberOfColors();
  if (numColors > (65536 >> 2))
    {
    vtkErrorMacro(<< "Build: LookupTable has too many colors.");
    return;
    }
  if (numColors < 2)
    {
    vtkErrorMacro(<< "Build: LookupTable needs at least 2 colors.");
    return;
    }

  // Perform thresholding by overwriting the map with the index to the
  // transparent color for every pixel value outside the threshold.
  // The transparent color is always at index 0.
  // Use the 'direct' for direct mode, and the 'map' otherwise.
  //
  if (this->Direct)
    {
    memcpy(map, directMap, (max-min+1)*sizeof(short));
    }
  else
    {
    this->WindowLevel();
    memcpy(map, winLvlMap, (max-min+1)*sizeof(short));
    }

  // Apply threshold if desired
  if (this->GetApplyThreshold())
    {
    // Special thresholding for t volume
    if (this->GetFMRIMapping())
      {
      // "AND" threshold
      //
      // |-------|-------|------|------|--------|-------|
      // Min    -h      -l      0      +l      +h       Max
      // show values between -h and -l, and
      //      values between +l and +h
      // zero out values less than -h, and
      //      values between -l and +l, and
      //      values above +h
      if (lower <= upper)
        {
        short high = abs(upper);

        // Zero out values above +h
        memset(&(map[offset+high+1]), 0, (max-high)*sizeof(short));

        // Zero out values less than -h
        long width = (-upper)-min;
        width = (width < 0 ? 1 : width);
        memset(map, 0, width*sizeof(short));

        // Zero values between -l and +l
        if (lower != 0)
          {
          long w = abs(lower)*2;
          long start = offset-abs(lower)+1;

          if (start < 0)
            {
            start = 0;
            }
          memset(&(map[start]), 0, (w-1)*sizeof(short));
          }
        }
      // Zero out everything
      else
        {
        memset(map, 0, (max-min+1)*sizeof(short));
        }
      }
    else
      {
      // "AND" threshold
      // show values >= 'lower' AND <= 'upper'
      // zero values <  'lower' OR  >  'upper'
      if (lower <= upper)
        {
        // Zero out what's < 'lower'
        memset(map, 0, (lower-min)*sizeof(short));

        // Zero out what's > 'above'
        memset(&(map[offset+upper+1]), 0, (max-upper)*sizeof(short));
        }
      // "OR" threshold
      // show values <  'upper' OR  > 'lower'
      // zero values >= 'upper' AND <= 'lower'
      else
        {
        // Zero out what's >= 'upper' and <= 'lower'
        memset(&(map[offset+upper]), 0, (lower-upper+1)*sizeof(short));
        }
      }
    }

  this->BuildTime.Modified();
}

//----------------------------------------------------------------------------
// get the color for a scalar value
void vtkIndirectLookupTable::GetColor(vtkFloatingPointType v, vtkFloatingPointType rgb[3])
{
  unsigned char *rgb8 = this->MapValue(v);

  rgb[0] = rgb8[0]/255.0;
  rgb[1] = rgb8[1]/255.0;
  rgb[2] = rgb8[2]/255.0;
}

//----------------------------------------------------------------------------
// Given a scalar value v, return an RGBA color value from LookupTable
unsigned char *vtkIndirectLookupTable::MapValue(vtkFloatingPointType v)
{
  unsigned short *map = this->Map->GetPointer(0);
  unsigned char *lut = this->LookupTable->GetPointer(0);

  // Boundary check
  if (v < this->Range[0])
    {
    v = this->Range[0];
    }
  else if (v > this->Range[1])
    {
    v = this->Range[1];
    }

  // Map it
  return &lut[map[this->MapOffset + (long)v]];
}

//----------------------------------------------------------------------------
// This version of the "MapData" function works around lots of the premature
// efficiency hacks in the rest of the code in order to provide a correct
// result for floating point data.
  
template<class T>
static void vtkIndirectLookupTableMapDataExact(vtkIndirectLookupTable *self,
  T *input, unsigned char *output,int length, int incr)
{
  int i;
  int index;
  double inval;

  vtkFloatingPointType contrastWindow = self->GetWindow();
  vtkFloatingPointType contrastLevel = self->GetLevel();
  unsigned char *lut = self->GetLookupTable()->GetPointer(0);
  int numberOfColors = self->GetLookupTable()->GetNumberOfColors();
  vtkFloatingPointType lowerThresh = self->GetLowerThreshold();
  vtkFloatingPointType upperThresh = self->GetUpperThreshold();
  vtkFloatingPointType lowerRange = contrastLevel - contrastWindow/2.0;
  vtkFloatingPointType upperRange = contrastLevel + contrastWindow/2.0;
  int applyThreshold = self->GetApplyThreshold();

  // delta is -2: 1 is for a fencepost, second is because the first pixel
  // in the lookup table is a special transparent one.
  vtkFloatingPointType delta = (double)(numberOfColors - 2) / contrastWindow;

  for(i = 0; i < length; i++) 
    {
    inval = *input;
    if (applyThreshold && (inval <= lowerThresh || inval >= upperThresh)) 
      {
      output[0] = lut[0];
      output[1] = lut[1];
      output[2] = lut[2];
      output[3] = lut[3];
      }
    else 
      {
      if (inval < lowerRange)
        {
        inval = lowerRange;
        }
      else if (inval > upperRange)
        {
        inval = upperRange;
        }
      
      // Offset by 1 because the lut has transparent pixel for first entry.
      // Add 0.5 for proper rounding to int (we'll always be above zero,
      // so truncation rules apply).
      index = (int)((inval - lowerRange)*delta + 0.5) + 1;
      index *= 4;
      output[0] = lut[index];
      output[1] = lut[index+1];
      output[2] = lut[index+2];
      output[3] = lut[index+3];
      }
    output += 4;
    input += incr;
    }
}

//----------------------------------------------------------------------------
// accelerate the mapping by copying the data in 32-bit chunks instead
// of 8-bit chunks
template<class T>
static void vtkIndirectLookupTableMapData(vtkIndirectLookupTable *self,
  T *input, unsigned char *output,int length, int incr)
{
  int i;
  vtkFloatingPointType *range = self->GetRange();
  long offset = self->GetMapOffset();
  unsigned char *lut = self->GetLookupTable()->GetPointer(0);
  unsigned short *map = self->GetMap()->GetPointer(0);

  // DAVE do this safer
  //
  // Karl Krissian: pb with unsigned ...
  // range is -32768, 32767, can not be converted to unsigned short for example ...
  //
  T v;
  long min = (long)range[0];
  long max = (long)range[1];

  for (i=0; i<length; i++)
    {
    v = *input;

    // Boundary check
    if ((long)v < min)
      {
      v = min;
      }
    else if ((long)v > max)
      {
      v = max;
      }
    long s = (long)v;

    // Map s to RGBA
    int index;

    // For VTK_FLOAT or VTK_DOUBLE, we need handle the following case:
    // For instance, we set 2.3 for the lower threshold. All values less than 2.3, 
    // including 2.1 and 2.2, must be thresholded out. 2.1 and 2.2 are special cases
    // here since 2.1, 2.2 and 2.3 all become 2 if we cast them into long, int or short.
    // The following statment will guarantee the removal of 2.1 and 2.2.
    if (v < self->GetLowerThreshold())
      {
      index = offset;
      }
    else
      {
      index = offset + s;
      }

    memcpy(output, &lut[map[index]], 4);

    output += 4;
    input += incr;
    }
}


static void vtkIndirectLookupTableMapShortData(vtkIndirectLookupTable *self,
  short *input, unsigned char *output, int length, int incr)
{
  int i;
  long offset = self->GetMapOffset();
  unsigned char *lut = self->GetLookupTable()->GetPointer(0);
  unsigned short *map = self->GetMap()->GetPointer(0);

  for (i=0; i<length; i++)
    {
    memcpy(output, &lut[map[offset + *input]], 4);
    output += 4;
    input += incr;
    }
}


static void vtkIndirectLookupTableMapFMRIData(vtkIndirectLookupTable *self, float *input, 
                                              unsigned char *output, int length, int incr)
{
  int i;
  vtkFloatingPointType *range = self->GetRange();
  long offset = self->GetMapOffset();
  unsigned char *lut = self->GetLookupTable()->GetPointer(0);
  unsigned short *map = self->GetMap()->GetPointer(0);

  long min = (long)range[0];
  long max = (long)range[1];

  float v;
  for (i = 0; i < length; i++)
    {
    v = *input;

    // Boundary check
    if ((long)v < min)
      {
      v = min;
      }
    else if ((long)v > max)
      {
      v = max;
      }
    long s = (long)v;

    // Map s to RGBA
    int index;

    float low = fabs(self->GetLowerThreshold());
    float high = fabs(self->GetUpperThreshold());
    if ((v > 0 && v < low)          ||    // low = 2.5, v = 2.1, 2.3, or 2.4
        (v < 0 && v > (-low))       ||    // low = -2.5, v = -2.1, -2.3, or -2.4
        (v > 0 && v > high)         ||    // high = 5.1, v = 5.2
        (v < 0 && v < (-high)))           // high = -5.1, v = -5.2
      {
      index = offset;
      }
    else
      {
      index = offset + s;
      }

    memcpy(output, &lut[map[index]], 4);

    output += 4;
    input += incr;
    }
}


void vtkIndirectLookupTable::MapScalarsThroughTable2(void *input, unsigned char *output,
                                                     int inputDataType, int numberOfValues,
                                                     int inputIncrement, int /*outputFormat*/)
{
  switch (inputDataType)
    {
  case VTK_CHAR:
    vtkIndirectLookupTableMapData(this, (char *)input, output,
      numberOfValues, inputIncrement);
    break;

  case VTK_UNSIGNED_CHAR:
    vtkIndirectLookupTableMapData(this, (unsigned char *)input, output,
      numberOfValues, inputIncrement);
    break;

  case VTK_SHORT:
    vtkIndirectLookupTableMapShortData(this, (short *)input, output,
      numberOfValues, inputIncrement);
    break;

  case VTK_UNSIGNED_SHORT:
    vtkIndirectLookupTableMapData(this, (unsigned short *)input, output,
      numberOfValues, inputIncrement);
    break;

  case VTK_INT:
    vtkIndirectLookupTableMapData(this, (int *)input, output,
      numberOfValues, inputIncrement);
    break;

  case VTK_UNSIGNED_INT:
    vtkIndirectLookupTableMapData(this, (unsigned int *)input, output,
      numberOfValues, inputIncrement);
    break;

  case VTK_LONG:
    vtkIndirectLookupTableMapData(this, (long *)input, output,
      numberOfValues, inputIncrement);
    break;

  case VTK_UNSIGNED_LONG:
    vtkIndirectLookupTableMapData(this, (unsigned long *)input, output,
      numberOfValues, inputIncrement);
    break;

  case VTK_FLOAT:
    if (this->GetFMRIMapping())
      {
      vtkIndirectLookupTableMapFMRIData(this, (float *)input, output,
        numberOfValues, inputIncrement);
      }
    else
      {
      vtkIndirectLookupTableMapDataExact(this,(float *)input, output,
        numberOfValues, inputIncrement);
      }
    break;

  case VTK_DOUBLE:
    vtkIndirectLookupTableMapDataExact(this, (double *)input, output,
      numberOfValues, inputIncrement);
    break;

  default:
    vtkErrorMacro(<< "MapScalarsThroughTable2: Unknown input ScalarType");
    return;
    }
}
#if (VTK_MAJOR_VERSION == 5 && VTK_MINOR_VERSION == 8)
//----------------------------------------------------------------------------
vtkIdType vtkIndirectLookupTable::GetNumberOfAvailableColors()
{
  return this->LookupTable->GetNumberOfAvailableColors();
}
#endif
