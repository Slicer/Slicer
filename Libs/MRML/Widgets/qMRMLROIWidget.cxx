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
#include "qMRMLROIWidget.h"
#include "ui_qMRMLROIWidget.h"

// MRML includes
#include <vtkMRMLROINode.h>

// --------------------------------------------------------------------------
class qMRMLROIWidgetPrivate: public Ui_qMRMLROIWidget
{
  Q_DECLARE_PUBLIC(qMRMLROIWidget);
protected:
  qMRMLROIWidget* const q_ptr;
public:
  qMRMLROIWidgetPrivate(qMRMLROIWidget& object);
  void init();
  vtkMRMLROINode* ROINode;
};

// --------------------------------------------------------------------------
qMRMLROIWidgetPrivate::qMRMLROIWidgetPrivate(qMRMLROIWidget& object)
  : q_ptr(&object)
{
  this->ROINode = nullptr;
}

// --------------------------------------------------------------------------
void qMRMLROIWidgetPrivate::init()
{
  Q_Q(qMRMLROIWidget);
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
// qMRMLROIWidget methods

// --------------------------------------------------------------------------
qMRMLROIWidget::qMRMLROIWidget(QWidget* _parent)
  : QWidget(_parent)
  , d_ptr(new qMRMLROIWidgetPrivate(*this))
{
  Q_D(qMRMLROIWidget);
  d->init();
}

// --------------------------------------------------------------------------
qMRMLROIWidget::~qMRMLROIWidget() = default;

// --------------------------------------------------------------------------
vtkMRMLROINode* qMRMLROIWidget::mrmlROINode()const
{
  Q_D(const qMRMLROIWidget);
  return d->ROINode;
}

// --------------------------------------------------------------------------
void qMRMLROIWidget::setMRMLROINode(vtkMRMLROINode* roiNode)
{
  Q_D(qMRMLROIWidget);
  qvtkReconnect(d->ROINode, roiNode, vtkCommand::ModifiedEvent,
                this, SLOT(onMRMLNodeModified()));

  d->ROINode = roiNode;
  this->onMRMLNodeModified();
  this->setEnabled(roiNode != nullptr);
}

// --------------------------------------------------------------------------
void qMRMLROIWidget::setMRMLROINode(vtkMRMLNode* roiNode)
{
  this->setMRMLROINode(vtkMRMLROINode::SafeDownCast(roiNode));
}

// --------------------------------------------------------------------------
void qMRMLROIWidget::onMRMLNodeModified()
{
  Q_D(qMRMLROIWidget);
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
  Q_D(qMRMLROIWidget);
  d->LRRangeWidget->setRange(min, max);
  d->PARangeWidget->setRange(min, max);
  d->ISRangeWidget->setRange(min, max);
}

// --------------------------------------------------------------------------
void qMRMLROIWidget::setDisplayClippingBox(bool visible)
{
  Q_D(qMRMLROIWidget);
  d->ROINode->SetVisibility(visible);
}

// --------------------------------------------------------------------------
void qMRMLROIWidget::setInteractiveMode(bool interactive)
{
  Q_D(qMRMLROIWidget);
  d->ROINode->SetInteractiveMode(interactive);
}

// --------------------------------------------------------------------------
void qMRMLROIWidget::updateROI()
{
  Q_D(qMRMLROIWidget);
  double bounds[6];
  d->LRRangeWidget->values(bounds[0],bounds[1]);
  d->PARangeWidget->values(bounds[2],bounds[3]);
  d->ISRangeWidget->values(bounds[4],bounds[5]);

  int disabledModify = d->ROINode->StartModify();
  
  d->ROINode->SetXYZ(0.5*(bounds[1]+bounds[0]),
                     0.5*(bounds[3]+bounds[2]),
                     0.5*(bounds[5]+bounds[4]));
  d->ROINode->SetRadiusXYZ(0.5*(bounds[1]-bounds[0]),
                           0.5*(bounds[3]-bounds[2]),
                           0.5*(bounds[5]-bounds[4]));
  d->ROINode->EndModify(disabledModify);
}
