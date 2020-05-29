#ifndef __qSlicerCropVolumeModuleWidget_h
#define __qSlicerCropVolumeModuleWidget_h

// Slicer includes
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
  qSlicerCropVolumeModuleWidget(QWidget *parent=nullptr);
  ~qSlicerCropVolumeModuleWidget() override;

  bool setEditedNode(vtkMRMLNode* node, QString role = QString(), QString context = QString()) override;

public slots:
  void setParametersNode(vtkMRMLNode* node);

protected:
  QScopedPointer<qSlicerCropVolumeModuleWidgetPrivate> d_ptr;

  void setup() override;
  void enter() override;
  void setMRMLScene(vtkMRMLScene*) override;

protected slots:
  void setInputVolume(vtkMRMLNode*);
  void setOutputVolume(vtkMRMLNode* node);
  void setInputROI(vtkMRMLNode*);
  void initializeInputROI(vtkMRMLNode*);
  /// when ROIs get added to the node selector, if the selector doesn't
  /// have a current node, select it
  void onInputROIAdded(vtkMRMLNode* node);

  void onROIVisibilityChanged(bool);
  void onROIFit();
  void onInterpolationModeChanged();
  void onApply();
  void onFixAlignment();
  void updateWidgetFromMRML();
  void onSpacingScalingValueChanged(double);
  void onIsotropicModeChanged(bool);
  void onMRMLSceneEndBatchProcessEvent();
  void onInterpolationEnabled(bool interpolationEnabled);
  void onVolumeInformationSectionClicked(bool isOpen);
  void onFillValueChanged(double);

  void updateVolumeInfo();

private:
  Q_DECLARE_PRIVATE(qSlicerCropVolumeModuleWidget);
  Q_DISABLE_COPY(qSlicerCropVolumeModuleWidget);
};

#endif
