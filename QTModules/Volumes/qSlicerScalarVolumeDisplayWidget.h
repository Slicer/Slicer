#ifndef __qSlicerScalarVolumeDisplayWidget_h
#define __qSlicerScalarVolumeDisplayWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// SlicerQt includes
#include <qSlicerWidget.h>

#include "qSlicerVolumesModuleExport.h"

class vtkMRMLNode;
class vtkMRMLScalarVolumeDisplayNode;
class vtkMRMLScalarVolumeNode;
class qSlicerScalarVolumeDisplayWidgetPrivate;

class Q_SLICER_QTMODULES_VOLUMES_EXPORT qSlicerScalarVolumeDisplayWidget : public qSlicerWidget
{
  Q_OBJECT
  QVTK_OBJECT
public:
  /// Constructors
  typedef qSlicerWidget Superclass;
  explicit qSlicerScalarVolumeDisplayWidget(QWidget* parent);
  virtual ~qSlicerScalarVolumeDisplayWidget();

  vtkMRMLScalarVolumeNode* volumeNode()const;
  vtkMRMLScalarVolumeDisplayNode* volumeDisplayNode()const;
public slots:

  /// 
  /// Set the MRML node of interest
  void setMRMLVolumeNode(vtkMRMLScalarVolumeNode* volumeNode);
  void setMRMLVolumeNode(vtkMRMLNode* node);

  void setInterpolate(bool interpolate);
  void setColorNode(vtkMRMLNode* colorNode);
  void setPreset(const QString& presetName);

protected slots:
  void updateWidgetFromMRML();
  void updateTransferFunction();
  void onPresetButtonClicked();

protected:
  void showEvent(QShowEvent * event);
protected:
  QScopedPointer<qSlicerScalarVolumeDisplayWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerScalarVolumeDisplayWidget);
  Q_DISABLE_COPY(qSlicerScalarVolumeDisplayWidget);
};

#endif
