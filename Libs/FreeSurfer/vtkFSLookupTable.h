/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkFSLookupTable.h,v $
  Date:      $Date: 2006/02/28 20:50:07 $
  Version:   $Revision: 1.4 $

=========================================================================auto=*/
// .NAME vtkFSLookupTable - A look up table for FreeSurfer colour scales.
// .SECTION Description
// Reimplements vtkLookupTable to provide custom mapping of scalars to colours.
//

#ifndef __vtkFSLookupTable_h
#define __vtkFSLookupTable_h

#include <FreeSurferConfigure.h>
#include "vtkFreeSurferWin32Header.h"
#include "vtkLookupTable.h"
class VTK_FreeSurfer_EXPORT vtkFSLookupTable : public vtkLookupTable
{
public:
    static vtkFSLookupTable *New();
    vtkTypeMacro(vtkFSLookupTable,vtkLookupTable);
    void PrintSelf(ostream& os, vtkIndent indent);

    

    vtkGetMacro(LowThresh,float);
    vtkSetMacro(LowThresh,float);

    vtkGetMacro(HiThresh,float);
    vtkSetMacro(HiThresh,float);

    vtkGetMacro(LutType,int);
    // Description:
    // Map from integer type to a string description
    const char *GetLutTypeString();
    // Description:
    // reset all the values in case one was changed, so the next map of
    // scalars to colours will use the new values
    void SetLutTypeToLabels();
    void SetLutTypeToHeat();
    void SetLutTypeToBlueRed();
    void SetLutTypeToRedBlue();
    void SetLutTypeToRedGreen();
    void SetLutTypeToGreenRed();
        
    vtkGetMacro(Reverse,int);
    vtkSetMacro(Reverse,int);
    vtkBooleanMacro(Reverse,int);
    
    vtkGetMacro(Truncate,int);
    vtkSetMacro(Truncate,int);
    vtkBooleanMacro(Truncate,int);
    
    vtkGetMacro(Offset,float);
    vtkSetMacro(Offset,float);

    vtkGetMacro(Slope,float);
    vtkSetMacro(Slope,float);

    vtkGetMacro(Blufact,float);
    vtkSetMacro(Blufact,float);

    vtkGetMacro(FMid,float);
    vtkSetMacro(FMid,float);

    // Description:
    // from vtkScalarsToColors
    double *GetRange();
    // Description:
    // don't do anything as it's overriding the LUTs low threshold with the
    // scalar values' lowest value
    void SetRange(double, double);
    // Description:
    // Given a scalar value val, return an rgba color value
    // returns array of length 3, 0-255
    unsigned char *MapValue(double val);
    //BTX - begin tcl exclude
    // Description:
    // passes val to MapValue
    void GetColor(double, double[3]);
    //ETX - end tcl exclude
    // Description:
    // take input scalars and push them through the calculation to get colours
    // to put int the output array
    void MapScalarsThroughTable2(void* input, unsigned char* outupt, int inputDataType, int numberOfValues, int inputIncrement, int outputIncrement);

    //BTX
    // Description:
    // Type constant, can have different types of colour scales
    enum
    {
      FSLUTLABELS = 0,
      FSLUTHEAT = 1,
      FSLUTBLUERED = 2,
      FSLUTREDBLUE = 3,
      FSLUTREDGREEN = 4,
      FSLUTGREENRED = 5,
    };
    //ETX
protected:
    vtkFSLookupTable();
    ~vtkFSLookupTable();

    // Description:
    // Low cut off, values passed in that are below this level are mapped to
    // low cut off colour
    // FS cmin and fthresh
    float LowThresh;
    // Description:
    // High cut off, values passed in that are above this value are mapped to
    // high cut off colour
    // FS cmax
    float HiThresh;

    // Description:
    // which type of colour scale this is supposed to model, from defined
    // constants above
    int LutType;

    // Description:
    // is the colour scale reversed?
    // from FS invphaseflag
    int Reverse;

    // Description:
    // truncate the values below zero?
    // from FS truncphaseflag
    int Truncate;

    // Description:
    // offset value added to the input value - a brightening factor
    float Offset;

    // Description:
    // slope of the function that maps from input values to colours
    float Slope;

    // Description:
    // blue factor?
    float Blufact;

    // Description:
    // Midpoint of the function that maps from input values to colours
    // FS fmid and cmid
    float FMid;

    // Description:
    // output of colour computation
    unsigned char RGBA[4];
};

#endif
    
