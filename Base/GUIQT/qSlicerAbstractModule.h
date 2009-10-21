#ifndef __qSlicerAbstractModule_h
#define __qSlicerAbstractModule_h 

#include <QWidget>
#include "qVTKObject.h"

#include "qSlicerBaseGUIQTWin32Header.h"

class vtkMRMLScene; 

class Q_SLICER_BASE_GUIQT_EXPORT qSlicerAbstractModule : public QWidget
{
  QVTK_OBJECT
  Q_OBJECT
  
public:
  
  typedef QWidget Superclass;
  qSlicerAbstractModule(QWidget *parent=0);
  virtual ~qSlicerAbstractModule();
  
  virtual void dumpObjectInfo(); 
  
  // Description:
  virtual QString moduleName();
  
  // Description:
  // Set MRML scene
  virtual void setMRMLScene(vtkMRMLScene * scene);
  
  // Description:
  virtual void populateToolbar(){}
  virtual void unPopulateToolbar(){}
  
  // Description:
  virtual void populateApplicationSettings(){}
  virtual void unPopulateApplicationSettings(){}

#ifdef Slicer3_USE_KWWidgets
  void synchronizeGeometryWithKWModule();
#endif

public slots:

  // Description:
  // Set/Get module enabled
  bool moduleEnabled(); 
  virtual void setModuleEnabled(bool value); 

protected:
  // Description:
  // Get MRML scene
  virtual vtkMRMLScene* getMRMLScene(); 

private:
  class qInternal;
  qInternal* Internal;
};

#endif
