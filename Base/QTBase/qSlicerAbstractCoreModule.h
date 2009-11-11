#ifndef __qSlicerAbstractCoreModule_h
#define __qSlicerAbstractCoreModule_h 

#include "qSlicerAbstractModule.h"

#include "qSlicerBaseQTBaseWin32Header.h"

class Q_SLICER_BASE_QTBASE_EXPORT qSlicerAbstractCoreModule : public qSlicerAbstractModule
{
  Q_OBJECT
  
public:
  
  typedef qSlicerAbstractModule Superclass;
  qSlicerAbstractCoreModule(QWidget *parent=0);
  virtual ~qSlicerAbstractCoreModule();
  
  virtual void printAdditionalInfo(); 
  
protected:
  virtual void initializer(); 
  
  // Description:
  // Convenient method to return slicer wiki URL
  QString slicerWikiUrl(){ return "http://www.slicer.org/slicerWiki/index.php"; }

private:
  class qInternal;
  qInternal* Internal;
};

#endif
