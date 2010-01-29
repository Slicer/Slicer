#ifndef __qSlicerTractographyFiducialSeedingModule_h
#define __qSlicerTractographyFiducialSeedingModule_h

/// SlicerQT includes
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
  
  virtual QString title()const {return "FiducialSeeding";} 

protected:
  /// Create and return a widget representation of the object
  virtual qSlicerAbstractModuleWidget* createWidgetRepresentation();
  virtual vtkSlicerLogic* createLogic();

private:
  //QCTK_DECLARE_PRIVATE(qSlicerTractographyFiducialSeedingModule);

};
#endif

