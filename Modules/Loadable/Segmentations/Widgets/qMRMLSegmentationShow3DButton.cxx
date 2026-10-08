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
#include <QActionGroup>
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

  /// Updates the method used to generate the surface representation
  bool setConversionMethod(const std::string& conversionMethod);

  /// This property holds whether smoothing internal to surface nets filter is used
  bool setInternalSmoothing(int internalSmoothing);

  /// Representation shown in 3D views (closed surface or binary labelmap), the same as the
  /// representation in 3D views of the display node
  std::string representation3D();

  QAction* SurfaceSmoothingEnabledAction{ nullptr };
  QAction* SurfaceNetsEnableAction{ nullptr };
  QAction* SurfaceNetsSmoothingEnableAction{ nullptr };
  ctkSliderWidget* SurfaceSmoothingSlider{ nullptr };
  QAction* ClosedSurfaceRepresentationAction{ nullptr };
  QAction* BinaryLabelmapRepresentationAction{ nullptr };
  bool Locked{ false };
  vtkWeakPointer<vtkMRMLSegmentationNode> SegmentationNode;
};

//-----------------------------------------------------------------------------
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
  this->SurfaceSmoothingEnabledAction->setToolTip(qMRMLSegmentationShow3DButton::tr("Apply smoothing when converting binary labelmap to closed surface representation."));
  this->SurfaceSmoothingEnabledAction->setCheckable(true);
  show3DButtonMenu->addAction(this->SurfaceSmoothingEnabledAction);
  QObject::connect(this->SurfaceSmoothingEnabledAction, SIGNAL(toggled(bool)), q, SLOT(onEnableSurfaceSmoothingToggled(bool)));

  QMenu* surfaceSmoothingFactorMenu = new QMenu(qMRMLSegmentationShow3DButton::tr("Smoothing factor"), show3DButtonMenu);
  surfaceSmoothingFactorMenu->setObjectName("surfaceSmoothingFactorMenu");

  // Add menu for experimental features
  QMenu* experimentalMenu = new QMenu(qMRMLSegmentationShow3DButton::tr("Experimental"), show3DButtonMenu);
  experimentalMenu->setObjectName("show3DExperimentalMenu");

  // Toggle for using surface nets instead of Flying Edges
  this->SurfaceNetsEnableAction = new QAction(qMRMLSegmentationShow3DButton::tr("Use Surface Nets (fast)"), experimentalMenu);
  this->SurfaceNetsEnableAction->setToolTip(
    qMRMLSegmentationShow3DButton::tr("Create closed surface using surface nets. By default, flying edges is used. Surface nets are more performant."));
  this->SurfaceNetsEnableAction->setCheckable(true);
  experimentalMenu->addAction(this->SurfaceNetsEnableAction);
  QObject::connect(this->SurfaceNetsEnableAction, SIGNAL(toggled(bool)), q, SLOT(onEnableSurfaceNetsToggled(bool)));

  // Toggle for using surface nets internal smoothing algorithm instead of vtkWindowedSincPolyDataFilter
  this->SurfaceNetsSmoothingEnableAction = new QAction(qMRMLSegmentationShow3DButton::tr("Use Surface Nets Smoothing (faster)"), experimentalMenu);
  this->SurfaceNetsSmoothingEnableAction->setToolTip(
    qMRMLSegmentationShow3DButton::tr("Use surface nets internal smoothing (more performant). vtkWindowedSincPolyDataFilter is used by default."));
  this->SurfaceNetsSmoothingEnableAction->setCheckable(true);
  experimentalMenu->addAction(this->SurfaceNetsSmoothingEnableAction);
  QObject::connect(this->SurfaceNetsSmoothingEnableAction, SIGNAL(toggled(bool)), q, SLOT(onEnableSurfaceNetsSmoothingToggled(bool)));

  this->SurfaceSmoothingSlider = new ctkSliderWidget(surfaceSmoothingFactorMenu);
  this->SurfaceSmoothingSlider->setToolTip(qMRMLSegmentationShow3DButton::tr("Higher value means stronger smoothing during closed surface representation conversion."));
  this->SurfaceSmoothingSlider->setDecimals(2);
  this->SurfaceSmoothingSlider->setRange(0.0, 1.0);
  this->SurfaceSmoothingSlider->setSingleStep(0.1);
  this->SurfaceSmoothingSlider->setValue(0.5);
  this->SurfaceSmoothingSlider->setTracking(false);
  QObject::connect(this->SurfaceSmoothingSlider, SIGNAL(valueChanged(double)), q, SLOT(onSurfaceSmoothingFactorChanged(double)));
  QWidgetAction* smoothingFactorAction = new QWidgetAction(surfaceSmoothingFactorMenu);
  smoothingFactorAction->setCheckable(true);
  smoothingFactorAction->setDefaultWidget(this->SurfaceSmoothingSlider);
  surfaceSmoothingFactorMenu->addAction(smoothingFactorAction);

  // Representation shown in 3D views (the same as in the segmentation display node widget)
  QMenu* representationMenu = new QMenu(qMRMLSegmentationShow3DButton::tr("Representation"), show3DButtonMenu);
  representationMenu->setObjectName("show3DRepresentationMenu");
  QActionGroup* representationActions = new QActionGroup(representationMenu);
  representationActions->setExclusive(true);
  this->BinaryLabelmapRepresentationAction = new QAction(qMRMLSegmentationShow3DButton::tr("Binary labelmap"), representationMenu);
  QString binaryLabelmapToolTip = qMRMLSegmentationShow3DButton::tr("Show the binary labelmap as smooth surfaces computed on the GPU (experimental).");
  this->BinaryLabelmapRepresentationAction->setToolTip(binaryLabelmapToolTip);
  this->BinaryLabelmapRepresentationAction->setData(QString::fromStdString(vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName()));
  this->ClosedSurfaceRepresentationAction = new QAction(qMRMLSegmentationShow3DButton::tr("Closed surface"), representationMenu);
  QString closedSurfaceToolTip = qMRMLSegmentationShow3DButton::tr("Show the closed surface representation, created from the binary labelmap.");
  this->ClosedSurfaceRepresentationAction->setToolTip(closedSurfaceToolTip);
  this->ClosedSurfaceRepresentationAction->setData(QString::fromStdString(vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName()));
  for (QAction* action : { this->BinaryLabelmapRepresentationAction, this->ClosedSurfaceRepresentationAction })
  {
    action->setCheckable(true);
    representationActions->addAction(action);
    representationMenu->addAction(action);
  }
  QObject::connect(representationActions, SIGNAL(triggered(QAction*)), q, SLOT(onRepresentationActionTriggered(QAction*)));

  show3DButtonMenu->addMenu(surfaceSmoothingFactorMenu);
  show3DButtonMenu->addMenu(representationMenu);
  show3DButtonMenu->addMenu(experimentalMenu);
  q->setMenu(show3DButtonMenu);
  q->setEnabled(false);
}

