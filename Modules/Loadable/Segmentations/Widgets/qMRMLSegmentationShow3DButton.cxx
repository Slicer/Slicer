/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
 All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

// Segmentation includes
#include "qMRMLSegmentationShow3DButton.h"
#include "vtkBinaryLabelmapToClosedSurfaceConversionRule.h"
#include "vtkMRMLSegmentationDisplayNode.h"
#include "vtkMRMLSegmentationNode.h"
#include "vtkSegmentationConverter.h"

// CTK includes
#include <ctkSliderWidget.h>

// VTK includes
#include "vtkWeakPointer.h"

// Qt includes
#include <QAction>
#include <QApplication>
#include <QDebug>
#include <QIcon>
#include <QMenu>
#include <QWidgetAction>

class qMRMLSegmentationShow3DButtonPrivate
{
  Q_DECLARE_PUBLIC(qMRMLSegmentationShow3DButton);
protected:
  qMRMLSegmentationShow3DButton* const q_ptr;
public:
  qMRMLSegmentationShow3DButtonPrivate(qMRMLSegmentationShow3DButton& object);
  void init();

  /// Updates surface smoothing factor in segmentation node and updates surface representation
  /// if it is enabled.
  bool setSurfaceSmoothingFactor(double smoothingFactor);

  QAction* SurfaceSmoothingEnabledAction{ nullptr };
  ctkSliderWidget* SurfaceSmoothingSlider{ nullptr };
  bool Locked{ false };
  vtkWeakPointer<vtkMRMLSegmentationNode> SegmentationNode;
};

//-----------------------------------------------------------------------------
CTK_SET_CPP(qMRMLSegmentationShow3DButton, bool, setLocked, Locked);
CTK_GET_CPP(qMRMLSegmentationShow3DButton, bool, locked, Locked);

//-----------------------------------------------------------------------------
qMRMLSegmentationShow3DButtonPrivate::qMRMLSegmentationShow3DButtonPrivate(qMRMLSegmentationShow3DButton& object)
  : q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
void qMRMLSegmentationShow3DButtonPrivate::init()
{
  Q_Q(qMRMLSegmentationShow3DButton);
  q->setIcon(QIcon(":/Icons/MakeModel.png"));
  q->setCheckable(true);
  q->setText(qMRMLSegmentationShow3DButton::tr("Show 3D"));
  QObject::connect(q, SIGNAL(toggled(bool)), q, SLOT(onToggled(bool)));

  QMenu* show3DButtonMenu = new QMenu(qMRMLSegmentationShow3DButton::tr("Show 3D"), q);

  this->SurfaceSmoothingEnabledAction = new QAction(qMRMLSegmentationShow3DButton::tr("Surface smoothing"), show3DButtonMenu);
  this->SurfaceSmoothingEnabledAction->setToolTip(
    qMRMLSegmentationShow3DButton::tr("Apply smoothing when converting binary labelmap to closed surface representation."));
  this->SurfaceSmoothingEnabledAction->setCheckable(true);
  show3DButtonMenu->addAction(this->SurfaceSmoothingEnabledAction);
  QObject::connect(this->SurfaceSmoothingEnabledAction, SIGNAL(toggled(bool)), q, SLOT(onEnableSurfaceSmoothingToggled(bool)));

  QMenu* surfaceSmoothingFactorMenu = new QMenu(qMRMLSegmentationShow3DButton::tr("Smoothing factor"), show3DButtonMenu);
  surfaceSmoothingFactorMenu->setObjectName("slicerSpacingManualMode");
  surfaceSmoothingFactorMenu->setIcon(QIcon(":/Icon/SlicerManualSliceSpacing.png"));

  this->SurfaceSmoothingSlider = new ctkSliderWidget(surfaceSmoothingFactorMenu);
  this->SurfaceSmoothingSlider->setToolTip(
    qMRMLSegmentationShow3DButton::tr("Higher value means stronger smoothing during closed surface representation conversion."));
  this->SurfaceSmoothingSlider->setDecimals(2);
  this->SurfaceSmoothingSlider->setRange(0.0, 1.0);
  this->SurfaceSmoothingSlider->setSingleStep(0.1);
  this->SurfaceSmoothingSlider->setValue(0.5);
  this->SurfaceSmoothingSlider->setTracking(false);
  QObject::connect(this->SurfaceSmoothingSlider, SIGNAL(valueChanged(double)),
    q, SLOT(onSurfaceSmoothingFactorChanged(double)));
  QWidgetAction* smoothingFactorAction = new QWidgetAction(surfaceSmoothingFactorMenu);
  smoothingFactorAction->setCheckable(true);
  smoothingFactorAction->setDefaultWidget(this->SurfaceSmoothingSlider);
  surfaceSmoothingFactorMenu->addAction(smoothingFactorAction);
  show3DButtonMenu->addMenu(surfaceSmoothingFactorMenu);

  q->setMenu(show3DButtonMenu);

  q->setEnabled(false);
}

