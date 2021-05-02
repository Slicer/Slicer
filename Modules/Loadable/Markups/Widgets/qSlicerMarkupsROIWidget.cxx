/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, Cancer
  Care Ontario, OpenAnatomy, and Brigham and Women's Hospital through NIH grant R01MH112748.

  ==============================================================================*/

// Markups widgets includes
#include "qSlicerMarkupsAdditionalOptionsWidget_p.h"

// qMRML includes
#include "qSlicerMarkupsROIWidget.h"
#include "ui_qSlicerMarkupsROIWidget.h"

// MRML includes
#include <vtkMRMLMarkupsROINode.h>
#include <vtkMRMLDisplayNode.h>

// STD includes
#include <vector>

// 0.001 because the sliders only handle 2 decimals
#define SLIDERS_EPSILON 0.001

// --------------------------------------------------------------------------
class qSlicerMarkupsROIWidgetPrivate:
  public qSlicerMarkupsAdditionalOptionsWidgetPrivate,
  public Ui_qSlicerMarkupsROIWidget
{
  Q_DECLARE_PUBLIC(qSlicerMarkupsROIWidget);
protected:
  qSlicerMarkupsROIWidget* const q_ptr;
public:

  qSlicerMarkupsROIWidgetPrivate(qSlicerMarkupsROIWidget* object);
  void setupUi(qSlicerMarkupsROIWidget* widget);

  bool IsProcessingOnMRMLNodeModified;
  bool AutoRange;
};

// --------------------------------------------------------------------------
qSlicerMarkupsROIWidgetPrivate::qSlicerMarkupsROIWidgetPrivate(qSlicerMarkupsROIWidget* object)
  : q_ptr(object)
{
  this->MarkupsNode = nullptr;
  this->IsProcessingOnMRMLNodeModified = false;
  this->AutoRange = true;
}

// --------------------------------------------------------------------------
void qSlicerMarkupsROIWidgetPrivate::setupUi(qSlicerMarkupsROIWidget* widget)
{
  Q_Q(qSlicerMarkupsROIWidget);

  this->Ui_qSlicerMarkupsROIWidget::setupUi(widget);

  this->roiSettingsCollapseButton->setVisible(false);
  this->roiTypeComboBox->clear();
  for (int roiType = 0; roiType < vtkMRMLMarkupsROINode::ROIType_Last; ++roiType)
    {
    this->roiTypeComboBox->addItem(vtkMRMLMarkupsROINode::GetROITypeAsString(roiType), roiType);
    }

  QObject::connect(this->roiTypeComboBox, SIGNAL(currentIndexChanged(int)),
                   q, SLOT(onROITypeParameterChanged()));
  QObject::connect(this->DisplayClippingBoxButton, SIGNAL(toggled(bool)),
                   q, SLOT(setDisplayClippingBox(bool)));
  QObject::connect(this->InteractiveModeCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(setInteractiveMode(bool)));
  QObject::connect(this->LRRangeWidget, SIGNAL(valuesChanged(double,double)),
                   q, SLOT(updateROI()));
  QObject::connect(this->PARangeWidget, SIGNAL(valuesChanged(double,double)),
                   q, SLOT(updateROI()));
  QObject::connect(this->ISRangeWidget, SIGNAL(valuesChanged(double,double)),
                   q, SLOT(updateROI()));
  q->setEnabled(this->MarkupsNode != nullptr);
}

// --------------------------------------------------------------------------
// qSlicerMarkupsROIWidget methods

// --------------------------------------------------------------------------
qSlicerMarkupsROIWidget::
qSlicerMarkupsROIWidget(QWidget* parent)
  : Superclass(*new qSlicerMarkupsROIWidgetPrivate(this), parent)
{
  this->setup();
}

// --------------------------------------------------------------------------
qSlicerMarkupsROIWidget::
qSlicerMarkupsROIWidget(qSlicerMarkupsROIWidgetPrivate &d, QWidget* parent)
  : Superclass(d, parent)
{
  this->setup();
}

// --------------------------------------------------------------------------
qSlicerMarkupsROIWidget::~qSlicerMarkupsROIWidget() = default;

// --------------------------------------------------------------------------
void qSlicerMarkupsROIWidget::setup()
{
  Q_D(qSlicerMarkupsROIWidget);
  d->setupUi(this);
}

// --------------------------------------------------------------------------
vtkMRMLMarkupsROINode* qSlicerMarkupsROIWidget::mrmlROINode()const
{
  Q_D(const qSlicerMarkupsROIWidget);
  return vtkMRMLMarkupsROINode::SafeDownCast(d->MarkupsNode);
}

