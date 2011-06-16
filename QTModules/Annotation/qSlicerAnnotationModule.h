#ifndef __qSlicerAnnotationModule_h
#define __qSlicerAnnotationModule_h

// SlicerQT includes
#include "qSlicerLoadableModule.h"

// CTK includes
#include <ctkPimpl.h>

#include "qSlicerAnnotationModuleExport.h"

class qSlicerAbstractModuleWidget;
class qSlicerAnnotationModulePrivate;

/// \ingroup Slicer_QtModules_Annotation
class Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT qSlicerAnnotationModule :
  public qSlicerLoadableModule
{
  Q_OBJECT
  Q_INTERFACES(qSlicerLoadableModule)
public:

  typedef qSlicerLoadableModule Superclass;
  qSlicerAnnotationModule(QObject *parent=0);
  virtual ~qSlicerAnnotationModule();

  ///
  /// Return the help and acknowledgement text for the Annotation module.
  virtual QString helpText()const ;
  virtual QString acknowledgementText()const;

  ///
  /// Return the icon of the Annotation module.
  virtual QIcon icon()const;

  qSlicerGetTitleMacro(QTMODULE_TITLE);

protected:

  ///
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
  QScopedPointer<qSlicerAnnotationModulePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerAnnotationModule);
  Q_DISABLE_COPY(qSlicerAnnotationModule);
};

#endif
