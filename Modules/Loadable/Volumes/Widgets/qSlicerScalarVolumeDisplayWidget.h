#ifndef __qSlicerScalarVolumeDisplayWidget_h
#define __qSlicerScalarVolumeDisplayWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkVTKObject.h>

// Slicer includes
#include <qSlicerWidget.h>

#include "qSlicerVolumesModuleWidgetsExport.h"

class vtkMRMLNode;
class vtkMRMLScalarVolumeDisplayNode;
class vtkMRMLScalarVolumeNode;
class qSlicerScalarVolumeDisplayWidgetPrivate;

/// \ingroup Slicer_QtModules_Volumes
class Q_SLICER_QTMODULES_VOLUMES_WIDGETS_EXPORT qSlicerScalarVolumeDisplayWidget
  : public qSlicerWidget
{
  Q_OBJECT
  QVTK_OBJECT
  Q_PROPERTY(bool enableColorTableComboBox READ isColorTableComboBoxEnabled WRITE setColorTableComboBoxEnabled )
  Q_PROPERTY(bool enableMRMLWindowLevelWidget READ isMRMLWindowLevelWidgetEnabled WRITE setMRMLWindowLevelWidgetEnabled )
public:
  /// Constructors
  typedef qSlicerWidget Superclass;
  explicit qSlicerScalarVolumeDisplayWidget(QWidget* parent);
  ~qSlicerScalarVolumeDisplayWidget() override;

  vtkMRMLScalarVolumeNode* volumeNode()const;
  vtkMRMLScalarVolumeDisplayNode* volumeDisplayNode()const;

  bool isColorTableComboBoxEnabled()const;
  void setColorTableComboBoxEnabled(bool);

  bool isMRMLWindowLevelWidgetEnabled()const;
  void setMRMLWindowLevelWidgetEnabled(bool);

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
  void updateHistogram();
  void onPresetButtonClicked();
  void onLockWindowLevelButtonClicked();
  void onHistogramSectionExpanded(bool);

protected:
  void showEvent(QShowEvent * event) override;
protected:
  QScopedPointer<qSlicerScalarVolumeDisplayWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerScalarVolumeDisplayWidget);
  Q_DISABLE_COPY(qSlicerScalarVolumeDisplayWidget);
};

#endif
