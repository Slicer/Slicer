#ifndef __qSlicerTractographyInteractiveSeedingModule_h
#define __qSlicerTractographyInteractiveSeedingModule_h

/// SlicerQT includes
#include "qSlicerLoadableModule.h"

#include "qSlicerTractographyInteractiveSeedingModuleExport.h"

class qSlicerTractographyInteractiveSeedingModulePrivate;
class qSlicerTractographyInteractiveSeedingModuleWidget;

/// \ingroup Slicer_QtModules_TractographyInteractiveSeeding
class Q_SLICER_QTMODULES_TRACTOGRAPHYINTERACTIVESEEDING_EXPORT qSlicerTractographyInteractiveSeedingModule : public qSlicerLoadableModule
{
  Q_OBJECT
  Q_INTERFACES(qSlicerLoadableModule);
public:
  typedef qSlicerLoadableModule Superclass;

  qSlicerTractographyInteractiveSeedingModule(QObject *_parent = 0);

  /// Category of the module
  virtual QStringList categories() const;

  qSlicerGetTitleMacro(QTMODULE_TITLE);

  virtual QString helpText()const;
  virtual QString acknowledgementText()const;
  virtual QStringList contributors()const;

protected:
  /// Create and return a widget representation of the object
  virtual qSlicerAbstractModuleRepresentation* createWidgetRepresentation();
  virtual vtkMRMLAbstractLogic* createLogic();

private:
  //  Q_DECLARE_PRIVATE(qSlicerTractographyInteractiveSeedingModule);
  Q_DISABLE_COPY(qSlicerTractographyInteractiveSeedingModule);

};
#endif

