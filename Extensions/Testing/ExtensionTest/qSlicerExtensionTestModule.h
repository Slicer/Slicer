#ifndef __qSlicerExtensionTestModule_h
#define __qSlicerExtensionTestModule_h

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerLoadableModule.h"

#include "qSlicerExtensionTestModuleExport.h"

class qSlicerExtensionTestModulePrivate;

class Q_SLICER_QTMODULES_EXTENSIONTEST_EXPORT qSlicerExtensionTestModule :
  public qSlicerLoadableModule
{
  Q_OBJECT
  Q_INTERFACES(qSlicerLoadableModule);

public:

  typedef qSlicerLoadableModule Superclass;
  explicit qSlicerExtensionTestModule(QObject *parent=0);
  virtual ~qSlicerExtensionTestModule();

  qSlicerGetTitleMacro(QTMODULE_TITLE);
  
  ///
  /// Help to use the module
  virtual QString helpText()const;

  ///
  /// Return acknowledgements
  virtual QString acknowledgementText()const;
  
  ///
  /// Return a custom icon for the module
  virtual QIcon icon()const;

protected:
  ///
  /// Initialize the module. Register the volumes reader/writer
  virtual void setup();

  ///
  /// Create and return the widget representation associated to this module
  virtual qSlicerAbstractModuleRepresentation * createWidgetRepresentation();

  ///
  /// Create and return the logic associated to this module
  virtual vtkMRMLAbstractLogic* createLogic();

protected:
  QScopedPointer<qSlicerExtensionTestModulePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerExtensionTestModule);
  Q_DISABLE_COPY(qSlicerExtensionTestModule);

};

#endif
