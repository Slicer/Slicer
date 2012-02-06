#ifndef __qSlicerTractographyFiducialSeedingModule_h
#define __qSlicerTractographyFiducialSeedingModule_h

/// SlicerQT includes
#include "qSlicerLoadableModule.h"

#include "qSlicerTractographyFiducialSeedingModuleExport.h"

class qSlicerTractographyFiducialSeedingModulePrivate;
class qSlicerTractographyFiducialSeedingModuleWidget;

/// \ingroup Slicer_QtModules_TractographyFiducialSeeding
class Q_SLICER_QTMODULES_TRACTOGRAPHYFIDUCIALSEEDING_EXPORT qSlicerTractographyFiducialSeedingModule : public qSlicerLoadableModule
{
  Q_OBJECT
  Q_INTERFACES(qSlicerLoadableModule);
public:
  typedef qSlicerLoadableModule Superclass;

  qSlicerTractographyFiducialSeedingModule(QObject *_parent = 0);

  /// Category of the module
  virtual QStringList categories() const;

  qSlicerGetTitleMacro(QTMODULE_TITLE);

  virtual QString helpText()const;
  virtual QString acknowledgementText()const;
  virtual QString contributor()const;

protected:
  /// Create and return a widget representation of the object
  virtual qSlicerAbstractModuleRepresentation* createWidgetRepresentation();
  virtual vtkMRMLAbstractLogic* createLogic();

private:
  //  Q_DECLARE_PRIVATE(qSlicerTractographyFiducialSeedingModule);
  Q_DISABLE_COPY(qSlicerTractographyFiducialSeedingModule);

};
#endif

