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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes

// qMRML includes
#include "qMRMLAnnotationROIWidget.h"
#include "ui_qMRMLAnnotationROIWidget.h"

// MRML includes
#include <vtkMRMLAnnotationROINode.h>
#include <vtkMRMLDisplayNode.h>

// STD includes
#include <vector>

// 0.001 because the sliders only handle 2 decimals
#define SLIDERS_EPSILON 0.001

// --------------------------------------------------------------------------
class qMRMLAnnotationROIWidgetPrivate: public Ui_qMRMLAnnotationROIWidget
{
  Q_DECLARE_PUBLIC(qMRMLAnnotationROIWidget);
protected:
  qMRMLAnnotationROIWidget* const q_ptr;
public:
  qMRMLAnnotationROIWidgetPrivate(qMRMLAnnotationROIWidget& object);
  void init();

  vtkMRMLAnnotationROINode* ROINode;
  bool IsProcessingOnMRMLNodeModified;
  bool AutoRange;
};

// --------------------------------------------------------------------------
qMRMLAnnotationROIWidgetPrivate::qMRMLAnnotationROIWidgetPrivate(qMRMLAnnotationROIWidget& object)
  : q_ptr(&object)
{
  this->ROINode = nullptr;
  this->IsProcessingOnMRMLNodeModified = false;
  this->AutoRange = true;
}

// --------------------------------------------------------------------------
void qMRMLAnnotationROIWidgetPrivate::init()
{
  Q_Q(qMRMLAnnotationROIWidget);
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
// qMRMLAnnotationROIWidget methods

// --------------------------------------------------------------------------
qMRMLAnnotationROIWidget::qMRMLAnnotationROIWidget(QWidget* _parent)
  : QWidget(_parent)
  , d_ptr(new qMRMLAnnotationROIWidgetPrivate(*this))
{
  Q_D(qMRMLAnnotationROIWidget);
  d->init();
}

// --------------------------------------------------------------------------
qMRMLAnnotationROIWidget::~qMRMLAnnotationROIWidget() = default;

// --------------------------------------------------------------------------
vtkMRMLAnnotationROINode* qMRMLAnnotationROIWidget::mrmlROINode()const
{
  Q_D(const qMRMLAnnotationROIWidget);
  return d->ROINode;
}

// --------------------------------------------------------------------------
void qMRMLAnnotationROIWidget::setMRMLAnnotationROINode(vtkMRMLAnnotationROINode* roiNode)
{
  Q_D(qMRMLAnnotationROIWidget);

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
void qMRMLAnnotationROIWidget::setMRMLAnnotationROINode(vtkMRMLNode* roiNode)
{
  this->setMRMLAnnotationROINode(vtkMRMLAnnotationROINode::SafeDownCast(roiNode));
}

// --------------------------------------------------------------------------
void qMRMLAnnotationROIWidget::onMRMLNodeModified()
{
  Q_D(qMRMLAnnotationROIWidget);

  if (!d->ROINode)
    {
    return;
    }

  d->IsProcessingOnMRMLNodeModified = true;

  // Interactive Mode
  bool interactive = d->ROINode->GetInteractiveMode();
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
void qMRMLAnnotationROIWidget::setExtent(double min, double max)
{
  this->setExtent(min, max, min, max, min, max);
}

// --------------------------------------------------------------------------
void qMRMLAnnotationROIWidget::setExtent(double minLR, double maxLR,
                                         double minPA, double maxPA,
                                         double minIS, double maxIS)
{
  Q_D(qMRMLAnnotationROIWidget);
  d->LRRangeWidget->setRange(minLR, maxLR);
  d->PARangeWidget->setRange(minPA, maxPA);
  d->ISRangeWidget->setRange(minIS, maxIS);
}

// --------------------------------------------------------------------------
void qMRMLAnnotationROIWidget::setDisplayClippingBox(bool visible)
{
  Q_D(qMRMLAnnotationROIWidget);

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
void qMRMLAnnotationROIWidget::setInteractiveMode(bool interactive)
{
  Q_D(qMRMLAnnotationROIWidget);
  d->ROINode->SetInteractiveMode(interactive);
}

// --------------------------------------------------------------------------
void qMRMLAnnotationROIWidget::updateROI()
{
  Q_D(qMRMLAnnotationROIWidget);

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

  int wasModifying = d->ROINode->StartModify();
  d->ROINode->SetXYZ(0.5*(bounds[1]+bounds[0]),
                     0.5*(bounds[3]+bounds[2]),
                     0.5*(bounds[5]+bounds[4]));
  d->ROINode->SetRadiusXYZ(0.5*(bounds[1]-bounds[0]),
                           0.5*(bounds[3]-bounds[2]),
                           0.5*(bounds[5]-bounds[4]));
  d->ROINode->EndModify(wasModifying);
}

// --------------------------------------------------------------------------
void qMRMLAnnotationROIWidget::onMRMLDisplayNodeModified()
{
  Q_D(qMRMLAnnotationROIWidget);

  if (!d->ROINode)
    {
    return;
    }

  // Visibility
  d->DisplayClippingBoxButton->setChecked(d->ROINode->GetDisplayVisibility());
}
