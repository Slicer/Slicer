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

// SlicerQt includes
#include "qSlicerDataModuleWidget.h"
#include "ui_qSlicerDataModule.h"
#include "qSlicerApplication.h"
#include "qSlicerIOManager.h"

// SlicerLibs includes
#include <vtkSlicerTransformLogic.h>

// MRMLWidgets includes
#include <qMRMLSceneModel.h>

// MRML includes
#include <vtkMRMLLinearTransformNode.h>

//-----------------------------------------------------------------------------
class qSlicerDataModuleWidgetPrivate: public Ui_qSlicerDataModule
{
public:
  qSlicerDataModuleWidgetPrivate();
  vtkMRMLNode*                MRMLNode;
  QAction*                    HardenTransformAction;
};

//-----------------------------------------------------------------------------
qSlicerDataModuleWidgetPrivate::qSlicerDataModuleWidgetPrivate()
{
  this->MRMLNode = 0;
  this->HardenTransformAction = 0;
}

//-----------------------------------------------------------------------------
qSlicerDataModuleWidget::qSlicerDataModuleWidget(QWidget* parentWidget)
  :qSlicerAbstractModuleWidget(parentWidget)
  , d_ptr(new qSlicerDataModuleWidgetPrivate)
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

  // Filter on all the columns
  d->MRMLTreeView->sortFilterProxyModel()->setFilterKeyColumn(-1);
  connect(d->FilterLineEdit, SIGNAL(textChanged(QString)),
          d->MRMLTreeView->sortFilterProxyModel(), SLOT(setFilterFixedString(QString)));

  // hide the IDs by default
  d->DisplayMRMLIDsCheckBox->setChecked(false);

  // Hide the node inspector as it is possible to edit nodes via the tree
  d->MRMLNodeInspectorGroupBox->setVisible(false);
  //   connect(d->MRMLTreeView, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
  //           this, SLOT(onMRMLNodeChanged(vtkMRMLNode*)));
  // Change the node name only when the Enter key is pressed or the line edit
  // looses the focus
  //connect(d->NodeNameLineEdit, SIGNAL(editingFinished()),
  //        this, SLOT(validateNodeName()));

  // Connect the buttons to the associated slots
  connect(d->LoadSceneToolButton, SIGNAL(clicked()),
          this, SLOT(loadScene()));
  connect(d->AddSceneToolButton, SIGNAL(clicked()),
          this, SLOT(addScene()));
  connect(d->AddDataToolButton, SIGNAL(clicked()),
          this, SLOT(addData()));
  connect(d->AddVolumesToolButton, SIGNAL(clicked()),
          this, SLOT(addVolumes()));
  connect(d->AddModelsToolButton, SIGNAL(clicked()),
          this, SLOT(addModels()));
  connect(d->AddScalarOverlayToolButton, SIGNAL(clicked()),
          this, SLOT(addScalarOverlay()));
  connect(d->AddTransformationToolButton, SIGNAL(clicked()),
          this, SLOT(addTransformation()));
  connect(d->AddFiducialListToolButton, SIGNAL(clicked()),
          this, SLOT(addFiducialList()));
  connect(d->AddColorTableToolButton, SIGNAL(clicked()),
          this, SLOT(addColorTable()));
  connect(d->AddFiberBundleToolButton, SIGNAL(clicked()),
          this, SLOT(addFiberBundle()));

  QList<QToolButton*> helpToolButtons =
    d->LoadAddSceneButton->findChildren<QToolButton*>(
      QRegExp("*HelpToolButton", Qt::CaseSensitive, QRegExp::Wildcard));
  foreach(QToolButton* help, helpToolButtons)
    {
    // Set the help icon
    help->setIcon(this->style()->standardIcon(QStyle::SP_MessageBoxQuestion));
    // hide the text
    help->setChecked(false);
    }
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
  vtkMRMLTransformNode* linearTransform = vtkMRMLLinearTransformNode::New();

  vtkMRMLNode* parent = vtkMRMLTransformNode::SafeDownCast(d->MRMLTreeView->currentNode());
  if (parent)
    {
    linearTransform->SetAndObserveTransformNodeID( parent->GetID() );
    }

  this->mrmlScene()->AddNode(linearTransform);
  linearTransform->Delete();
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

//-----------------------------------------------------------------------------
void qSlicerDataModuleWidget::loadScene()
{
  qSlicerApplication::application()->ioManager()->openLoadSceneDialog();
}

//-----------------------------------------------------------------------------
void qSlicerDataModuleWidget::addScene()
{
  qSlicerApplication::application()->ioManager()->openAddSceneDialog();
}

//-----------------------------------------------------------------------------
void qSlicerDataModuleWidget::addData()
{
  qSlicerApplication::application()->ioManager()->openAddDataDialog();
}

//-----------------------------------------------------------------------------
void qSlicerDataModuleWidget::addVolumes()
{
  qSlicerApplication::application()->ioManager()->openAddVolumeDialog();
}

//-----------------------------------------------------------------------------
void qSlicerDataModuleWidget::addModels()
{
  qSlicerApplication::application()->ioManager()->openAddModelDialog();
}

//-----------------------------------------------------------------------------
void qSlicerDataModuleWidget::addScalarOverlay()
{
  qSlicerApplication::application()->ioManager()->openAddScalarOverlayDialog();
}

//-----------------------------------------------------------------------------
void qSlicerDataModuleWidget::addTransformation()
{
  qSlicerApplication::application()->ioManager()->openAddTransformDialog();
}

//-----------------------------------------------------------------------------
void qSlicerDataModuleWidget::addFiducialList()
{
  qSlicerApplication::application()->ioManager()->openAddFiducialDialog();
}

//-----------------------------------------------------------------------------
void qSlicerDataModuleWidget::addColorTable()
{
  qSlicerApplication::application()->ioManager()->openAddColorTableDialog();
}

//-----------------------------------------------------------------------------
void qSlicerDataModuleWidget::addFiberBundle()
{
  qSlicerApplication::application()->ioManager()->openAddFiberBundleDialog();
}
