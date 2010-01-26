/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/


#ifndef __qSlicerModuleLogic_h
#define __qSlicerModuleLogic_h

// qCTK includes
#include <qCTKPimpl.h>

#include "qSlicerBaseQTBaseExport.h"

class vtkMRMLScene;
class vtkSlicerApplicationLogic;
class qSlicerModuleLogicPrivate;
class qSlicerAbstractModule;

class Q_SLICER_BASE_QTBASE_EXPORT qSlicerModuleLogic
{
public:

  qSlicerModuleLogic();
  virtual ~qSlicerModuleLogic();

  void initialize(vtkSlicerApplicationLogic* appLogic);

protected:

  // Description:
  // Set/Get MRML Scene
  void setMRMLScene(vtkMRMLScene* mrmlScene);
  vtkMRMLScene* mrmlScene() const;

  // Description:
  // Get application logic
  vtkSlicerApplicationLogic* appLogic() const;
  
  // Description
  // Called when appLogic is set. The inherited classes can call appLogic().
  // to do an special initialization if needed.
  // Doesn't do anything by default.
  virtual void setup(){}

  // Description:
  // This method is respondible to register additional MRML node with the current scene
  // Should only be called from setMRMLScene method
  virtual void registerNodes(vtkMRMLScene* scene){ Q_UNUSED(scene); }

private:
  QCTK_DECLARE_PRIVATE(qSlicerModuleLogic);

  // Give access to setMRMLScene method
  friend class qSlicerAbstractModule; 
};

#endif
