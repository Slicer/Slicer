#ifndef __qSlicerCropVolumeModule_h
#define __qSlicerCropVolumeModule_h

// Slicer includes
#include "qSlicerLoadableModule.h"

#include "qSlicerCropVolumeModuleExport.h"

class qSlicerCropVolumeModulePrivate;

/// \ingroup Slicer_QtModules_CropVolume
class Q_SLICER_QTMODULES_CROPVOLUME_EXPORT qSlicerCropVolumeModule :
  public qSlicerLoadableModule
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.slicer.modules.loadable.qSlicerLoadableModule/1.0");
  Q_INTERFACES(qSlicerLoadableModule);

public:

  typedef qSlicerLoadableModule Superclass;
  explicit qSlicerCropVolumeModule(QObject *parent=nullptr);
  ~qSlicerCropVolumeModule() override;

  qSlicerGetTitleMacro(QTMODULE_TITLE);

  /// Return a custom icon for the module
  QIcon icon()const override;
  QStringList categories() const override;

  QString helpText()const override;
  QString acknowledgementText()const override;
  QStringList contributors()const override;

  QStringList dependencies()const override;

  /// Specify editable node types
  QStringList associatedNodeTypes()const override;

protected:
  /// Initialize the module. Register the volumes reader/writer
  void setup() override;

  /// Create and return the widget representation associated to this module
  qSlicerAbstractModuleRepresentation * createWidgetRepresentation() override;

  /// Create and return the logic associated to this module
  vtkMRMLAbstractLogic* createLogic() override;

protected:
  QScopedPointer<qSlicerCropVolumeModulePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerCropVolumeModule);
  Q_DISABLE_COPY(qSlicerCropVolumeModule);

};

#endif
