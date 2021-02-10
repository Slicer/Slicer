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

// Qt includes

// qMRML includes
#include "qMRMLMarkupsROIWidget.h"
#include "ui_qMRMLMarkupsROIWidget.h"

// MRML includes
#include <vtkMRMLMarkupsROINode.h>
#include <vtkMRMLDisplayNode.h>

// STD includes
#include <vector>

// 0.001 because the sliders only handle 2 decimals
#define SLIDERS_EPSILON 0.001

// --------------------------------------------------------------------------
class qMRMLMarkupsROIWidgetPrivate: public Ui_qMRMLMarkupsROIWidget
{
  Q_DECLARE_PUBLIC(qMRMLMarkupsROIWidget);
protected:
  qMRMLMarkupsROIWidget* const q_ptr;
public:
  qMRMLMarkupsROIWidgetPrivate(qMRMLMarkupsROIWidget& object);
  void init();

  vtkMRMLMarkupsROINode* ROINode;
  bool IsProcessingOnMRMLNodeModified;
  bool AutoRange;
};

// --------------------------------------------------------------------------
qMRMLMarkupsROIWidgetPrivate::qMRMLMarkupsROIWidgetPrivate(qMRMLMarkupsROIWidget& object)
  : q_ptr(&object)
{
  this->ROINode = nullptr;
  this->IsProcessingOnMRMLNodeModified = false;
  this->AutoRange = true;
}

// --------------------------------------------------------------------------
void qMRMLMarkupsROIWidgetPrivate::init()
{
  Q_Q(qMRMLMarkupsROIWidget);
  this->setupUi(q);
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
  q->setEnabled(this->ROINode != nullptr);
}

// --------------------------------------------------------------------------
// qMRMLMarkupsROIWidget methods

// --------------------------------------------------------------------------
qMRMLMarkupsROIWidget::qMRMLMarkupsROIWidget(QWidget* _parent)
  : QWidget(_parent)
  , d_ptr(new qMRMLMarkupsROIWidgetPrivate(*this))
{
  Q_D(qMRMLMarkupsROIWidget);
  d->init();
}

// --------------------------------------------------------------------------
qMRMLMarkupsROIWidget::~qMRMLMarkupsROIWidget() = default;

// --------------------------------------------------------------------------
vtkMRMLMarkupsROINode* qMRMLMarkupsROIWidget::mrmlROINode()const
{
  Q_D(const qMRMLMarkupsROIWidget);
  return d->ROINode;
}

// --------------------------------------------------------------------------
void qMRMLMarkupsROIWidget::setMRMLMarkupsROINode(vtkMRMLMarkupsROINode* roiNode)
{
  Q_D(qMRMLMarkupsROIWidget);

  this->qvtkReconnect(d->ROINode, roiNode, vtkCommand::ModifiedEvent,
                this, SLOT(onMRMLNodeModified()));

  this->qvtkReconnect(d->ROINode, roiNode, vtkMRMLDisplayableNode::DisplayModifiedEvent,
                      this, SLOT(onMRMLDisplayNodeModified()));

  d->ROINode = roiNode;

  this->onMRMLNodeModified();
  this->onMRMLDisplayNodeModified();
  this->setEnabled(roiNode != nullptr);
}

// --------------------------------------------------------------------------
void qMRMLMarkupsROIWidget::setMRMLMarkupsROINode(vtkMRMLNode* roiNode)
{
  this->setMRMLMarkupsROINode(vtkMRMLMarkupsROINode::SafeDownCast(roiNode));
}

// --------------------------------------------------------------------------
void qMRMLMarkupsROIWidget::onMRMLNodeModified()
{
  Q_D(qMRMLMarkupsROIWidget);

  if (!d->ROINode)
    {
    return;
    }

  d->IsProcessingOnMRMLNodeModified = true;

  // Interactive Mode
  bool interactive = false;
  if (d->ROINode->GetDisplayNode())
    {
    interactive = vtkMRMLMarkupsDisplayNode::SafeDownCast(d->ROINode->GetDisplayNode())->GetHandlesInteractive();
    }
  d->LRRangeWidget->setTracking(interactive);
  d->PARangeWidget->setTracking(interactive);
  d->ISRangeWidget->setTracking(interactive);
  d->InteractiveModeCheckBox->setChecked(interactive);

  // ROI
  double xyz[3];
  double rxyz[3];

  d->ROINode->GetXYZ(xyz);
  d->ROINode->GetRadiusXYZ(rxyz);

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
  Q_D(qMRMLMarkupsROIWidget);

  int numberOfDisplayNodes = d->ROINode->GetNumberOfDisplayNodes();

  std::vector<int> wasModifying(numberOfDisplayNodes);
  for(int index = 0; index < numberOfDisplayNodes; index++)
    {
    wasModifying[index] = d->ROINode->GetNthDisplayNode(index)->StartModify();
    }

  d->ROINode->SetDisplayVisibility(visible);

  for(int index = 0; index < numberOfDisplayNodes; index++)
    {
    d->ROINode->GetNthDisplayNode(index)->EndModify(wasModifying[index]);
    }
}

// --------------------------------------------------------------------------
void qMRMLMarkupsROIWidget::setInteractiveMode(bool interactive)
{
  Q_D(qMRMLMarkupsROIWidget);
  if (!d->ROINode->GetDisplayNode())
    {
    d->ROINode->CreateDefaultDisplayNodes();
    }
  vtkMRMLMarkupsDisplayNode::SafeDownCast(d->ROINode->GetDisplayNode())->SetHandlesInteractive(interactive);
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

  double bounds[6];
  d->LRRangeWidget->values(bounds[0],bounds[1]);
  d->PARangeWidget->values(bounds[2],bounds[3]);
  d->ISRangeWidget->values(bounds[4],bounds[5]);

  MRMLNodeModifyBlocker blocker(d->ROINode);
  d->ROINode->SetXYZ(0.5*(bounds[1]+bounds[0]),
                     0.5*(bounds[3]+bounds[2]),
                     0.5*(bounds[5]+bounds[4]));
  d->ROINode->SetRadiusXYZ(0.5*(bounds[1]-bounds[0]),
                           0.5*(bounds[3]-bounds[2]),
                           0.5*(bounds[5]-bounds[4]));
}

// --------------------------------------------------------------------------
void qMRMLMarkupsROIWidget::onMRMLDisplayNodeModified()
{
  Q_D(qMRMLMarkupsROIWidget);

  if (!d->ROINode)
    {
    return;
    }

  // Visibility
  d->DisplayClippingBoxButton->setChecked(d->ROINode->GetDisplayVisibility());
}
