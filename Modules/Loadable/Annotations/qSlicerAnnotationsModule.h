#ifndef __qSlicerAnnotationsModule_h
#define __qSlicerAnnotationsModule_h

// SlicerQT includes
#include "qSlicerLoadableModule.h"

// CTK includes
#include <ctkPimpl.h>

#include "qSlicerAnnotationsModuleExport.h"

class qSlicerAbstractModuleWidget;
class qSlicerAnnotationsModulePrivate;

/// \ingroup Slicer_QtModules_Annotation
class Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT qSlicerAnnotationsModule :
  public qSlicerLoadableModule
{
  Q_OBJECT
  Q_INTERFACES(qSlicerLoadableModule)
public:

  typedef qSlicerLoadableModule Superclass;
  qSlicerAnnotationsModule(QObject *parent=0);
  virtual ~qSlicerAnnotationsModule();

  /// Return the help and acknowledgement text for the Annotation module.
  virtual QString helpText()const;
  virtual QString acknowledgementText()const;
  virtual QStringList contributors()const;
  virtual QStringList dependencies() const;

  /// Return the icon of the Annotation module.
  virtual QIcon icon()const;

  virtual QStringList categories()const;

  /// Specify editable node types
  virtual QStringList associatedNodeTypes()const;

  qSlicerGetTitleMacro(QTMODULE_TITLE);

protected:

  /// All initialization code should be done in the setup
  virtual void setup();

  // Description:
  // Create and return the widget representation associated to this module
  virtual qSlicerAbstractModuleRepresentation* createWidgetRepresentation();

  // Description:
  // Create and return the logic associated to this module
  virtual vtkMRMLAbstractLogic* createLogic();

public slots:
  /// a public slot to open up the screen  capture
  /// dialog (get the module manager, get the module annotation, get the
  /// widget representation, then invoke this method
  /// \sa qSlicerIOManager::openScreenshotDialog
  void showScreenshotDialog();

protected:
  QScopedPointer<qSlicerAnnotationsModulePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerAnnotationsModule);
  Q_DISABLE_COPY(qSlicerAnnotationsModule);
};

#endif
