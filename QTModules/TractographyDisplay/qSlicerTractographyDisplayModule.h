#ifndef __qSlicerTractographyDisplayModule_h
#define __qSlicerTractographyDisplayModule_h

/// SlicerQT includes
#include "qSlicerAbstractLoadableModule.h"

#include "qSlicerTractographyDisplayModuleExport.h"

class qSlicerTractographyDisplayModulePrivate;

class Q_SLICER_QTMODULES_TRACTOGRAPHYDISPLAY_EXPORT qSlicerTractographyDisplayModule
  :public qSlicerAbstractLoadableModule
{
  Q_OBJECT
  Q_INTERFACES(qSlicerAbstractLoadableModule);
public:
  typedef qSlicerAbstractLoadableModule Superclass;

  qSlicerTractographyDisplayModule(QObject *_parent = 0);

  ///
  /// Display name for the module
  virtual QString title()const {return "FiberBundles";}

  ///
  /// Help text of the module
  virtual QString helpText()const;

  ///
  /// Acknowledgement of the module
  virtual QString acknowledgementText()const;

protected:
  ///
  /// Initialize the module. Register the volumes reader/writer
  virtual void setup();

  /// Create and return a widget representation of the object
  virtual qSlicerAbstractModuleRepresentation* createWidgetRepresentation();

  ///
  /// Create and return the logic associated to this module
  virtual vtkSlicerLogic* createLogic();
};
#endif

