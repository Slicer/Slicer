#ifndef __qSlicerModuleTransform_h
#define __qSlicerModuleTransform_h 

#include "qSlicerAbstractModule.h"

#include "qSlicerBaseGUI2Win32Header.h"

class vtkMRMLNode; 

class Q_SLICER_BASE_GUI2_EXPORT qSlicerModuleTransform : public qSlicerAbstractModule
{ 
  Q_OBJECT

public:

  typedef qSlicerAbstractModule Superclass;
  qSlicerModuleTransform(QWidget *parent=0);
  virtual ~qSlicerModuleTransform(); 
  
  virtual void dumpObjectInfo();

protected slots:
  void onCoordinateReferenceButtonPressed(int id); 
  void onIdentityButtonPressed(); 
  void onNodeSelected(vtkMRMLNode* node);

private:
  class qInternal;
  qInternal* Internal;
};

#endif