// --------------------------------------------------------------------------
void qSlicerMarkupsROIWidget::setMRMLMarkupsNode(vtkMRMLMarkupsNode* markupsNode)
{
  Q_D(qSlicerMarkupsROIWidget);

  Superclass::setMRMLMarkupsNode(markupsNode);

  this->qvtkReconnect(d->MarkupsNode, markupsNode, vtkCommand::ModifiedEvent,
                      this, SLOT(onMRMLNodeModified()));

  this->qvtkReconnect(d->MarkupsNode, markupsNode, vtkMRMLDisplayableNode::DisplayModifiedEvent,
                      this, SLOT(onMRMLDisplayNodeModified()));

  this->onMRMLNodeModified();
  this->onMRMLDisplayNodeModified();
  this->setEnabled(markupsNode != nullptr);
}

// --------------------------------------------------------------------------
void qSlicerMarkupsROIWidget::setMRMLMarkupsNode(vtkMRMLNode* node)
{
  this->setMRMLMarkupsNode(vtkMRMLMarkupsROINode::SafeDownCast(node));
}

// --------------------------------------------------------------------------
void qSlicerMarkupsROIWidget::onMRMLNodeModified()
{
  Q_D(qSlicerMarkupsROIWidget);

  vtkMRMLMarkupsROINode* roiNode = vtkMRMLMarkupsROINode::SafeDownCast(d->MarkupsNode);
  if (!roiNode)
    {
    return;
    }

  d->IsProcessingOnMRMLNodeModified = true;

  // Interactive Mode
  bool interactive = false;
  if (roiNode->GetDisplayNode())
    {
    interactive = vtkMRMLMarkupsDisplayNode::SafeDownCast(roiNode->GetDisplayNode())->GetHandlesInteractive();
    }
  d->LRRangeWidget->setTracking(interactive);
  d->PARangeWidget->setTracking(interactive);
  d->ISRangeWidget->setTracking(interactive);
  d->InteractiveModeCheckBox->setChecked(interactive);

  // ROI
  double xyz[3];
  double rxyz[3];

  roiNode->GetXYZ(xyz);
  roiNode->GetRadiusXYZ(rxyz);

  double bounds[6];
  for (int i=0; i < 3; ++i)
    {
    bounds[i]   = xyz[i]-rxyz[i];
    bounds[3+i] = xyz[i]+rxyz[i];
    }

  if (d->AutoRange)
    {
    d->LRRangeWidget->setRange(
      qMin(bounds[0], d->LRRangeWidget->minimum()),
      qMax(bounds[3], d->LRRangeWidget->maximum()));
    d->PARangeWidget->setRange(
      qMin(bounds[1], d->PARangeWidget->minimum()),
      qMax(bounds[4], d->PARangeWidget->maximum()));
    d->ISRangeWidget->setRange(
      qMin(bounds[2], d->ISRangeWidget->minimum()),
      qMax(bounds[5], d->ISRangeWidget->maximum()));
    }

  d->LRRangeWidget->setValues(bounds[0], bounds[3]);
  d->PARangeWidget->setValues(bounds[1], bounds[4]);
  d->ISRangeWidget->setValues(bounds[2], bounds[5]);

  d->IsProcessingOnMRMLNodeModified = false;
}

// --------------------------------------------------------------------------
void qSlicerMarkupsROIWidget::setExtent(double min, double max)
{
  this->setExtent(min, max, min, max, min, max);
}

// --------------------------------------------------------------------------
void qSlicerMarkupsROIWidget::setExtent(double minLR, double maxLR,
                                        double minPA, double maxPA,
                                        double minIS, double maxIS)
{
  Q_D(qSlicerMarkupsROIWidget);
  d->LRRangeWidget->setRange(minLR, maxLR);
  d->PARangeWidget->setRange(minPA, maxPA);
  d->ISRangeWidget->setRange(minIS, maxIS);
}

// --------------------------------------------------------------------------
void qSlicerMarkupsROIWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerMarkupsROIWidget);

  if (!this->canManageMRMLMarkupsNode(d->MarkupsNode))
    {
    d->roiSettingsCollapseButton->setVisible(false);
    return;
    }

  d->roiSettingsCollapseButton->setVisible(true);
  vtkMRMLMarkupsROINode* markupsROINode = vtkMRMLMarkupsROINode::SafeDownCast(d->MarkupsNode);
  if (markupsROINode)
    {
    bool wasBlocked = d->roiTypeComboBox->blockSignals(true);
    d->roiTypeComboBox->setCurrentIndex(d->roiTypeComboBox->findData(markupsROINode->GetROIType()));
    d->roiTypeComboBox->blockSignals(wasBlocked);
    this->setMRMLMarkupsNode(markupsROINode);
    }
}

