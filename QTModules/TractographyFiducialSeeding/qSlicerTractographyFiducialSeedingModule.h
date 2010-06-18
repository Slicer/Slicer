#ifndef __qSlicerTractographyFiducialSeedingModule_h
#define __qSlicerTractographyFiducialSeedingModule_h

/// SlicerQT includes
#include "qSlicerAbstractLoadableModule.h"

#include "qSlicerTractographyFiducialSeedingModuleExport.h"

class qSlicerTractographyFiducialSeedingModulePrivate;
class qSlicerTractographyFiducialSeedingModuleWidget;

class Q_SLICER_QTMODULES_TRACTOGRAPHYFIDUCIALSEEDING_EXPORT qSlicerTractographyFiducialSeedingModule : public qSlicerAbstractLoadableModule
{
  Q_OBJECT
  Q_INTERFACES(qSlicerAbstractLoadableModule);
public:
  typedef qSlicerAbstractLoadableModule Superclass;

  qSlicerTractographyFiducialSeedingModule(QObject *_parent = 0);

  /// 
  /// Display name for the module
  virtual QString title()const {return "FiducialSeeding";} 
   /// 
  /// Help text of the module
  virtual QString helpText()const;

  /// 
  /// Acknowledgement of the module
  virtual QString acknowledgementText()const;


protected:
  /// Create and return a widget representation of the object
  virtual qSlicerAbstractModuleRepresentation* createWidgetRepresentation();
  virtual vtkSlicerLogic* createLogic();

private:
  //CTK_DECLARE_PRIVATE(qSlicerTractographyFiducialSeedingModule);

};
#endif

