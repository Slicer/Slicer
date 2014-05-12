/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Andras Lasso and Franklin King at
  PerkLab, Queen's University and was supported through the Applied Cancer
  Research Unit program of Cancer Care Ontario with funds provided by the
  Ontario Ministry of Health and Long-Term Care.

==============================================================================*/


// qMRML includes
#include "qMRMLTransformInfoWidget.h"
#include "ui_qMRMLTransformInfoWidget.h"

// MRML includes
#include <vtkMRMLTransformNode.h>

//------------------------------------------------------------------------------
class qMRMLTransformInfoWidgetPrivate: public Ui_qMRMLTransformInfoWidget
{
  Q_DECLARE_PUBLIC(qMRMLTransformInfoWidget);

protected:
  qMRMLTransformInfoWidget* const q_ptr;

public:
  qMRMLTransformInfoWidgetPrivate(qMRMLTransformInfoWidget& object);
  void init();

  vtkMRMLTransformNode* MRMLTransformNode;
};

//------------------------------------------------------------------------------
qMRMLTransformInfoWidgetPrivate::qMRMLTransformInfoWidgetPrivate(qMRMLTransformInfoWidget& object)
  : q_ptr(&object)
{
  this->MRMLTransformNode = 0;
}

//------------------------------------------------------------------------------
void qMRMLTransformInfoWidgetPrivate::init()
{
  Q_Q(qMRMLTransformInfoWidget);
  this->setupUi(q);
  q->setEnabled(this->MRMLTransformNode != 0);
  this->TransformToParentInfoTextBrowser->setTextInteractionFlags(Qt::TextSelectableByMouse);
  this->TransformFromParentInfoTextBrowser->setTextInteractionFlags(Qt::TextSelectableByMouse);
}

//------------------------------------------------------------------------------
qMRMLTransformInfoWidget::qMRMLTransformInfoWidget(QWidget *_parent)
  : Superclass(_parent)
  , d_ptr(new qMRMLTransformInfoWidgetPrivate(*this))
{
  Q_D(qMRMLTransformInfoWidget);
  d->init();
}

//------------------------------------------------------------------------------
qMRMLTransformInfoWidget::~qMRMLTransformInfoWidget()
{
}

//------------------------------------------------------------------------------
vtkMRMLTransformNode* qMRMLTransformInfoWidget::mrmlTransformNode()const
{
  Q_D(const qMRMLTransformInfoWidget);
  return d->MRMLTransformNode;
}

//------------------------------------------------------------------------------
void qMRMLTransformInfoWidget::setMRMLTransformNode(vtkMRMLNode* node)
{
  this->setMRMLTransformNode(vtkMRMLTransformNode::SafeDownCast(node));
}

//------------------------------------------------------------------------------
void qMRMLTransformInfoWidget::setMRMLTransformNode(vtkMRMLTransformNode* transformNode)
{
  Q_D(qMRMLTransformInfoWidget);
  qvtkReconnect(d->MRMLTransformNode, transformNode, vtkCommand::ModifiedEvent,
                this, SLOT(updateWidgetFromMRML()));
  d->MRMLTransformNode = transformNode;
  this->updateWidgetFromMRML();
}

//------------------------------------------------------------------------------
void qMRMLTransformInfoWidget::updateWidgetFromMRML()
{
  Q_D(qMRMLTransformInfoWidget);
  if (d->MRMLTransformNode)
    {
    d->TransformToParentInfoTextBrowser->setText(d->MRMLTransformNode->GetTransformToParentInfo());
    d->TransformFromParentInfoTextBrowser->setText(d->MRMLTransformNode->GetTransformFromParentInfo());
    }
  else
    {
    d->TransformToParentInfoTextBrowser->setText("");
    d->TransformFromParentInfoTextBrowser->setText("");
    }

  this->setEnabled(d->MRMLTransformNode != 0);
}
