/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
 All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

#ifndef __qMRMLSegmentationShow3DButton_h
#define __qMRMLSegmentationShow3DButton_h

// CTK includes
#include <ctkMenuButton.h>
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// Segmentations includes
#include "qSlicerSegmentationsModuleWidgetsExport.h"

class qMRMLSegmentationShow3DButtonPrivate;
class vtkMRMLSegmentationNode;

/// \brief Button that shows/hide segmentation in 3D
/// It creates or removes closed surface representation.
class Q_SLICER_MODULE_SEGMENTATIONS_WIDGETS_EXPORT qMRMLSegmentationShow3DButton : public ctkMenuButton
{
  Q_OBJECT
  QVTK_OBJECT

  /// Disables the button. It is useful for preventing segmentation from editing.
  /// Using QWidget::setEnabled would only temporarily disable the button.
  Q_PROPERTY(bool locked READ locked WRITE setLocked)

public:
  explicit qMRMLSegmentationShow3DButton(QWidget* parent=nullptr);
  ~qMRMLSegmentationShow3DButton() override;

  /// Get current segmentation node
  Q_INVOKABLE vtkMRMLSegmentationNode* segmentationNode() const;

  bool locked() const;

public slots:
  /// Set current segmentation node
  Q_INVOKABLE void setSegmentationNode(vtkMRMLSegmentationNode* node);

  void setLocked(bool);

protected slots:
  void onToggled(bool toggled=true);
  void onEnableSurfaceSmoothingToggled(bool smoothingEnabled);
  void onSurfaceSmoothingFactorChanged(double newSmoothingFactor);
  void updateWidgetFromMRML();

protected:
  QScopedPointer<qMRMLSegmentationShow3DButtonPrivate> d_ptr;
private :
  Q_DECLARE_PRIVATE(qMRMLSegmentationShow3DButton);
  Q_DISABLE_COPY(qMRMLSegmentationShow3DButton);
};

#endif
