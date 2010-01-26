#ifndef __qSlicerTractographyFiducialSeedingModule_h
#define __qSlicerTractographyFiducialSeedingModule_h

// SlicerQT includes
#include "qSlicerAbstractLoadableModule.h"

#include "qSlicerTractographyFiducialSeedingModuleExport.h"

class qSlicerTractographyFiducialSeedingModulePrivate;
class qSlicerTractographyFiducialSeedingModuleWidget;

class Q_SLICER_QTMODULES_TRACTOGRAPHYFIDUCIALSEEDING_EXPORT qSlicerTractographyFiducialSeedingModule : public qSlicerAbstractLoadableModule
{
  Q_INTERFACES(qSlicerAbstractLoadableModule);
public:
  typedef qSlicerAbstractLoadableModule Superclass;

  qSlicerTractographyFiducialSeedingModule(QObject *_parent = 0);

  qSlicerGetTitleMacro(QTMODULE_TITLE);

protected:
  // Create and return a widget representation of the object
  virtual qSlicerAbstractModuleWidget* createWidgetRepresentation();
  virtual qSlicerModuleLogic* createLogic();

private:
  //QCTK_DECLARE_PRIVATE(qSlicerTractographyFiducialSeedingModule);

};
#endif

