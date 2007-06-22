/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkQdecModuleLogic.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
#ifndef __vtkQdecModuleLogic_h
#define __vtkQdecModuleLogic_h

#include "vtkSlicerModuleLogic.h"
#include "vtkMRMLScene.h"
#include "vtkSlicerVolumesGUI.h"


#include "vtkQdecModule.h"

#include "QdecProject.h"

class VTK_QDECMODULE_EXPORT vtkQdecModuleLogic : public vtkSlicerModuleLogic
{
  public:
  static vtkQdecModuleLogic *New();
  vtkTypeMacro(vtkQdecModuleLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  // TODO: do we need to observe MRML here?
  virtual void ProcessMrmlEvents ( vtkObject *caller, unsigned long event,
                                   void *callData ){};  
  
  // The method that creates and runs VTK or ITK pipeline
  void Apply(vtkSlicerVolumesGUI *volsGUI);

  // Description:
  // Qdec project, encapsulates useful methods
  QdecProject* QDECProject;
  
  // Description:
  // Load in the data table, uses QDECProject method
  int LoadDataTable(const char* fileName);

  
protected:
  vtkQdecModuleLogic();
  ~vtkQdecModuleLogic();
  vtkQdecModuleLogic(const vtkQdecModuleLogic&);
  void operator=(const vtkQdecModuleLogic&);

};

#endif
