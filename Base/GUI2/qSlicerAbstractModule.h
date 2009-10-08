#ifndef __qSlicerAbstractModule_h
#define __qSlicerAbstractModule_h 

#include <QWidget>
#include "qVTKObject.h"

#include "qSlicerBaseGUI2Win32Header.h"

class vtkMRMLScene; 

class Q_SLICER_BASE_GUI2_EXPORT qSlicerAbstractModule : public QWidget
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
  // todo: title is a property
  // Title is the name of the module displayed in the module list.
  void setTitle(const QString&);
  QString title()const;
  
  // Description:
  // Set MRML scene
  virtual void setMRMLScene(vtkMRMLScene * scene);
  
  // Description:
  virtual void populateToolbar(){}
  virtual void unPopulateToolbar(){}
  
  // Description:
  virtual void populateApplicationSettings(){}
  virtual void unPopulateApplicationSettings(){}

public slots:

  // Description:
  // Set/Get module enabled
  bool moduleEnabled(); 
  virtual void setModuleEnabled(bool value); 

protected:
  // Description:
  // Get MRML scene
  virtual vtkMRMLScene* getMRMLScene(); 
  
  QString Title;

private:
  class qInternal;
  qInternal* Internal;
};

#endif