//-----------------------------------------------------------------------------
bool qMRMLSegmentationShow3DButtonPrivate::setSurfaceSmoothingFactor(double smoothingFactor)
{
  if (!this->SegmentationNode || !this->SegmentationNode->GetSegmentation())
    {
    return false;
    }

  MRMLNodeModifyBlocker blocker(this->SegmentationNode);

  this->SegmentationNode->GetSegmentation()->SetConversionParameter(
    vtkBinaryLabelmapToClosedSurfaceConversionRule::GetSmoothingFactorParameterName(),
    QVariant(smoothingFactor).toString().toUtf8().constData());

  bool closedSurfacePresent = this->SegmentationNode->GetSegmentation()->ContainsRepresentation(
    vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName());
  if (closedSurfacePresent)
    {
    QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
    this->SegmentationNode->GetSegmentation()->CreateRepresentation(
      vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName(), true);
    this->SegmentationNode->Modified();
    QApplication::restoreOverrideCursor();
    }
  return true;
}

//-----------------------------------------------------------------------------
vtkMRMLSegmentationNode* qMRMLSegmentationShow3DButton::segmentationNode() const
{
  Q_D(const qMRMLSegmentationShow3DButton);
  return d->SegmentationNode;
}

//-----------------------------------------------------------------------------
void qMRMLSegmentationShow3DButton::setSegmentationNode(vtkMRMLSegmentationNode* node)
{
  Q_D(qMRMLSegmentationShow3DButton);

  if (d->SegmentationNode == node)
    {
    return;
    }

  qvtkReconnect(d->SegmentationNode, node, vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromMRML()));
  qvtkReconnect(d->SegmentationNode, node, vtkSegmentation::SegmentAdded, this, SLOT(updateWidgetFromMRML()));
  qvtkReconnect(d->SegmentationNode, node, vtkSegmentation::SegmentRemoved, this, SLOT(updateWidgetFromMRML()));
  qvtkReconnect(d->SegmentationNode, node, vtkSegmentation::ContainedRepresentationNamesModified, this, SLOT(updateWidgetFromMRML()));

  vtkMRMLSegmentationNode* segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(node);
  d->SegmentationNode = segmentationNode;

  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qMRMLSegmentationShow3DButton::updateWidgetFromMRML()
{
  Q_D(qMRMLSegmentationShow3DButton);

  // Update state of Show3DButton
  if (d->SegmentationNode && d->SegmentationNode->GetSegmentation())
    {
    // Enable button if there is at least one segment in the segmentation
    this->setEnabled(!d->Locked
      && d->SegmentationNode->GetSegmentation()->GetNumberOfSegments() > 0
      && d->SegmentationNode->GetSegmentation()->GetMasterRepresentationName() !=
        vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName());

    // Change button state based on whether it contains closed surface representation
    bool closedSurfacePresent = d->SegmentationNode->GetSegmentation()->ContainsRepresentation(
      vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName());
    bool wasBlocked = this->blockSignals(true);
    this->setChecked(closedSurfacePresent);
    this->blockSignals(wasBlocked);
    }
  else
    {
    this->setEnabled(false);
    }

  double surfaceSmoothingFactor = 0.5;
  if (d->SegmentationNode && d->SegmentationNode->GetSegmentation())
    {
    surfaceSmoothingFactor = QString(d->SegmentationNode->GetSegmentation()->GetConversionParameter(
      vtkBinaryLabelmapToClosedSurfaceConversionRule::GetSmoothingFactorParameterName()).c_str()).toDouble();
    }
  bool wasBlocked = d->SurfaceSmoothingEnabledAction->blockSignals(true);
  d->SurfaceSmoothingEnabledAction->setChecked(surfaceSmoothingFactor >= 0.0);
  d->SurfaceSmoothingEnabledAction->blockSignals(wasBlocked);

  wasBlocked = d->SurfaceSmoothingSlider->blockSignals(true);
  d->SurfaceSmoothingSlider->setValue(surfaceSmoothingFactor);
  d->SurfaceSmoothingSlider->setEnabled(surfaceSmoothingFactor >= 0.0);
  d->SurfaceSmoothingSlider->blockSignals(wasBlocked);
}

