#ifndef __qSlicerAnnotationsModule_h
#define __qSlicerAnnotationsModule_h

// Slicer includes
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
  Q_PLUGIN_METADATA(IID "org.slicer.modules.loadable.qSlicerLoadableModule/1.0");
  Q_INTERFACES(qSlicerLoadableModule);
public:

  typedef qSlicerLoadableModule Superclass;
  qSlicerAnnotationsModule(QObject *parent=nullptr);
  ~qSlicerAnnotationsModule() override;

  /// Return the help and acknowledgement text for the Annotation module.
  QString helpText()const override;
  QString acknowledgementText()const override;
  QStringList contributors()const override;
  QStringList dependencies() const override;

  /// Return the icon of the Annotation module.
  QIcon icon()const override;

  QStringList categories()const override;

  /// Specify editable node types
  QStringList associatedNodeTypes()const override;

  qSlicerGetTitleMacro(QTMODULE_TITLE);

protected:

  /// All initialization code should be done in the setup
  void setup() override;

  // Description:
  // Create and return the widget representation associated to this module
  qSlicerAbstractModuleRepresentation* createWidgetRepresentation() override;

  // Description:
  // Create and return the logic associated to this module
  vtkMRMLAbstractLogic* createLogic() override;

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
