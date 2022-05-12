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
// qMRML includes
#include "qMRMLMarkupsROIWidget.h"
#include "ui_qMRMLMarkupsROIWidget.h"

// MRML includes
#include <vtkMRMLMarkupsROINode.h>
#include <vtkMRMLMarkupsROIDisplayNode.h>

// VTK includes
#include <vtkWeakPointer.h>

// STD includes
#include <vector>

// 0.001 because the sliders only handle 2 decimals
#define SLIDERS_EPSILON 0.001

// --------------------------------------------------------------------------
class qMRMLMarkupsROIWidgetPrivate:
  public Ui_qMRMLMarkupsROIWidget
{

public:
  qMRMLMarkupsROIWidgetPrivate(qMRMLMarkupsROIWidget &widget);
  void setupUi(qMRMLMarkupsROIWidget* widget);

  bool IsProcessingOnMRMLNodeModified;
  bool AutoRange;

protected:
  qMRMLMarkupsROIWidget* const q_ptr;

private:
  Q_DECLARE_PUBLIC(qMRMLMarkupsROIWidget);

};

// --------------------------------------------------------------------------
qMRMLMarkupsROIWidgetPrivate::qMRMLMarkupsROIWidgetPrivate(qMRMLMarkupsROIWidget& widget)
  : q_ptr(&widget)
{
  this->IsProcessingOnMRMLNodeModified = false;
  this->AutoRange = true;
}

// --------------------------------------------------------------------------
void qMRMLMarkupsROIWidgetPrivate::setupUi(qMRMLMarkupsROIWidget* widget)
{
  Q_Q(qMRMLMarkupsROIWidget);

  this->Ui_qMRMLMarkupsROIWidget::setupUi(widget);

  this->roiTypeComboBox->clear();
  for (int roiType = 0; roiType < vtkMRMLMarkupsROINode::ROIType_Last; ++roiType)
    {
    this->roiTypeComboBox->addItem(vtkMRMLMarkupsROINode::GetROITypeAsString(roiType), roiType);
    }

  QObject::connect(this->roiTypeComboBox, SIGNAL(currentIndexChanged(int)),
                   q, SLOT(onROITypeParameterChanged()));
  QObject::connect(this->insideOutCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(setInsideOut(bool)));
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
  q->setEnabled(q->MarkupsNode != nullptr);
}

// --------------------------------------------------------------------------
// qMRMLMarkupsROIWidget methods

// --------------------------------------------------------------------------
qMRMLMarkupsROIWidget::qMRMLMarkupsROIWidget(QWidget* parent)
  : Superclass(parent), d_ptr(new qMRMLMarkupsROIWidgetPrivate(*this))
{
  this->setup();
}

// --------------------------------------------------------------------------
qMRMLMarkupsROIWidget::~qMRMLMarkupsROIWidget() = default;

// --------------------------------------------------------------------------
void qMRMLMarkupsROIWidget::setup()
{
  Q_D(qMRMLMarkupsROIWidget);
  d->setupUi(this);
}

// --------------------------------------------------------------------------
vtkMRMLMarkupsROINode* qMRMLMarkupsROIWidget::mrmlROINode() const
{
  return vtkMRMLMarkupsROINode::SafeDownCast(this->MarkupsNode);
}


// --------------------------------------------------------------------------
void qMRMLMarkupsROIWidget::setExtent(double min, double max)
{
  this->setExtent(min, max, min, max, min, max);
}

// --------------------------------------------------------------------------
void qMRMLMarkupsROIWidget::setExtent(double minLR, double maxLR,
                                        double minPA, double maxPA,
                                        double minIS, double maxIS)
{
  Q_D(qMRMLMarkupsROIWidget);

  d->LRRangeWidget->setRange(minLR, maxLR);
  d->PARangeWidget->setRange(minPA, maxPA);
  d->ISRangeWidget->setRange(minIS, maxIS);
}


// --------------------------------------------------------------------------
void qMRMLMarkupsROIWidget::setDisplayClippingBox(bool visible)
{
  auto roiNode = vtkMRMLMarkupsROINode::SafeDownCast(this->MarkupsNode);
  if (!roiNode)
    {
    return;
    }

  int numberOfDisplayNodes = roiNode->GetNumberOfDisplayNodes();

  std::vector<int> wasModifying(numberOfDisplayNodes);
  for(int index = 0; index < numberOfDisplayNodes; index++)
    {
    vtkMRMLDisplayNode* displayNode = roiNode->GetNthDisplayNode(index);
    if (!displayNode)
      {
      continue;
      }
    wasModifying[index] = displayNode->StartModify();
    }

  this->MarkupsNode->SetDisplayVisibility(visible);

  for(int index = 0; index < numberOfDisplayNodes; index++)
    {
    vtkMRMLDisplayNode* displayNode = roiNode->GetNthDisplayNode(index);
    if (!displayNode)
      {
      continue;
      }
    displayNode->EndModify(wasModifying[index]);
    }
}