//-----------------------------------------------------------------------------
qMRMLSegmentationShow3DButton::qMRMLSegmentationShow3DButton(QWidget* _parent)
  : ctkMenuButton(_parent)
  , d_ptr(new qMRMLSegmentationShow3DButtonPrivate(*this))
{
  Q_D(qMRMLSegmentationShow3DButton);
  d->init();
}

//-----------------------------------------------------------------------------
qMRMLSegmentationShow3DButton::~qMRMLSegmentationShow3DButton() = default;

//-----------------------------------------------------------------------------
void qMRMLSegmentationShow3DButton::onToggled(bool on)
{
  Q_D(qMRMLSegmentationShow3DButton);
  if (!d->SegmentationNode || !d->SegmentationNode->GetSegmentation())
    {
    return;
    }

  MRMLNodeModifyBlocker segmentationNodeBlocker(d->SegmentationNode);

  if (on)
    {
    // Button is pressed, create closed surface representation and show it.
    // Make sure closed surface representation exists
    if (d->SegmentationNode->GetSegmentation()->CreateRepresentation(
      vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName() ))
      {
      vtkMRMLSegmentationDisplayNode* displayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(
        d->SegmentationNode->GetDisplayNode());
      if (displayNode)
        {
        // Set closed surface as displayed poly data representation
        displayNode->SetPreferredDisplayRepresentationName3D(
          vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName());
        }
      // But keep binary labelmap for 2D
      bool binaryLabelmapPresent = d->SegmentationNode->GetSegmentation()->ContainsRepresentation(
        vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName());
      if (binaryLabelmapPresent && displayNode)
        {
        displayNode->SetPreferredDisplayRepresentationName2D(
          vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName() );
        }
      }
    }
  else
    {
    // Button is released, remove the closed surface representation
    // (but only if it's not the master representation).
    if (d->SegmentationNode->GetSegmentation()->GetMasterRepresentationName() !=
      vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName())
      {
      d->SegmentationNode->GetSegmentation()->RemoveRepresentation(
        vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName());
      }
    }
}

//-----------------------------------------------------------------------------
void qMRMLSegmentationShow3DButton::onEnableSurfaceSmoothingToggled(bool smoothingEnabled)
{
  Q_D(qMRMLSegmentationShow3DButton);
  if (!d->SegmentationNode || !d->SegmentationNode->GetSegmentation())
    {
    qCritical() << Q_FUNC_INFO << ": No segmentation selected";
    return;
    }

  // Get smoothing factor
  double originalSmoothingFactor = QString(d->SegmentationNode->GetSegmentation()->GetConversionParameter(
    vtkBinaryLabelmapToClosedSurfaceConversionRule::GetSmoothingFactorParameterName()).c_str()).toDouble();
  double newSmoothingFactor = fabs(originalSmoothingFactor);
  if (originalSmoothingFactor == 0.0)
    {
    // if original smoothing factor was 0 then we cannot toggle smoothing
    // therefore we reset it to the default
    newSmoothingFactor = 0.5;
    }
  if (!smoothingEnabled)
    {
    newSmoothingFactor *= -1;
    }

  // Set smoothing factor
  if (newSmoothingFactor != originalSmoothingFactor)
    {
    d->setSurfaceSmoothingFactor(newSmoothingFactor);
    this->updateWidgetFromMRML();
    }
}

//---------------------------------------------------------------------------
void qMRMLSegmentationShow3DButton::onSurfaceSmoothingFactorChanged(double newSmoothingFactor)
{
  Q_D(qMRMLSegmentationShow3DButton);
  if (!d->SegmentationNode || !d->SegmentationNode->GetSegmentation())
    {
    qCritical() << Q_FUNC_INFO << ": No segmentation selected";
    return;
    }

  // Get smoothing factor
  double originalSmoothingFactor = QString(d->SegmentationNode->GetSegmentation()->GetConversionParameter(
    vtkBinaryLabelmapToClosedSurfaceConversionRule::GetSmoothingFactorParameterName() ).c_str() ).toDouble();

  // Sign of smoothing factor is used to indicate that smoothing is enabled or not.
  // if smoothing factor is negative then it means smoothing is disabled.
  // Here we allow changing the absolute value of the smoothing factor, while maintaining its sign.

  // Set smoothing factor
  if (newSmoothingFactor != fabs(originalSmoothingFactor))
    {
    if (originalSmoothingFactor < 0.0)
      {
      newSmoothingFactor = -newSmoothingFactor;
      }
    d->setSurfaceSmoothingFactor(newSmoothingFactor);
    }
}
