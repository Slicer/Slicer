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

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// Qt includes

// qMRML includes
#include "qMRMLNodeAttributeTableWidget.h"
#include "ui_qMRMLNodeAttributeTableWidget.h"

// MRML includes
#include <vtkMRMLNode.h>

// STD includes
#include <sstream>

// --------------------------------------------------------------------------
class qMRMLNodeAttributeTableWidgetPrivate: public Ui_qMRMLNodeAttributeTableWidget
{
  Q_DECLARE_PUBLIC(qMRMLNodeAttributeTableWidget);
protected:
  qMRMLNodeAttributeTableWidget* const q_ptr;
public:
  qMRMLNodeAttributeTableWidgetPrivate(qMRMLNodeAttributeTableWidget& object);
  void init();

  vtkWeakPointer<vtkMRMLNode> MRMLNode;
};

// --------------------------------------------------------------------------
qMRMLNodeAttributeTableWidgetPrivate::qMRMLNodeAttributeTableWidgetPrivate(qMRMLNodeAttributeTableWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qMRMLNodeAttributeTableWidgetPrivate::init()
{
  Q_Q(qMRMLNodeAttributeTableWidget);
  this->setupUi(q);

  QObject::connect(this->AddAttributeButton, SIGNAL(clicked()),
          this->MRMLNodeAttributeTableView, SLOT(addAttribute()));
  QObject::connect(this->RemoveAttributeButton, SIGNAL(clicked()),
          this->MRMLNodeAttributeTableView, SLOT(removeSelectedAttributes()));
}

// --------------------------------------------------------------------------
// qMRMLNodeAttributeTableWidget methods

// --------------------------------------------------------------------------
qMRMLNodeAttributeTableWidget::qMRMLNodeAttributeTableWidget(QWidget* _parent)
  : QWidget(_parent)
  , d_ptr(new qMRMLNodeAttributeTableWidgetPrivate(*this))
{
  Q_D(qMRMLNodeAttributeTableWidget);
  d->init();
}

// --------------------------------------------------------------------------
qMRMLNodeAttributeTableWidget::~qMRMLNodeAttributeTableWidget() = default;

// --------------------------------------------------------------------------
vtkMRMLNode* qMRMLNodeAttributeTableWidget::mrmlNode()const
{
  Q_D(const qMRMLNodeAttributeTableWidget);
  return d->MRMLNode.GetPointer();
}

// --------------------------------------------------------------------------
void qMRMLNodeAttributeTableWidget::setMRMLNode(vtkMRMLNode* node)
{
  Q_D(qMRMLNodeAttributeTableWidget);
  d->MRMLNodeAttributeTableView->setInspectedNode(node);

  qvtkReconnect(d->MRMLNode.GetPointer(), node, vtkCommand::ModifiedEvent,
                this, SLOT(updateWidgetFromMRML()));
  qvtkReconnect(d->MRMLNode.GetPointer(), node, vtkMRMLNode::ReferenceAddedEvent,
                this, SLOT(updateWidgetFromMRML()));
  qvtkReconnect(d->MRMLNode.GetPointer(), node, vtkMRMLNode::ReferenceModifiedEvent,
                this, SLOT(updateWidgetFromMRML()));
  qvtkReconnect(d->MRMLNode.GetPointer(), node, vtkMRMLNode::ReferenceRemovedEvent,
                this, SLOT(updateWidgetFromMRML()));
  d->MRMLNode = node;

  this->updateWidgetFromMRML();
}

// --------------------------------------------------------------------------
qMRMLNodeAttributeTableView* qMRMLNodeAttributeTableWidget::tableView()
{
  Q_D(qMRMLNodeAttributeTableWidget);
  return d->MRMLNodeAttributeTableView;
}

//------------------------------------------------------------------------------
void qMRMLNodeAttributeTableWidget::showEvent(QShowEvent *)
{
  // Update the widget, now that it becomes becomes visible
  // (we might have missed some updates, because widget contents is not updated
  // if the widget is not visible).
  this->updateWidgetFromMRML();
}

//------------------------------------------------------------------------------
void qMRMLNodeAttributeTableWidget::updateWidgetFromMRML()
{
  Q_D(qMRMLNodeAttributeTableWidget);
  if (!this->isVisible())
    {
    // Getting the node information may be expensive,
    // so if the widget is not visible then do not update
    return;
    }

  if (d->MRMLNode.GetPointer())
    {
    d->NodeInformationGroupBox->setVisible(true);
    std::stringstream infoStream;
    d->MRMLNode->PrintSelf(infoStream, vtkIndent(0));
    d->MRMLNodeInfoLabel->setText(infoStream.str().c_str());
    }
  else
    {
    d->NodeInformationGroupBox->setVisible(false);
    d->MRMLNodeInfoLabel->clear();
    }
}
