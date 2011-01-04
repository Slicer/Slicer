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

// QT includes
#include <QDebug>

// qMRML includes
#include "qMRMLColorPickerWidget.h"
#include "qMRMLColorModel.h"
#include "ui_qMRMLColorPickerWidget.h"

// MRMLLogic includes
#include <vtkMRMLColorLogic.h>

// MRML includes
#include <vtkMRMLColorNode.h>

// VTK includes
#include <vtkSmartPointer.h>

//------------------------------------------------------------------------------
class qMRMLColorPickerWidgetPrivate: public Ui_qMRMLColorPickerWidget
{
  Q_DECLARE_PUBLIC(qMRMLColorPickerWidget);

protected:
  qMRMLColorPickerWidget* const q_ptr;

public:
  qMRMLColorPickerWidgetPrivate(qMRMLColorPickerWidget& object);
  void init();
};

//------------------------------------------------------------------------------
qMRMLColorPickerWidgetPrivate::qMRMLColorPickerWidgetPrivate(qMRMLColorPickerWidget& object)
  : q_ptr(&object)
{
}

//------------------------------------------------------------------------------
void qMRMLColorPickerWidgetPrivate::init()
{
  Q_Q(qMRMLColorPickerWidget);
  this->setupUi(q);
  QObject::connect(this->ColorTableComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                   q, SIGNAL(currentColorNodeChanged(vtkMRMLNode*)));
  QObject::connect(this->MRMLColorListView, SIGNAL(colorSelected(int)),
                   q, SIGNAL(colorEntrySelected(int)));
  QObject::connect(this->MRMLColorListView, SIGNAL(colorSelected(const QColor&)),
                   q, SIGNAL(colorSelected(const QColor&)));
}

//------------------------------------------------------------------------------
qMRMLColorPickerWidget::qMRMLColorPickerWidget(QWidget *_parent)
  : qMRMLWidget(_parent)
  , d_ptr(new qMRMLColorPickerWidgetPrivate(*this))
{
  Q_D(qMRMLColorPickerWidget);
  d->init();
}

//------------------------------------------------------------------------------
qMRMLColorPickerWidget::~qMRMLColorPickerWidget()
{
}

//------------------------------------------------------------------------------
vtkMRMLColorNode* qMRMLColorPickerWidget::currentColorNode()const
{
  Q_D(const qMRMLColorPickerWidget);
  return vtkMRMLColorNode::SafeDownCast(d->ColorTableComboBox->currentNode());
}

//------------------------------------------------------------------------------
void qMRMLColorPickerWidget::setCurrentColorNode(vtkMRMLNode* node)
{
  Q_D(qMRMLColorPickerWidget);
  d->ColorTableComboBox->setCurrentNode(node);
  this->qvtkDisconnect(this->mrmlScene(), vtkMRMLScene::NodeAddedEvent,
                       this, SLOT(onNodeAdded(vtkObject*, vtkObject*)));
}

//------------------------------------------------------------------------------
void qMRMLColorPickerWidget::setCurrentColorNodeToDefault()
{
  if (!this->mrmlScene())
    {
    return;
    }
  // TODO: Retrieve a unique instance of color logic from the MRML Application logic
  vtkSmartPointer<vtkMRMLColorLogic> colorLogic =
    vtkSmartPointer<vtkMRMLColorLogic>::New();
  vtkMRMLNode* defaultColorNode =
    this->mrmlScene()->GetNodeByID(colorLogic->GetDefaultLabelMapColorNodeID());
  if (defaultColorNode)
    {
    this->setCurrentColorNode(defaultColorNode);
    }
}

//------------------------------------------------------------------------------
void qMRMLColorPickerWidget::onNodeAdded(vtkObject* scene, vtkObject* nodeObject)
{
  Q_UNUSED(scene);
  vtkMRMLNode* node = vtkMRMLNode::SafeDownCast(nodeObject);
  // TODO: Retrieve a unique instance of color logic from the MRML Application logic
  vtkSmartPointer<vtkMRMLColorLogic> colorLogic =
    vtkSmartPointer<vtkMRMLColorLogic>::New();
  if (node &&
      QString(node->GetID()) == colorLogic->GetDefaultLabelMapColorNodeID())
    {
    this->setCurrentColorNode(node);
    }
}

//------------------------------------------------------------------------------
void qMRMLColorPickerWidget::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qMRMLColorPickerWidget);
  this->setCurrentColorNode(0); // eventually disconnect NodeAddedEvent
  this->qMRMLWidget::setMRMLScene(scene);
  if (scene && !d->ColorTableComboBox->currentNode())
    {
    this->qvtkConnect(scene, vtkMRMLScene::NodeAddedEvent,
                      this, SLOT(onNodeAdded(vtkObject*, vtkObject*)));
    this->setCurrentColorNodeToDefault();
   }
}
