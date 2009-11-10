#ifndef __qSlicerCmdLineModuleLogic_h
#define __qSlicerCmdLineModuleLogic_h 

#include "qSlicerModuleLogic.h"

#include "qSlicerBaseGUIQTWin32Header.h"

class Q_SLICER_BASE_GUIQT_EXPORT qSlicerCmdLineModuleLogic : public qSlicerModuleLogic
{
  Q_OBJECT
  
public:
  
  typedef qSlicerModuleLogic Superclass;
  qSlicerCmdLineModuleLogic(QObject *parent=0);
  virtual ~qSlicerCmdLineModuleLogic();
  
  virtual void printAdditionalInfo(); 
  
  QString constructTemporaryFileName(const QString& tag,
                             const QString& type,
                             const QString& name,
                             const QStringList& extensions/*,
                             CommandLineModuleType commandType*/) const; 

protected:
//   std::string ConstructTemporarySceneFileName(vtkMRMLScene *scene);
//   std::string FindHiddenNodeID(const ModuleDescription& d,
//                                const ModuleParameter& p);

private:
  class qInternal;
  qInternal* Internal;
};

#endif
