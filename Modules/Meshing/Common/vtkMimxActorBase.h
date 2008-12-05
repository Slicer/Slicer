/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxActorBase.h,v $
Language:  C++
Date:      $Date: 2008/07/28 15:06:02 $
Version:   $Revision: 1.13 $

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
// .NAME vtkMimxActorBase - a 3D non-orthogonal axes representation
// .SECTION Description
// vtkMimxActorBase is the abstract base class for all the pipeline setup for
// different types of datatypes. Data types concidered are vtkPolyData,
// vtkStructuredGrid (both plane and solid) and vtkUnstructuredGrid.

#ifndef __vtkMimxActorBase_h
#define __vtkMimxActorBase_h

#include "mimxCommonDefine.h"
#include "vtkProp3D.h"
#include "vtkMimxCommonWin32Header.h"

class vtkActor;
class vtkDataSet;
class vtkPolyData;

class VTK_MIMXCOMMON_EXPORT vtkMimxActorBase : public vtkProp3D
{
public:
 // static vtkActorBase *New();
  vtkTypeRevisionMacro(vtkMimxActorBase,vtkProp3D);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description: 
  //virtual vtkDataSet* GetDataSet() = 0;
  vtkGetMacro(DataType, vtkIdType);
  vtkSetMacro(DataType, vtkIdType);
  vtkGetMacro(Actor, vtkActor*);
  virtual double *GetBounds() {return NULL;}
//  virtual void SetDataType(int ) = 0;
  vtkGetMacro(FilePath, char*);
  vtkGetMacro(FileName, char*);
  vtkGetMacro(UniqueId, char*);
  vtkGetMacro(FoundationName, char*);
  void SetFilePath(const char *InputFilePath);
  //void SetFileName(const char *InputFileName);
  void SetUniqueId( const char *Id);
  void SetObjectName(const char *FilterName, vtkIdType &Count);
  void SetFoundationName(const char *FoundationName);
protected:
  vtkMimxActorBase();
  virtual ~vtkMimxActorBase();
  vtkActor *Actor;
  vtkIdType DataType;
  char* FilePath;
  char* FileName;
  char* UniqueId;
  char* FoundationName;
private:
  vtkMimxActorBase(const vtkMimxActorBase&);  // Not implemented.
  void operator=(const vtkMimxActorBase&);  // Not implemented.
};

#endif