//-----------------------------------------------------------------------------
std::string qMRMLSegmentationShow3DButtonPrivate::representation3D()
{
  vtkMRMLSegmentationNode* segmentationNode = this->SegmentationNode;
  vtkMRMLSegmentationDisplayNode* displayNode = segmentationNode ? vtkMRMLSegmentationDisplayNode::SafeDownCast(segmentationNode->GetDisplayNode()) : nullptr;
  if (displayNode && displayNode->IsBinaryLabelmapPreferredDisplayRepresentation3D())
  {
    return vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName();
  }
  return vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName();
}

//-----------------------------------------------------------------------------
bool qMRMLSegmentationShow3DButtonPrivate::setSurfaceSmoothingFactor(double smoothingFactor)
{
  if (!this->SegmentationNode || !this->SegmentationNode->GetSegmentation())
  {
    return false;
  }

  MRMLNodeModifyBlocker blocker(this->SegmentationNode);

  this->SegmentationNode->GetSegmentation()->SetConversionParameter(vtkBinaryLabelmapToClosedSurfaceConversionRule::GetSmoothingFactorParameterName(),
                                                                    QVariant(smoothingFactor).toString().toUtf8().constData());

  bool closedSurfacePresent = this->SegmentationNode->GetSegmentation()->ContainsRepresentation(vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName());
  if (closedSurfacePresent)
  {
    QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
    this->SegmentationNode->GetSegmentation()->CreateRepresentation(vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName(), true);
    QApplication::restoreOverrideCursor();
  }
  // Binary labelmap shown in 3D uses the smoothing factor, too
  this->SegmentationNode->Modified();
  return true;
}

