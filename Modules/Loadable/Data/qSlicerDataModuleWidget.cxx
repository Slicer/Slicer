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

// SlicerQt includes
#include "qSlicerDataModuleWidget.h"
#include "ui_qSlicerDataModuleWidget.h"
#include "qSlicerApplication.h"
#include "qSlicerIOManager.h"

// Data Logic includes
#include "vtkSlicerDataModuleLogic.h"

// SlicerLibs includes
#include <vtkSlicerTransformLogic.h>

// MRMLWidgets includes
#include <qMRMLSceneModel.h>

// MRML includes
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkNew.h>

// STL includes
#include <set>

//-----------------------------------------------------------------------------
class qSlicerDataModuleWidgetPrivate: public Ui_qSlicerDataModuleWidget
{
  Q_DECLARE_PUBLIC(qSlicerDataModuleWidget);
protected:
  qSlicerDataModuleWidget* const q_ptr;
public:
  qSlicerDataModuleWidgetPrivate(qSlicerDataModuleWidget& object);
  vtkSlicerDataModuleLogic* logic() const;

  QAction*                    HardenTransformAction;
};

//-----------------------------------------------------------------------------
qSlicerDataModuleWidgetPrivate::qSlicerDataModuleWidgetPrivate(qSlicerDataModuleWidget& object)
 : q_ptr(&object)
{
  this->HardenTransformAction = 0;
}

//-----------------------------------------------------------------------------
vtkSlicerDataModuleLogic*
qSlicerDataModuleWidgetPrivate::logic() const
{
  Q_Q(const qSlicerDataModuleWidget);
  return vtkSlicerDataModuleLogic::SafeDownCast(q->logic());
}

//-----------------------------------------------------------------------------
qSlicerDataModuleWidget::qSlicerDataModuleWidget(QWidget* parentWidget)
  :qSlicerAbstractModuleWidget(parentWidget)
  , d_ptr( new qSlicerDataModuleWidgetPrivate(*this) )
{
}

//-----------------------------------------------------------------------------
qSlicerDataModuleWidget::~qSlicerDataModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerDataModuleWidget::setup()
{
  Q_D(qSlicerDataModuleWidget);

  d->setupUi(this);

  // Edit properties...
  connect(d->MRMLTreeView, SIGNAL(editNodeRequested(vtkMRMLNode*)),
          qSlicerApplication::application(), SLOT(openNodeModule(vtkMRMLNode*)));

  // Insert transform
  QAction* insertTransformAction = new QAction(tr("Insert transform"),this);
  d->MRMLTreeView->prependNodeMenuAction(insertTransformAction);
  d->MRMLTreeView->prependSceneMenuAction(insertTransformAction);
  connect(insertTransformAction, SIGNAL(triggered()),
          this, SLOT(insertTransformNode()));
  // Harden transform
  connect( d->MRMLTreeView, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
           this, SLOT(onCurrentNodeChanged(vtkMRMLNode*)) );
  d->HardenTransformAction = new QAction(tr("Harden transform"), this);
  connect( d->HardenTransformAction, SIGNAL(triggered()),
           this, SLOT(hardenTransformOnCurrentNode()) );

  connect(d->MRMLSceneModelComboBox, SIGNAL(currentIndexChanged(QString)),
          this, SLOT(onSceneModelChanged(QString)));

  // Connection with TreeView is done in UI file
  d->MRMLSceneModelComboBox->addItem(QString("Transform"));
  d->MRMLSceneModelComboBox->addItem(QString("Displayable"));
  d->MRMLSceneModelComboBox->addItem(QString("ModelHierarchy"));

  connect(d->DisplayMRMLIDsCheckBox, SIGNAL(toggled(bool)),
          this, SLOT(setMRMLIDsVisible(bool)));
  connect(d->ShowHiddenCheckBox, SIGNAL(toggled(bool)),
          d->MRMLTreeView->sortFilterProxyModel(), SLOT(setShowHidden(bool)));

  QAction* printNodeAction = new QAction(tr("Print"),this);
  d->MRMLTreeView->appendNodeMenuAction(printNodeAction);
  d->MRMLTreeView->appendSceneMenuAction(printNodeAction);
  connect(printNodeAction, SIGNAL(triggered()),
          this, SLOT(printObject()));

  // Filter on all the columns
  d->MRMLTreeView->sortFilterProxyModel()->setFilterKeyColumn(-1);
  connect(d->FilterLineEdit, SIGNAL(textChanged(QString)),
          d->MRMLTreeView->sortFilterProxyModel(), SLOT(setFilterFixedString(QString)));

  // Hide the IDs by default
  d->DisplayMRMLIDsCheckBox->setChecked(false);

  // Make connections for the attribute table widget
  connect(d->MRMLTreeView, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
          d->MRMLNodeAttributeTableWidget, SLOT(setMRMLNode(vtkMRMLNode*)));
}