// --------------------------------------------------------------------------
void qSlicerMarkupsROIWidget::setDisplayClippingBox(bool visible)
{
  Q_D(qSlicerMarkupsROIWidget);

  int numberOfDisplayNodes = d->MarkupsNode->GetNumberOfDisplayNodes();

  std::vector<int> wasModifying(numberOfDisplayNodes);
  for(int index = 0; index < numberOfDisplayNodes; index++)
    {
    vtkMRMLDisplayNode* displayNode = d->MarkupsNode->GetNthDisplayNode(index);
    if (!displayNode)
      {
      continue;
      }
    wasModifying[index] = displayNode->StartModify();
    }

  d->MarkupsNode->SetDisplayVisibility(visible);

  for(int index = 0; index < numberOfDisplayNodes; index++)
    {
    vtkMRMLDisplayNode* displayNode = d->MarkupsNode->GetNthDisplayNode(index);
    if (!displayNode)
      {
      continue;
      }
    displayNode->EndModify(wasModifying[index]);
    }
}

// --------------------------------------------------------------------------
void qSlicerMarkupsROIWidget::setInteractiveMode(bool interactive)
{
  Q_D(qSlicerMarkupsROIWidget);
  if (!d->MarkupsNode->GetDisplayNode())
    {
    d->MarkupsNode->CreateDefaultDisplayNodes();
    }
  vtkMRMLMarkupsDisplayNode::SafeDownCast(d->MarkupsNode->GetDisplayNode())->SetHandlesInteractive(interactive);
}

// --------------------------------------------------------------------------
void qSlicerMarkupsROIWidget::updateROI()
{
  Q_D(qSlicerMarkupsROIWidget);

  // Ignore the calls from onMRMLNodeModified() as it
  // could set the node in an inconsistent state (except for
  // ISRangeWidget->setValues()).
  if (d->IsProcessingOnMRMLNodeModified)
    {
    return;
    }

  vtkMRMLMarkupsROINode* roiNode = vtkMRMLMarkupsROINode::SafeDownCast(d->MarkupsNode);
  if (!roiNode)
    {
    return;
    }

  double bounds[6];
  d->LRRangeWidget->values(bounds[0],bounds[1]);
  d->PARangeWidget->values(bounds[2],bounds[3]);
  d->ISRangeWidget->values(bounds[4],bounds[5]);

  MRMLNodeModifyBlocker blocker(roiNode);
  roiNode->SetXYZ(0.5*(bounds[1]+bounds[0]),
                  0.5*(bounds[3]+bounds[2]),
                  0.5*(bounds[5]+bounds[4]));
  roiNode->SetRadiusXYZ(0.5*(bounds[1]-bounds[0]),
                        0.5*(bounds[3]-bounds[2]),
                        0.5*(bounds[5]-bounds[4]));
}

// --------------------------------------------------------------------------
void qSlicerMarkupsROIWidget::onMRMLDisplayNodeModified()
{
  Q_D(qSlicerMarkupsROIWidget);

  if (!d->MarkupsNode)
    {
    return;
    }

  // Visibility
  d->DisplayClippingBoxButton->setChecked(d->MarkupsNode->GetDisplayVisibility());
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsROIWidget::onROITypeParameterChanged()
{
  Q_D(qSlicerMarkupsROIWidget);
  vtkMRMLMarkupsROINode* roiNode = vtkMRMLMarkupsROINode::SafeDownCast(d->MarkupsNode);
  if (!roiNode)
    {
    return;
    }
  MRMLNodeModifyBlocker blocker(roiNode);
  roiNode->SetROIType(d->roiTypeComboBox->currentData().toInt());
}

//-----------------------------------------------------------------------------
bool qSlicerMarkupsROIWidget::canManageMRMLMarkupsNode(vtkMRMLMarkupsNode *markupsNode) const
{
  Q_D(const qSlicerMarkupsROIWidget);

  vtkMRMLMarkupsROINode* roiNode = vtkMRMLMarkupsROINode::SafeDownCast(markupsNode);
  if (!roiNode)
    {
    return false;
    }

  return true;
}
