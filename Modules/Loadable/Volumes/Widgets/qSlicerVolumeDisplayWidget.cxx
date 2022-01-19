// Qt includes
#include <QVBoxLayout>

// Slicer includes
#include "qSlicerDiffusionTensorVolumeDisplayWidget.h"
#include "qSlicerDiffusionWeightedVolumeDisplayWidget.h"
#include "qSlicerLabelMapVolumeDisplayWidget.h"
#include "qSlicerScalarVolumeDisplayWidget.h"
#include "qSlicerVolumeDisplayWidget.h"

// MRML includes
#include <vtkMRMLDiffusionTensorVolumeNode.h>
#include <vtkMRMLDiffusionWeightedVolumeNode.h>
#include <vtkMRMLLabelMapVolumeNode.h>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Volumes
class qSlicerVolumeDisplayWidgetPrivate
{
  Q_DECLARE_PUBLIC(qSlicerVolumeDisplayWidget);

protected:
  qSlicerVolumeDisplayWidget* const q_ptr;

public:
  qSlicerVolumeDisplayWidgetPrivate(qSlicerVolumeDisplayWidget& object);
  void init();
  qSlicerWidget* widgetForVolume(vtkMRMLNode* volumeNode);
  void setVolumeInWidget(qSlicerWidget* displayWidget, vtkMRMLVolumeNode* volumeNode);
  vtkMRMLVolumeNode* volumeInWidget(qSlicerWidget* displayWidget);

  // show the selected widget, hide all others
  void setCurrentDisplayWidget(qSlicerWidget* displayWidget);

  qSlicerWidget* CurrentWidget{ nullptr };

  qSlicerScalarVolumeDisplayWidget*            ScalarVolumeDisplayWidget{ nullptr };
  qSlicerLabelMapVolumeDisplayWidget*          LabelMapVolumeDisplayWidget{ nullptr };
  qSlicerDiffusionWeightedVolumeDisplayWidget* DWVolumeDisplayWidget{ nullptr };
  qSlicerDiffusionTensorVolumeDisplayWidget*   DTVolumeDisplayWidget{ nullptr };
};

