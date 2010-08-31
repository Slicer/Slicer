/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
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
#include <QDebug>

// qMRML includes
#include "qMRMLROIWidget.h"
#include "ui_qMRMLROIWidget.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLROINode.h>

// --------------------------------------------------------------------------
class qMRMLROIWidgetPrivate: public ctkPrivate<qMRMLROIWidget>,
                             public Ui_qMRMLROIWidget
{
public:
  CTK_DECLARE_PUBLIC(qMRMLROIWidget);
  qMRMLROIWidgetPrivate();
  void init();
  vtkMRMLROINode* ROINode;
};

// --------------------------------------------------------------------------
qMRMLROIWidgetPrivate::qMRMLROIWidgetPrivate()
{
  this->ROINode = 0;
}

// --------------------------------------------------------------------------
void qMRMLROIWidgetPrivate::init()
{
  CTK_P(qMRMLROIWidget);
  this->setupUi(p);
  QObject::connect(this->DisplayClippingBoxButton, SIGNAL(toggled(bool)),
                   p, SLOT(setDisplayClippingBox(bool)));
  QObject::connect(this->InteractiveModeCheckBox, SIGNAL(toggled(bool)),
                   p, SLOT(setInteractiveMode(bool)));
  QObject::connect(this->LRRangeWidget, SIGNAL(valuesChanged(double, double)),
                   p, SLOT(updateROI()));
  QObject::connect(this->PARangeWidget, SIGNAL(valuesChanged(double, double)),
                   p, SLOT(updateROI()));
  QObject::connect(this->ISRangeWidget, SIGNAL(valuesChanged(double, double)),
                   p, SLOT(updateROI()));
  p->setEnabled(this->ROINode != 0);
}

// --------------------------------------------------------------------------
// qMRMLROIWidget methods

// --------------------------------------------------------------------------
qMRMLROIWidget::qMRMLROIWidget(QWidget* _parent)
  :QWidget(_parent)
{
  CTK_INIT_PRIVATE(qMRMLROIWidget);
  ctk_d()->init();
}

// --------------------------------------------------------------------------
qMRMLROIWidget::~qMRMLROIWidget()
{
}

// --------------------------------------------------------------------------
vtkMRMLROINode* qMRMLROIWidget::mrmlROINode()const
{
  CTK_D(const qMRMLROIWidget);
  return d->ROINode;
}

// --------------------------------------------------------------------------
void qMRMLROIWidget::setMRMLROINode(vtkMRMLROINode* roiNode)
{
  CTK_D(qMRMLROIWidget);
  qvtkReconnect(d->ROINode, roiNode, vtkCommand::ModifiedEvent,
                this, SLOT(onMRMLNodeModified()));

  d->ROINode = roiNode;
  this->onMRMLNodeModified();
  this->setEnabled(roiNode != 0);
}

// --------------------------------------------------------------------------
void qMRMLROIWidget::setMRMLROINode(vtkMRMLNode* roiNode)
{
  this->setMRMLROINode(vtkMRMLROINode::SafeDownCast(roiNode));
}

// --------------------------------------------------------------------------
void qMRMLROIWidget::onMRMLNodeModified()
{
  CTK_D(qMRMLROIWidget);
  if (!d->ROINode)
    {
    return;
    }
  // Visibility
  d->DisplayClippingBoxButton->setChecked(d->ROINode->GetVisibility());

  // Interactive Mode
  bool interactive = d->ROINode->GetInteractiveMode();
  d->LRRangeWidget->setTracking(interactive);
  d->PARangeWidget->setTracking(interactive);
  d->ISRangeWidget->setTracking(interactive);
  d->InteractiveModeCheckBox->setChecked(interactive);

  // ROI
  double *xyz = d->ROINode->GetXYZ();
  double *rxyz = d->ROINode->GetRadiusXYZ();
  double bounds[6];
  for (int i=0; i < 3; ++i)
    {
    bounds[i]   = xyz[i]-rxyz[i];
    bounds[3+i] = xyz[i]+rxyz[i];
    }
  d->LRRangeWidget->setValues(bounds[0], bounds[3]);
  d->PARangeWidget->setValues(bounds[1], bounds[4]);
  d->ISRangeWidget->setValues(bounds[2], bounds[5]);
}

// --------------------------------------------------------------------------
void qMRMLROIWidget::setExtent(double min, double max)
{
  CTK_D(qMRMLROIWidget);
  d->LRRangeWidget->setRange(min, max);
  d->PARangeWidget->setRange(min, max);
  d->ISRangeWidget->setRange(min, max);
}

// --------------------------------------------------------------------------
void qMRMLROIWidget::setDisplayClippingBox(bool visible)
{
  CTK_D(qMRMLROIWidget);
  d->ROINode->SetVisibility(visible);
}

// --------------------------------------------------------------------------
void qMRMLROIWidget::setInteractiveMode(bool interactive)
{
  CTK_D(qMRMLROIWidget);
  d->ROINode->SetInteractiveMode(interactive);
}

// --------------------------------------------------------------------------
void qMRMLROIWidget::updateROI()
{
  CTK_D(qMRMLROIWidget);
  double bounds[6];
  d->LRRangeWidget->values(bounds[0],bounds[1]);
  d->PARangeWidget->values(bounds[2],bounds[3]);
  d->ISRangeWidget->values(bounds[4],bounds[5]);

  d->ROINode->DisableModifiedEventOn();
  d->ROINode->SetXYZ(0.5*(bounds[1]+bounds[0]),
                     0.5*(bounds[3]+bounds[2]),
                     0.5*(bounds[5]+bounds[4]));
  d->ROINode->SetRadiusXYZ(0.5*(bounds[1]-bounds[0]),
                           0.5*(bounds[3]-bounds[2]),
                           0.5*(bounds[5]-bounds[4]));
  d->ROINode->DisableModifiedEventOff();
  d->ROINode->InvokePendingModifiedEvent();
}
