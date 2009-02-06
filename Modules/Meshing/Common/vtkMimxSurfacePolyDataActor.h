/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxSurfacePolyDataActor.h,v $
Language:  C++
Date:      $Date: 2008/08/05 02:32:48 $
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

// .NAME vtkMimxSurfacePolyDataActor - a 3D non-orthogonal axes representation
// .SECTION Description
// vtkMimxSurfacePolyDataActor is the abstract base class for all the pipeline setup for
// different types of datatypes. Data types concidered are vtkPolyData,
// vtkStructuredGrid (both plane and solid) and vtkUnstructuredGrid.

#ifndef __vtkMimxSurfacePolyDataActor_h
#define __vtkMimxSurfacePolyDataActor_h

#include "vtkMimxActorBase.h"
#include "vtkMimxCommonWin32Header.h"

class vtkActor;
class vtkPolyData;
class vtkPolyDataMapper;

class VTK_MIMXCOMMON_EXPORT vtkMimxSurfacePolyDataActor : public vtkMimxActorBase
{
public:

  enum { 
    DisplaySurface                = 1,
    DisplayOutline                = 2,
    DisplaySurfaceAndOutline      = 3
  };
  
  static vtkMimxSurfacePolyDataActor *New();
  vtkTypeRevisionMacro(vtkMimxSurfacePolyDataActor,vtkMimxActorBase);
  void PrintSelf(ostream& os, vtkIndent indent);
  vtkPolyData* GetDataSet();
  void SetFillColor(double rgb[3]);
  void SetFillColor(double red, double green, double blue);
  void GetFillColor(double &red, double &green, double &blue);
  void GetFillColor(double rgb[3]);
  void SetOutlineColor(double rgb[3]);
  void SetOutlineColor(double red, double green, double blue);
  void GetOutlineColor(double &red, double &green, double &blue);
  void GetOutlineColor(double rgb[3]);
  void SetDisplayType( int mode );
  int  GetDisplayType();
// void SetDataType(int){};
 //vtkSetMacro(PolyData, vtkPolyData*);
 //vtkGetMacro(PolyData, vtkPolyData*);
 
  // added to support slicer integration
  void SaveVisibility(void);
  void RestoreVisibility(void);
  void Hide();
  void Show();
  
  // *** moved to public so can be instantiated by FESurface list factory
  vtkMimxSurfacePolyDataActor();
   ~vtkMimxSurfacePolyDataActor();
protected:
 
  vtkPolyData *PolyData;
  vtkPolyDataMapper *PolyDataMapper;
private:
  vtkMimxSurfacePolyDataActor(const vtkMimxSurfacePolyDataActor&);  // Not implemented.
  void operator=(const vtkMimxSurfacePolyDataActor&);  // Not implemented.
  double FillColor[3];
        double OutlineColor[3];
        int DisplayType;
        bool SavedVisibility;
};

#endif