//-----------------------------------------------------------------------------
bool qMRMLSegmentationShow3DButtonPrivate::setConversionMethod(const std::string& conversionMethod)
{
  if (!this->SegmentationNode || !this->SegmentationNode->GetSegmentation())
  {
    return false;
  }

  MRMLNodeModifyBlocker blocker(this->SegmentationNode);

  this->SegmentationNode->GetSegmentation()->SetConversionParameter(vtkBinaryLabelmapToClosedSurfaceConversionRule::GetConversionMethodParameterName(), conversionMethod);

  bool closedSurfacePresent = this->SegmentationNode->GetSegmentation()->ContainsRepresentation(vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName());
  if (closedSurfacePresent)
  {
    QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
    this->SegmentationNode->GetSegmentation()->CreateRepresentation(vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName(), true);
    this->SegmentationNode->Modified();
    QApplication::restoreOverrideCursor();
  }
  return true;
}

//-----------------------------------------------------------------------------
bool qMRMLSegmentationShow3DButtonPrivate::setInternalSmoothing(int internalSmoothing)
{
  if (!this->SegmentationNode || !this->SegmentationNode->GetSegmentation())
  {
    return false;
  }

  MRMLNodeModifyBlocker blocker(this->SegmentationNode);

  this->SegmentationNode->GetSegmentation()->SetConversionParameter(vtkBinaryLabelmapToClosedSurfaceConversionRule::GetSurfaceNetInternalSmoothingParameterName(),
                                                                    QVariant(internalSmoothing).toString().toUtf8().constData());

  bool closedSurfacePresent = this->SegmentationNode->GetSegmentation()->ContainsRepresentation(vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName());
  if (closedSurfacePresent)
  {
    QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
    this->SegmentationNode->GetSegmentation()->CreateRepresentation(vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName(), true);
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
  qvtkReconnect(d->SegmentationNode, node, vtkMRMLDisplayableNode::DisplayModifiedEvent, this, SLOT(updateWidgetFromMRML()));

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
    this->setEnabled(!d->Locked                                                                  //
                     && d->SegmentationNode->GetSegmentation()->GetNumberOfSegments() > 0        //
                     && d->SegmentationNode->GetSegmentation()->GetSourceRepresentationName() != //
                          vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName());

    // Change button state based on whether it contains the representation shown in 3D views
    // (and, for binary labelmap, whether it is visible in 3D)
    std::string representation3D = d->representation3D();
    bool representationPresent = d->SegmentationNode->GetSegmentation()->ContainsRepresentation(representation3D);
    vtkMRMLSegmentationDisplayNode* displayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(d->SegmentationNode->GetDisplayNode());
    if (representation3D == vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName())
    {
      representationPresent = representationPresent && displayNode && displayNode->GetVisibility3D();
    }
    bool wasBlocked = this->blockSignals(true);
    this->setChecked(representationPresent);
    this->blockSignals(wasBlocked);
    d->BinaryLabelmapRepresentationAction->setChecked(representation3D == vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName());
    d->ClosedSurfaceRepresentationAction->setChecked(representation3D == vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName());
  }
  else
  {
    this->setEnabled(false);
  }

  // Smoothing factor
  double surfaceSmoothingFactor = 0.5;
  if (d->SegmentationNode && d->SegmentationNode->GetSegmentation())
  {
    surfaceSmoothingFactor =
      QString(d->SegmentationNode->GetSegmentation()->GetConversionParameter(vtkBinaryLabelmapToClosedSurfaceConversionRule::GetSmoothingFactorParameterName()).c_str()).toDouble();
  }
  bool wasBlocked = d->SurfaceSmoothingEnabledAction->blockSignals(true);
  d->SurfaceSmoothingEnabledAction->setChecked(surfaceSmoothingFactor >= 0.0);
  d->SurfaceSmoothingEnabledAction->blockSignals(wasBlocked);

  wasBlocked = d->SurfaceSmoothingSlider->blockSignals(true);
  d->SurfaceSmoothingSlider->setValue(surfaceSmoothingFactor);
  d->SurfaceSmoothingSlider->setEnabled(surfaceSmoothingFactor >= 0.0);
  d->SurfaceSmoothingSlider->blockSignals(wasBlocked);

  // Conversion method
  std::string conversionMethod = vtkBinaryLabelmapToClosedSurfaceConversionRule::CONVERSION_METHOD_FLYING_EDGES;
  if (d->SegmentationNode && d->SegmentationNode->GetSegmentation())
  {
    conversionMethod = d->SegmentationNode->GetSegmentation()->GetConversionParameter(vtkBinaryLabelmapToClosedSurfaceConversionRule::GetConversionMethodParameterName());
  }
  wasBlocked = d->SurfaceNetsEnableAction->blockSignals(true);
  d->SurfaceNetsEnableAction->setChecked(conversionMethod == vtkBinaryLabelmapToClosedSurfaceConversionRule::CONVERSION_METHOD_SURFACE_NETS);
  d->SurfaceNetsEnableAction->blockSignals(wasBlocked);

  // SurfaceNets smoothing
  int surfaceNetinternalSmoothing = 0;
  if (d->SegmentationNode && d->SegmentationNode->GetSegmentation())
  {
    surfaceNetinternalSmoothing =
      QString(d->SegmentationNode->GetSegmentation()->GetConversionParameter(vtkBinaryLabelmapToClosedSurfaceConversionRule::GetSurfaceNetInternalSmoothingParameterName()).c_str())
        .toInt();
  }
  wasBlocked = d->SurfaceNetsSmoothingEnableAction->blockSignals(true);
  d->SurfaceNetsSmoothingEnableAction->setChecked(surfaceNetinternalSmoothing == 1);
  d->SurfaceNetsSmoothingEnableAction->setEnabled(surfaceSmoothingFactor >= 0.0 //
                                                  && conversionMethod == vtkBinaryLabelmapToClosedSurfaceConversionRule::CONVERSION_METHOD_SURFACE_NETS);
  d->SurfaceNetsSmoothingEnableAction->blockSignals(wasBlocked);
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
  vtkSegmentation* segmentation = d->SegmentationNode->GetSegmentation();
  vtkMRMLSegmentationDisplayNode* displayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(d->SegmentationNode->GetDisplayNode());
  const std::string closedSurfaceName = vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName();
  const std::string binaryLabelmapName = vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName();
  std::string representation3D = d->representation3D();

  MRMLNodeModifyBlocker segmentationNodeBlocker(d->SegmentationNode);

  if (on)
  {
    // Button is pressed: create the representation that is shown in 3D views (only that one) and show it
    if (segmentation->CreateRepresentation(representation3D) && displayNode)
    {
      displayNode->SetPreferredDisplayRepresentationName3D(representation3D.c_str());
      if (representation3D == binaryLabelmapName)
      {
        displayNode->SetVisibility3D(true);
      }
      // But keep binary labelmap for 2D
      if (segmentation->ContainsRepresentation(binaryLabelmapName))
      {
        displayNode->SetPreferredDisplayRepresentationName2D(binaryLabelmapName.c_str());
      }
    }
  }
  else
  {
    // Button is released: remove the closed surface and binary labelmap representations
    // (unless they are the source representation)
    for (const std::string& name : { closedSurfaceName, binaryLabelmapName })
    {
      if (segmentation->GetSourceRepresentationName() != name)
      {
        segmentation->RemoveRepresentation(name);
      }
    }
    // A source representation that is shown in 3D is hidden instead
    if (displayNode && representation3D == binaryLabelmapName && segmentation->GetSourceRepresentationName() == binaryLabelmapName)
    {
      displayNode->SetVisibility3D(false);
    }
  }
}

