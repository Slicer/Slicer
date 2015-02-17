#ifndef __qSlicerCropVolumeModuleWidget_h
#define __qSlicerCropVolumeModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerCropVolumeModuleExport.h"

class qSlicerCropVolumeModuleWidgetPrivate;
class vtkMRMLNode;
class vtkMRMLCropVolumeParametersNode;

/// \ingroup Slicer_QtModules_CropVolume
class Q_SLICER_QTMODULES_CROPVOLUME_EXPORT qSlicerCropVolumeModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerCropVolumeModuleWidget(QWidget *parent=0);
  virtual ~qSlicerCropVolumeModuleWidget();

public slots:

protected:
  QScopedPointer<qSlicerCropVolumeModuleWidgetPrivate> d_ptr;

  virtual void setup();
  virtual void enter();
  virtual void setMRMLScene(vtkMRMLScene*);

  void initializeParameterNode(vtkMRMLScene*);



protected slots:
  void initializeNode(vtkMRMLNode*);
  void onInputVolumeChanged();
  /// when input volumes get added to the node selector, if the selector doesn't
  /// have a current node, select it
  void onInputVolumeAdded(vtkMRMLNode*);
  void onInputROIChanged();
  /// when ROIs get added to the node selector, if the selector doesn't
  /// have a current node, select it
  void onInputROIAdded(vtkMRMLNode*);
  void onROIVisibilityChanged();
  void onInterpolationModeChanged();
  void onApply();
  void updateWidget();
  void updateParameters();
  void onSpacingScalingValueChanged(double);
  void onIsotropicModeChanged();
  void onEndCloseEvent();
  void onVoxelBasedChecked(bool checked);


private:
  Q_DECLARE_PRIVATE(qSlicerCropVolumeModuleWidget);
  Q_DISABLE_COPY(qSlicerCropVolumeModuleWidget);
};

#endif
