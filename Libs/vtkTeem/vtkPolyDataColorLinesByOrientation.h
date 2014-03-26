/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkPolyDataColorLinesByOrientation.h,v $

  Copyright (c) Demian Wassermann
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __vtkPolyDataColorLinesByOrientation_h
#define __vtkPolyDataColorLinesByOrientation_h

#include "vtkPolyDataAlgorithm.h"
#include "vtkTeemConfigure.h"

/// \brief Colors the lines in the polydata by their mean orientation.
class VTK_Teem_EXPORT vtkPolyDataColorLinesByOrientation : public vtkPolyDataAlgorithm
{
public:
  static vtkPolyDataColorLinesByOrientation *New();
  vtkTypeRevisionMacro(vtkPolyDataColorLinesByOrientation,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  ///
  /// Turn on/off extraction of scalars for color.
  vtkSetMacro(CopyOriginalData,int);
  vtkBooleanMacro(CopyOriginalData,int);
  vtkGetMacro(CopyOriginalData,int);


  vtkSetStringMacro(ScalarArrayName);
  vtkGetStringMacro(ScalarArrayName);


  //--------------------------------------------------------------------------
  /// Display Information: Color Mode
  /// 0) solid color by group 1) color by scalar invariant
  /// 2) color by avg scalar invariant 3) color by other
  //--------------------------------------------------------------------------

  enum
  {
    colorModePointFiberOrientation = 0,
    colorModeMeanFiberOrientation = 1,
  };

  //--------------------------------------------------------------------------
  /// Display Information: ColorMode
  //--------------------------------------------------------------------------

 /// Description:
  /// Color mode for the line points
  vtkGetMacro ( ColorMode, int );
  vtkSetMacro ( ColorMode, int );


protected:
  vtkPolyDataColorLinesByOrientation();
  ~vtkPolyDataColorLinesByOrientation();

  /// Usual data generation method
  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

  virtual int FillInputPortInformation(int port, vtkInformation *info);

  char* ScalarArrayName;
  int CopyOriginalData;
  int ColorMode;
private:
  vtkPolyDataColorLinesByOrientation(const vtkPolyDataColorLinesByOrientation&);  /// Not implemented.
  void operator=(const vtkPolyDataColorLinesByOrientation&);  /// Not implemented.
};

#endif