//-----------------------------------------------------------------------------
void qMRMLSegmentationShow3DButton::onRepresentationActionTriggered(QAction* action)
{
  Q_D(qMRMLSegmentationShow3DButton);
  vtkMRMLSegmentationNode* segmentationNode = d->SegmentationNode;
  vtkMRMLSegmentationDisplayNode* displayNode = segmentationNode ? vtkMRMLSegmentationDisplayNode::SafeDownCast(segmentationNode->GetDisplayNode()) : nullptr;
  if (!action || !displayNode)
  {
    return;
  }
  std::string representation3D = action->data().toString().toStdString();
  bool shown = this->isChecked();
  vtkSegmentation* segmentation = d->SegmentationNode->GetSegmentation();
  MRMLNodeModifyBlocker segmentationNodeBlocker(d->SegmentationNode);
  displayNode->SetPreferredDisplayRepresentationName3D(representation3D.c_str());
  if (shown)
  {
    // Shown in 3D: show the chosen representation
    segmentation->CreateRepresentation(representation3D);
    if (representation3D == vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName())
    {
      displayNode->SetVisibility3D(true);
    }
  }
  // Remove the representation that is not shown in 3D anymore (unless it is the source representation),
  // so that it is not kept up to date for nothing (updating closed surface while editing is expensive)
  const std::string closedSurfaceName = vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName();
  const std::string binaryLabelmapName = vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName();
  for (const std::string& name : { closedSurfaceName, binaryLabelmapName })
  {
    if (name != representation3D && name != segmentation->GetSourceRepresentationName())
    {
      segmentation->RemoveRepresentation(name);
    }
  }
  this->updateWidgetFromMRML();
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
  double originalSmoothingFactor =
    QString(d->SegmentationNode->GetSegmentation()->GetConversionParameter(vtkBinaryLabelmapToClosedSurfaceConversionRule::GetSmoothingFactorParameterName()).c_str()).toDouble();
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

//-----------------------------------------------------------------------------
void qMRMLSegmentationShow3DButton::onEnableSurfaceNetsToggled(bool surfaceNetsEnabled)
{
  Q_D(qMRMLSegmentationShow3DButton);
  if (!d->SegmentationNode || !d->SegmentationNode->GetSegmentation())
  {
    qCritical() << Q_FUNC_INFO << ": No segmentation selected in onEnableSurfaceNetsToggled";
    return;
  }

  // Get current conversion method
  std::string originalMethod = d->SegmentationNode->GetSegmentation()->GetConversionParameter(vtkBinaryLabelmapToClosedSurfaceConversionRule::GetConversionMethodParameterName());
  std::string newMethod = vtkBinaryLabelmapToClosedSurfaceConversionRule::CONVERSION_METHOD_FLYING_EDGES;
  if (surfaceNetsEnabled)
  {
    newMethod = vtkBinaryLabelmapToClosedSurfaceConversionRule::CONVERSION_METHOD_SURFACE_NETS;
  }

  // Set conversion method
  if (newMethod != originalMethod)
  {
    d->setConversionMethod(newMethod);
    this->updateWidgetFromMRML();
  }
}

//-----------------------------------------------------------------------------
void qMRMLSegmentationShow3DButton::onEnableSurfaceNetsSmoothingToggled(bool surfaceNetsSmoothingEnabled)
{
  Q_D(qMRMLSegmentationShow3DButton);
  if (!d->SegmentationNode || !d->SegmentationNode->GetSegmentation())
  {
    qCritical() << Q_FUNC_INFO << ": No segmentation selected in onEnableSurfaceNetsSmoothingToggled";
    return;
  }

  // Get current conversion method
  // 0 = use vtkWindowedSincPolyDataFilter for smoothing
  // 1 = Use surface nets internal smoothing
  int originalMethod =
    QString(d->SegmentationNode->GetSegmentation()->GetConversionParameter(vtkBinaryLabelmapToClosedSurfaceConversionRule::GetSurfaceNetInternalSmoothingParameterName()).c_str())
      .toInt();
  int newMethod = 0;
  if (surfaceNetsSmoothingEnabled)
  {
    newMethod = 1;
  }

  // Set conversion method
  if (newMethod != originalMethod)
  {
    d->setInternalSmoothing(newMethod);
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
  double originalSmoothingFactor =
    QString(d->SegmentationNode->GetSegmentation()->GetConversionParameter(vtkBinaryLabelmapToClosedSurfaceConversionRule::GetSmoothingFactorParameterName()).c_str()).toDouble();

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

//---------------------------------------------------------------------------
void qMRMLSegmentationShow3DButton::setLocked(bool locked)
{
  Q_D(qMRMLSegmentationShow3DButton);
  d->Locked = locked;
  this->updateWidgetFromMRML();
}
