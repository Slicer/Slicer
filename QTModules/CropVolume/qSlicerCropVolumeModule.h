#ifndef __qSlicerCropVolumeModule_h
#define __qSlicerCropVolumeModule_h

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerLoadableModule.h"

#include "qSlicerCropVolumeModuleExport.h"

class qSlicerCropVolumeModulePrivate;

/// \ingroup Slicer_QtModules_CropVolume
class Q_SLICER_QTMODULES_CROPVOLUME_EXPORT qSlicerCropVolumeModule :
  public qSlicerLoadableModule
{
  Q_OBJECT
  Q_INTERFACES(qSlicerLoadableModule);

public:

  typedef qSlicerLoadableModule Superclass;
  explicit qSlicerCropVolumeModule(QObject *parent=0);
  virtual ~qSlicerCropVolumeModule();

  qSlicerGetTitleMacro(QTMODULE_TITLE);
  
  ///
  /// Help to use the module
  virtual QString helpText()const;
  virtual QString category() const { return QString("Converters"); };

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
  QScopedPointer<qSlicerCropVolumeModulePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerCropVolumeModule);
  Q_DISABLE_COPY(qSlicerCropVolumeModule);

};

#endif
