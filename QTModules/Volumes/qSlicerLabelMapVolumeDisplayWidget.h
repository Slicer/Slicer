#ifndef __qSlicerLabelMapVolumeDisplayWidget_h
#define __qSlicerLabelMapVolumeDisplayWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// SlicerQt includes
#include <qSlicerWidget.h>

#include "qSlicerVolumesModuleExport.h"

class vtkMRMLNode;
class vtkMRMLLabelMapVolumeDisplayNode;
class vtkMRMLScalarVolumeNode;
class qSlicerLabelMapVolumeDisplayWidgetPrivate;

class Q_SLICER_QTMODULES_VOLUMES_EXPORT qSlicerLabelMapVolumeDisplayWidget : public qSlicerWidget
{
  Q_OBJECT

public:
  /// Constructors
  typedef qSlicerWidget Superclass;
  explicit qSlicerLabelMapVolumeDisplayWidget(QWidget* parent);
  virtual ~qSlicerLabelMapVolumeDisplayWidget(){}

  vtkMRMLScalarVolumeNode* volumeNode()const;
  vtkMRMLLabelMapVolumeDisplayNode* volumeDisplayNode()const;
public slots:

  ///
  /// Set the MRML node of interest
  void setMRMLVolumeNode(vtkMRMLScalarVolumeNode* volumeNode);
  void setMRMLVolumeNode(vtkMRMLNode* node);

  void setColorNode(vtkMRMLNode* colorNode);

protected slots:
  void updateWidgetFromMRML();

private:
  CTK_DECLARE_PRIVATE(qSlicerLabelMapVolumeDisplayWidget);
};

#endif
