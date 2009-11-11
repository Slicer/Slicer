#ifndef __qSlicerCLIModuleLogic_h
#define __qSlicerCLIModuleLogic_h 

#include "qSlicerModuleLogic.h"

#include "qSlicerBaseQTCLIWin32Header.h"

class Q_SLICER_BASE_QTCLI_EXPORT qSlicerCLIModuleLogic : public qSlicerModuleLogic
{
  Q_OBJECT
  
public:
  
  typedef qSlicerModuleLogic Superclass;
  qSlicerCLIModuleLogic(QObject *parent=0);
  virtual ~qSlicerCLIModuleLogic();
  
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