// --------------------------------------------------------------------------
void qMRMLMarkupsROIWidget::setInteractiveMode(bool interactive)
{
  auto roiNode = vtkMRMLMarkupsROINode::SafeDownCast(this->MarkupsNode);
  if (!roiNode)
    {
    return;
    }

  auto roiDisplayNode = vtkMRMLMarkupsROIDisplayNode::SafeDownCast(roiNode->GetDisplayNode());
  if (!roiDisplayNode)
    {
    roiNode->CreateDefaultDisplayNodes();
    }

  roiDisplayNode = vtkMRMLMarkupsROIDisplayNode::SafeDownCast(roiNode->GetDisplayNode());
  roiDisplayNode->SetHandlesInteractive(interactive);
}

// --------------------------------------------------------------------------
void qMRMLMarkupsROIWidget::updateROI()
{
  Q_D(qMRMLMarkupsROIWidget);

  // Ignore the calls from onMRMLNodeModified() as it
  // could set the node in an inconsistent state (except for
  // ISRangeWidget->setValues()).
  if (d->IsProcessingOnMRMLNodeModified)
    {
    return;
    }

  auto roiNode = vtkMRMLMarkupsROINode::SafeDownCast(this->MarkupsNode);
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
void qMRMLMarkupsROIWidget::setMRMLMarkupsNode(vtkMRMLMarkupsNode* markupsNode)
{
  this->qvtkReconnect(this->MarkupsNode, markupsNode, vtkCommand::ModifiedEvent,
                      this, SLOT(updateWidgetFromMRML()));

  this->qvtkReconnect(this->MarkupsNode, markupsNode, vtkMRMLDisplayableNode::DisplayModifiedEvent,
                      this, SLOT(onMRMLDisplayNodeModified()));

  this->MarkupsNode = markupsNode;

  this->updateWidgetFromMRML();

  this->onMRMLDisplayNodeModified();
  this->setEnabled(markupsNode != nullptr);
}

// --------------------------------------------------------------------------
void qMRMLMarkupsROIWidget::onMRMLDisplayNodeModified()
{
  Q_D(qMRMLMarkupsROIWidget);

  auto roiNode = vtkMRMLMarkupsROINode::SafeDownCast(this->MarkupsNode);
  if (!roiNode)
    {
    return;
    }

  // Visibility
  d->DisplayClippingBoxButton->setChecked(roiNode->GetDisplayVisibility());
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsROIWidget::onROITypeParameterChanged()
{
  Q_D(qMRMLMarkupsROIWidget);

  auto roiNode = vtkMRMLMarkupsROINode::SafeDownCast(this->MarkupsNode);
  if (!roiNode)
    {
    return;
    }

  MRMLNodeModifyBlocker blocker(roiNode);
  roiNode->SetROIType(d->roiTypeComboBox->currentData().toInt());
}

//-----------------------------------------------------------------------------
bool qMRMLMarkupsROIWidget::insideOut()
{
  Q_D(qMRMLMarkupsROIWidget);

  auto roiNode = vtkMRMLMarkupsROINode::SafeDownCast(this->MarkupsNode);
  if (!roiNode)
    {
    return false;
    }
  return roiNode->GetInsideOut();
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsROIWidget::setInsideOut(bool insideOut)
{
  Q_D(qMRMLMarkupsROIWidget);

  auto roiNode = vtkMRMLMarkupsROINode::SafeDownCast(this->MarkupsNode);
  if (!roiNode)
    {
    return;
    }

  MRMLNodeModifyBlocker blocker(roiNode);
  roiNode->SetInsideOut(insideOut);
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsROIWidget::updateWidgetFromMRML()
{
  Q_D(qMRMLMarkupsROIWidget);

  auto roiNode = vtkMRMLMarkupsROINode::SafeDownCast(this->MarkupsNode);
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

  bool wasBlocked = d->roiTypeComboBox->blockSignals(true);
  d->roiTypeComboBox->setCurrentIndex(d->roiTypeComboBox->findData(roiNode->GetROIType()));
  d->roiTypeComboBox->blockSignals(wasBlocked);

  wasBlocked = d->insideOutCheckBox->blockSignals(true);
  d->insideOutCheckBox->setChecked(roiNode->GetInsideOut());
  d->insideOutCheckBox->blockSignals(wasBlocked);
}

//-----------------------------------------------------------------------------
bool qMRMLMarkupsROIWidget::canManageMRMLMarkupsNode(vtkMRMLMarkupsNode *markupsNode) const
{
  auto roiNode = vtkMRMLMarkupsROINode::SafeDownCast(markupsNode);
  if (!roiNode)
    {
    return false;
    }

  return true;
}
