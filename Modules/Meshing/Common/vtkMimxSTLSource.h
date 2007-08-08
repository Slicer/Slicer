/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxSTLSource.h,v $
Language:  C++
Date:      $Date: 2007/07/12 14:15:21 $
Version:   $Revision: 1.9 $

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

#ifndef __vtkMimxSTLSource_h
#define __vtkMimxSTLSource_h

#include "vtkCommon.h"

#include "vtkPolyDataSource.h"
#include "vtkSTLReader.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "vtkProperty.h"
#include "vtkScalarBarActor.h"
#include "vtkSphereSource.h"
#include "vtkDoubleArray.h"
//  class for storing polydata generated from vtkSTLReader
class VTK_MIMXCOMMON_EXPORT vtkMimxSTLSource : public vtkPolyDataSource
{
public:
  vtkTypeRevisionMacro(vtkMimxSTLSource,vtkPolyDataSource);

//  void PrintSelf(ostream& os, vtkIndent indent);
  static vtkMimxSTLSource *New();
  void Execute();
  vtkMimxSTLSource();
  ~vtkMimxSTLSource();
  //  should look into eliminating this function
  void ReadSTLFile();
  vtkActor* GetActor();
  vtkPolyDataMapper* GetPolyDataMapper();
  //  to setup look up table for closest point distance map
  void ClosestPointDistanceMap(vtkMimxSTLSource*);
  void ClosestPointDistanceMapMod(vtkMimxSTLSource*,double);
  void GenerateLookupTableMod(vtkScalarBarActor*,double);
  void ClosestPointDistanceMapGrayScale(vtkMimxSTLSource*);
  void ClosestPointDistanceMapModGrayScale(vtkMimxSTLSource*,double);
  void GenerateLookupTableModGrayScale(vtkScalarBarActor*,double);

  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);
  void Copy(vtkPolyData*);
  vtkPolyDataMapper* PolyDataMapper;
  vtkPolyData* PolyData;
  vtkPolyData* GetPolyData();
  void SetPrevColor(double color[3]);
  void GetPrevColor(double color[3]);
  void Translate(double, double, double);
  void Rotate(double [], double, int);
  void Scale(double [], double);
private:
  vtkActor* Actor;
  char *FileName;
  vtkDoubleArray* DoubleArray;
protected:
  //vtkSTLSource();
  vtkSTLReader* STL;
  double PrevColor[3];
};

#endif
