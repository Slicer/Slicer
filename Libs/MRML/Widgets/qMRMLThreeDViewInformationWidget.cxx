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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QDebug>

// qMRML includes
#include "qMRMLThreeDViewInformationWidget_p.h"

// MRML includes
#include <vtkMRMLViewNode.h>

//--------------------------------------------------------------------------
// qMRMLThreeDViewViewPrivate methods

//---------------------------------------------------------------------------
qMRMLThreeDViewInformationWidgetPrivate::qMRMLThreeDViewInformationWidgetPrivate(qMRMLThreeDViewInformationWidget& object)
  : q_ptr(&object)
{
  this->MRMLViewNode = nullptr;
}

//---------------------------------------------------------------------------
qMRMLThreeDViewInformationWidgetPrivate::~qMRMLThreeDViewInformationWidgetPrivate() = default;

//---------------------------------------------------------------------------
void qMRMLThreeDViewInformationWidgetPrivate::setupUi(qMRMLWidget* widget)
{
  Q_Q(qMRMLThreeDViewInformationWidget);

  this->Ui_qMRMLThreeDViewInformationWidget::setupUi(widget);

  this->connect(this->ViewGroupSpinBox, SIGNAL(valueChanged(int)),
    q, SLOT(setViewGroup(int)));
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewInformationWidgetPrivate::updateWidgetFromMRMLViewNode()
{
  Q_Q(qMRMLThreeDViewInformationWidget);

  q->setEnabled(this->MRMLViewNode != nullptr);
  if (this->MRMLViewNode == nullptr)
    {
    return;
    }

  this->LayoutNameLineEdit->setText(this->MRMLViewNode->GetLayoutName());
  this->ViewGroupSpinBox->setValue(this->MRMLViewNode->GetViewGroup());
}

// --------------------------------------------------------------------------
// qMRMLThreeDViewView methods

// --------------------------------------------------------------------------
qMRMLThreeDViewInformationWidget::qMRMLThreeDViewInformationWidget(QWidget* _parent) : Superclass(_parent)
  , d_ptr(new qMRMLThreeDViewInformationWidgetPrivate(*this))
{
  Q_D(qMRMLThreeDViewInformationWidget);
  d->setupUi(this);
  this->setEnabled(false);
}

// --------------------------------------------------------------------------
qMRMLThreeDViewInformationWidget::~qMRMLThreeDViewInformationWidget() = default;

//---------------------------------------------------------------------------
vtkMRMLViewNode* qMRMLThreeDViewInformationWidget::mrmlViewNode()const
{
  Q_D(const qMRMLThreeDViewInformationWidget);
  return d->MRMLViewNode;
}

//---------------------------------------------------------------------------
void qMRMLThreeDViewInformationWidget::setMRMLViewNode(vtkMRMLNode* newNode)
{
  this->setMRMLViewNode(vtkMRMLViewNode::SafeDownCast(newNode));
}

//---------------------------------------------------------------------------
void qMRMLThreeDViewInformationWidget::setMRMLViewNode(vtkMRMLViewNode* newViewNode)
{
  Q_D(qMRMLThreeDViewInformationWidget);

  if (newViewNode == d->MRMLViewNode)
    {
    return;
    }

  d->qvtkReconnect(d->MRMLViewNode, newViewNode, vtkCommand::ModifiedEvent,
    d, SLOT(updateWidgetFromMRMLViewNode()));

  d->MRMLViewNode = newViewNode;

  // Update widget state given the new node
  d->updateWidgetFromMRMLViewNode();
}

//---------------------------------------------------------------------------
void qMRMLThreeDViewInformationWidget::setViewGroup(int viewGroup)
{
  Q_D(qMRMLThreeDViewInformationWidget);

  if (!d->MRMLViewNode)
    {
    return;
    }

  d->MRMLViewNode->SetViewGroup(viewGroup);
}