//-----------------------------------------------------------------------------
void qSlicerDataModuleWidget::setMRMLIDsVisible(bool visible)
{
  Q_D(qSlicerDataModuleWidget);

  d->MRMLTreeView->setColumnHidden(1, !visible);
  const int columnCount = d->MRMLTreeView->header()->count();
  for(int i = 0; i < columnCount; ++i)
    {
    d->MRMLTreeView->resizeColumnToContents(i);
    }
  d->DisplayMRMLIDsCheckBox->setChecked(visible);
}

//-----------------------------------------------------------------------------
void qSlicerDataModuleWidget::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qSlicerDataModuleWidget);
  this->qSlicerAbstractModuleWidget::setMRMLScene(scene);
  this->setMRMLIDsVisible(d->DisplayMRMLIDsCheckBox->isChecked());
}

//-----------------------------------------------------------------------------
void qSlicerDataModuleWidget::insertTransformNode()
{
  Q_D(qSlicerDataModuleWidget);
  vtkNew<vtkMRMLLinearTransformNode> linearTransform;
  this->mrmlScene()->AddNode(linearTransform.GetPointer());

  vtkMRMLNode* parent = vtkMRMLTransformNode::SafeDownCast(
    d->MRMLTreeView->currentNode());
  if (parent)
    {
    linearTransform->SetAndObserveTransformNodeID( parent->GetID() );
    }
}

//-----------------------------------------------------------------------------
void qSlicerDataModuleWidget::printObject()
{
  Q_D(qSlicerDataModuleWidget);
  vtkObject* object = d->MRMLTreeView->currentNode() ?
    vtkObject::SafeDownCast(d->MRMLTreeView->currentNode()) :
    vtkObject::SafeDownCast(this->mrmlScene());
  if (object)
    {
    object->Print(std::cout);
    }
}

//-----------------------------------------------------------------------------
void qSlicerDataModuleWidget::onCurrentNodeChanged(vtkMRMLNode* newCurrentNode)
{
  Q_D(qSlicerDataModuleWidget);
  vtkMRMLTransformableNode* transformableNode =
    vtkMRMLTransformableNode::SafeDownCast(newCurrentNode);
  vtkMRMLTransformNode* transformNode =
    transformableNode ? transformableNode->GetParentTransformNode() : 0;
  if (transformNode &&
      (transformNode->CanApplyNonLinearTransforms() ||
      transformNode->IsTransformToWorldLinear()))
    {
    d->MRMLTreeView->prependNodeMenuAction(d->HardenTransformAction);
    }
  else
    {
    d->MRMLTreeView->removeNodeMenuAction(d->HardenTransformAction);
    }
}

//-----------------------------------------------------------------------------
void qSlicerDataModuleWidget::onSceneModelChanged(const QString& modelType)
{
  Q_D(qSlicerDataModuleWidget);
  d->MRMLTreeView->setSceneModelType( modelType );

  d->MRMLTreeView->sceneModel()->setIDColumn(1);
  d->MRMLTreeView->sceneModel()->setHorizontalHeaderLabels(
    QStringList() << "Nodes" << "IDs");

  d->MRMLTreeView->header()->setStretchLastSection(false);
  d->MRMLTreeView->header()->setResizeMode(0, QHeaderView::Stretch);
  d->MRMLTreeView->header()->setResizeMode(1, QHeaderView::ResizeToContents);

  this->setMRMLIDsVisible(d->DisplayMRMLIDsCheckBox->isChecked());

  connect(d->ShowHiddenCheckBox, SIGNAL(toggled(bool)),
          d->MRMLTreeView->sortFilterProxyModel(), SLOT(setShowHidden(bool)));

  d->MRMLTreeView->sortFilterProxyModel()->invalidate();
}

//-----------------------------------------------------------------------------
void qSlicerDataModuleWidget::hardenTransformOnCurrentNode()
{
  Q_D(qSlicerDataModuleWidget);
  vtkMRMLNode* node = d->MRMLTreeView->currentNode();
  vtkSlicerTransformLogic::hardenTransform(
    vtkMRMLTransformableNode::SafeDownCast(node));
}

