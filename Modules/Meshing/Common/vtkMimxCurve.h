/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxCurve.h,v $
Language:  C++
Date:      $Date: 2007/07/12 14:15:21 $
Version:   $Revision: 1.7 $

 Musculoskeletal Imaging, Modelling and Experimentation (MIMX)
 Center for Computer Aided Design
 The University of Iowa
 Iowa City, IA 52242
 http://www.ccad.uiowa.edu/mimx/
 
Copyright (c) The University of Iowa. All rights reserved.
See MIMXCopyright.txt or http://www.ccad.uiowa.edu/mimx/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even 
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef _vtkMimxCurve_h
#define _vtkMimxCurve_h

#include "vtkCommon.h"

#include "vtkPolyDataSource.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"
#include "vtkPolyData.h"
//  class for storing polydata generated from Tracing
class VTK_MIMXCOMMON_EXPORT vtkMimxCurve : public vtkPolyDataSource
{
public:
  vtkTypeRevisionMacro(vtkMimxCurve,vtkPolyDataSource);

//  void PrintSelf(ostream& os, vtkIndent indent);
  static vtkMimxCurve *New();
  void Execute();
  vtkMimxCurve();
  ~vtkMimxCurve();
  vtkActor* GetActor();
  vtkPolyDataMapper* GetPolyDataMapper();
  vtkPoints* GetPoints();
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);
  vtkPolyData* PolyData;
  void SetPrevColor(double color[3]);
  void GetPrevColor(double color[3]);
  void Translate(double X, double Y, double Z);
  void Scale(double Center[3], double ScaleFactor);
  void CalculateCenter(double Center[3]);
private:
  vtkPolyDataMapper* PolyDataMapper;
  vtkActor* Actor;
  char *FileName;
  vtkPoints* Points;
  vtkCellArray* CellArray;
  double PrevColor[3];
protected:
};

#endif