// --------------------------------------------------------------------------
qSlicerVolumeDisplayWidgetPrivate::qSlicerVolumeDisplayWidgetPrivate(
  qSlicerVolumeDisplayWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerVolumeDisplayWidgetPrivate::init()
{
  Q_Q(qSlicerVolumeDisplayWidget);

  QVBoxLayout* layout = new QVBoxLayout(q);
  layout->setSpacing(4);
  layout->setContentsMargins(0, 0, 0, 0);

  this->ScalarVolumeDisplayWidget = new qSlicerScalarVolumeDisplayWidget(q);
  this->ScalarVolumeDisplayWidget->hide();
  layout->addWidget(this->ScalarVolumeDisplayWidget);

  this->LabelMapVolumeDisplayWidget = new qSlicerLabelMapVolumeDisplayWidget(q);
  this->LabelMapVolumeDisplayWidget->hide();
  layout->addWidget(this->LabelMapVolumeDisplayWidget);

  this->DWVolumeDisplayWidget = new qSlicerDiffusionWeightedVolumeDisplayWidget(q);
  this->DWVolumeDisplayWidget->hide();
  layout->addWidget(this->DWVolumeDisplayWidget);

  this->DTVolumeDisplayWidget = new qSlicerDiffusionTensorVolumeDisplayWidget(q);
  this->DTVolumeDisplayWidget->hide();
  layout->addWidget(this->DTVolumeDisplayWidget);
}

// --------------------------------------------------------------------------
void qSlicerVolumeDisplayWidgetPrivate::setCurrentDisplayWidget(qSlicerWidget* displayWidget)
{
  if (this->CurrentWidget == displayWidget)
    {
    return;
    }
  if (this->CurrentWidget)
    {
    this->CurrentWidget->hide();
    }
  this->CurrentWidget = displayWidget;
  if (this->CurrentWidget)
    {
    this->CurrentWidget->show();
    }
}

// --------------------------------------------------------------------------
qSlicerWidget* qSlicerVolumeDisplayWidgetPrivate::widgetForVolume(vtkMRMLNode* volumeNode)
{
  // We must check first the most specific volume type and if there is no match
  // then try scalar volume.
  if (vtkMRMLDiffusionTensorVolumeNode::SafeDownCast(volumeNode))
    {
    return this->DTVolumeDisplayWidget;
    }
  else if (vtkMRMLDiffusionWeightedVolumeNode::SafeDownCast(volumeNode))
    {
    return this->DWVolumeDisplayWidget;
    }
  else if (vtkMRMLLabelMapVolumeNode::SafeDownCast(volumeNode))
    {
    return this->LabelMapVolumeDisplayWidget;
    }
  else if (vtkMRMLScalarVolumeNode::SafeDownCast(volumeNode))
    {
    return this->ScalarVolumeDisplayWidget;
    }
  return nullptr;
}

// --------------------------------------------------------------------------
void qSlicerVolumeDisplayWidgetPrivate::setVolumeInWidget(qSlicerWidget* displayWidget, vtkMRMLVolumeNode* volumeNode)
{
  Q_Q(qSlicerVolumeDisplayWidget);
  if (!displayWidget)
    {
    return;
    }
  vtkMRMLScene* scene = volumeNode ? volumeNode->GetScene() : nullptr;
  // We must remove the node "before" the setting the scene to nullptr.
  // Because removing the scene could modify the observed node (e.g setting
  // the scene to 0 on a colortable combobox will set the color node of the
  // observed node to 0.
  if (scene && displayWidget->mrmlScene() != scene)
    {
    // set non-null scene
    displayWidget->setMRMLScene(scene);
    }
  if (displayWidget == this->ScalarVolumeDisplayWidget)
    {
    this->ScalarVolumeDisplayWidget->setMRMLVolumeNode(volumeNode);
    }
  if (displayWidget == this->LabelMapVolumeDisplayWidget)
    {
    this->LabelMapVolumeDisplayWidget->setMRMLVolumeNode(volumeNode);
    }
  if (displayWidget == this->DWVolumeDisplayWidget)
    {
    this->DWVolumeDisplayWidget->setMRMLVolumeNode(volumeNode);
    }
  if (displayWidget == this->DTVolumeDisplayWidget)
    {
    this->DTVolumeDisplayWidget->setMRMLVolumeNode(volumeNode);
    }
  if (!scene && displayWidget->mrmlScene() != scene)
    {
    // remove scene after the node has been removed
    displayWidget->setMRMLScene(scene);
    }
}

// --------------------------------------------------------------------------
vtkMRMLVolumeNode* qSlicerVolumeDisplayWidgetPrivate::volumeInWidget(qSlicerWidget* displayWidget)
{
  Q_Q(qSlicerVolumeDisplayWidget);
  if (displayWidget == this->ScalarVolumeDisplayWidget)
    {
    return this->ScalarVolumeDisplayWidget->volumeNode();
    }
  if (displayWidget == this->LabelMapVolumeDisplayWidget)
    {
    return this->LabelMapVolumeDisplayWidget->volumeNode();
    }
  if (displayWidget == this->DWVolumeDisplayWidget)
    {
    return this->DWVolumeDisplayWidget->volumeNode();
    }
  if (displayWidget == this->DTVolumeDisplayWidget)
    {
    return this->DTVolumeDisplayWidget->volumeNode();
    }
  return nullptr;
}

// --------------------------------------------------------------------------
// qSlicerVolumeDisplayWidget
// --------------------------------------------------------------------------
qSlicerVolumeDisplayWidget::qSlicerVolumeDisplayWidget(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qSlicerVolumeDisplayWidgetPrivate(*this))
{
  Q_D(qSlicerVolumeDisplayWidget);
  d->init();
}

// --------------------------------------------------------------------------
qSlicerVolumeDisplayWidget::~qSlicerVolumeDisplayWidget() = default;

// --------------------------------------------------------------------------
void qSlicerVolumeDisplayWidget::setMRMLVolumeNode(vtkMRMLNode* aVolumeNode)
{
  Q_D(qSlicerVolumeDisplayWidget);
  vtkMRMLVolumeNode* volumeNode = vtkMRMLVolumeNode::SafeDownCast(aVolumeNode);
  qSlicerWidget* newWidget = d->widgetForVolume(volumeNode);

  if (newWidget == d->CurrentWidget)
    {
    d->setVolumeInWidget(d->CurrentWidget, volumeNode);
    return;
    }

  if (d->CurrentWidget)
    {
    d->setVolumeInWidget(d->CurrentWidget, nullptr);
    }
  d->setVolumeInWidget(newWidget, volumeNode);
  d->setCurrentDisplayWidget(newWidget);
}

// --------------------------------------------------------------------------
void qSlicerVolumeDisplayWidget::updateFromMRML(vtkObject* volume)
{
  vtkMRMLVolumeNode* volumeNode = vtkMRMLVolumeNode::SafeDownCast(volume);
  this->setMRMLVolumeNode(volumeNode);
}
