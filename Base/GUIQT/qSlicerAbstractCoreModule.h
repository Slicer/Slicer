#ifndef __qSlicerAbstractCoreModule_h
#define __qSlicerAbstractCoreModule_h 

#include "qSlicerAbstractModule.h"

#include "qSlicerBaseGUIQTWin32Header.h"

class vtkMRMLScene; 

class Q_SLICER_BASE_GUIQT_EXPORT qSlicerAbstractCoreModule : public qSlicerAbstractModule
{
  Q_OBJECT
  
public:
  
  typedef qSlicerAbstractModule Superclass;
  qSlicerAbstractCoreModule(QWidget *parent=0);
  virtual ~qSlicerAbstractCoreModule();
  
  virtual void printAdditionalInfo(); 
  
protected:
  // Description:
  // Convenient method to return slicer wiki URL
  QString slicerWikiUrl(){ return "http://www.slicer.org/slicerWiki/index.php"; }

private:
  class qInternal;
  qInternal* Internal;
};

#endif
