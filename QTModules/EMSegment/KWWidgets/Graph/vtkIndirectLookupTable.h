/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkIndirectLookupTable.h,v $
  Date:      $Date: 2006/04/13 19:25:39 $
  Version:   $Revision: 1.24 $

=========================================================================auto=*/
// .NAME vtkIndirectLookupTable - indirectly map scalar values into colors.
// .SECTION Description
// vtkIndirectLookupTable is an object that is used by mapper objects.
// Normally, a mapper maps scalar values to colors by using a
// vtkLookupTable object.  This object offers an alternative that can
// execute 2.4 times faster.  Mapping is performed indirectly by first
// constructing an internal, intermediate lookup table that maps each
// possible scalar value to an index into the externally specified
// LookupTable.
//
// This object uses the first color in the LookupTable as the color to use for
// data that fails to satisfy the thresholds.  Therefore, it overwrites this
// first color with transparent black during the Build procedure.
//
// DAVE: This only maps to RGBA, but it should have all the options of a
// vtkScalarsToColors class.
//
// .SECTION See Also
// vtkLookupTable

#ifndef __vtkIndirectLookupTable_h
#define __vtkIndirectLookupTable_h

#include "vtkScalarsToColors.h"
#include "vtkEMSegmentStep.h"


class vtkLookupTable;
class vtkUnsignedShortArray;
class VTK_EMSEGMENT_EXPORT vtkIndirectLookupTable : public vtkScalarsToColors
{
public:
  static vtkIndirectLookupTable *New();
  vtkTypeMacro(vtkIndirectLookupTable,vtkScalarsToColors);
  void PrintSelf(ostream& os, vtkIndent indent);

  /// Description:
  /// Generate lookup table from window and level or direct.
  /// Table is built as a linear ramp, centered at Level and of width Window.
  void Build();

  /// Description:
  /// Set the Window for the lookuptable. Window is the width of the
  /// lookuptable ramp.
  vtkSetClampMacro(Window,vtkFloatingPointType,1.0,65536.0);
  vtkGetMacro(Window,vtkFloatingPointType);

  /// Description:
  /// Set the Level for the lookuptable. Level is the center of the ramp of
  /// the lookuptable.  ramp.
  vtkSetMacro(Level,vtkFloatingPointType);
  vtkGetMacro(Level,vtkFloatingPointType);

  /// Description:
  /// Set/get the threshold
  vtkBooleanMacro(ApplyThreshold, int);
  vtkSetMacro(ApplyThreshold,int);
  vtkGetMacro(ApplyThreshold,int);
  vtkSetMacro(LowerThreshold,vtkFloatingPointType);
  vtkGetMacro(LowerThreshold,vtkFloatingPointType);
  vtkSetMacro(UpperThreshold,vtkFloatingPointType);
  vtkGetMacro(UpperThreshold,vtkFloatingPointType);

  /// Description:
  /// Sets/Gets the range of scalars which will be mapped.
  vtkFloatingPointType *GetRange() {return &this->Range[0];};
  void SetRange(vtkFloatingPointType min, vtkFloatingPointType max) {
    this->SetMapRange((long)min, (long)max);};
  void SetRange(vtkFloatingPointType rng[2]) {
    this->SetRange(rng[0],rng[1]);};
  vtkGetMacro(MapOffset,long);

  /// Description:
  /// Lookup table containing the RGBA colors that the input scalars will
  /// be indirectly mapped to
  virtual void SetLookupTable(vtkLookupTable*);
  vtkGetObjectMacro(LookupTable,vtkLookupTable);

  /// Description:
  /// Set the Default index into the LookupTable that will be used
  /// when the Direct map is initialized.
  vtkSetMacro(DirectDefaultIndex,long);
  vtkGetMacro(DirectDefaultIndex,long);

  /// Description:
  /// We need to check the modified time of the LookupTable too.
  unsigned long GetMTime();

  /// Description:
  /// The map for translating input scalar values to LookupTable indices
  /// can be either specified directly or by Window/Level.
  /// These functions set/get which method to use.
  vtkGetMacro(Direct, int);
  vtkSetMacro(Direct, int);
  vtkBooleanMacro(Direct, int);

  /// Description:
  /// Initialize the map to translate all input scalar values to
  /// the DirectDefaultIndex into the LookupTable
  void InitDirect();

#if (VTK_MAJOR_VERSION == 3 && VTK_MINOR_VERSION == 2)
  /// Description:
  /// Map one value through the lookup table and return the color as
  /// an RGB array of vtkFloatingPointTypes between 0 and 1.
  vtkFloatingPointType *GetColor(vtkFloatingPointType x) {
    return vtkScalarsToColors::GetColor(x); }
#endif

  void GetColor(vtkFloatingPointType x, vtkFloatingPointType rgb[3]);


  /// Description:
  /// Specifically set a certain input scalar value to map to a
  /// certain index into the LookupTable
  void MapDirect(vtkFloatingPointType scalar, int index);

  /// Description:
  /// Indirectly Map one value through the LookupTable
  virtual unsigned char *MapValue(vtkFloatingPointType v);

  /// Description:
  /// indirectly map a set of scalars through the LookupTable
  void MapScalarsThroughTable2(void *input, unsigned char *output,
    int inputDataType, int numberOfValues, int inputIncrement,
    int outputFormat);

  /// Description:
  /// The indirect LUT
  vtkGetObjectMacro(Map,vtkUnsignedShortArray);

  // Description:
  // Get or set FMRIMapping (branched mapping through the LookupTable)
  vtkGetMacro(FMRIMapping, int);
  vtkSetMacro(FMRIMapping, int);

//BTX
#if (VTK_MAJOR_VERSION == 5 && VTK_MINOR_VERSION == 8)
  virtual vtkIdType GetNumberOfAvailableColors();
#endif
//ETX

protected:
  vtkIndirectLookupTable();
  ~vtkIndirectLookupTable();

  vtkFloatingPointType Range[2];
  long MapRange[2];
  long MapOffset;
  void SetMapRange(long min, long max);

  vtkFloatingPointType Window;
  vtkFloatingPointType Level;
  vtkFloatingPointType UpperThreshold;
  vtkFloatingPointType LowerThreshold;
  int ApplyThreshold;

  short inHiPrev;
  short inLoPrev;
  short outHiPrev;
  short outLoPrev;
  void WindowLevel();

  int Direct;
  int DirectDefaultIndex;
  int FMRIMapping;

  vtkLookupTable *LookupTable;

  vtkUnsignedShortArray *WinLvlMap;
  vtkUnsignedShortArray *DirectMap;
  vtkUnsignedShortArray *Map;

  vtkTimeStamp BuildTime;

private:
  vtkIndirectLookupTable(const vtkIndirectLookupTable&);
  void operator=(const vtkIndirectLookupTable&);
};

#endif


