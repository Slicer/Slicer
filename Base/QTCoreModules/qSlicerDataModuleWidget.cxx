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
#include <QRegExp>

// SlicerQt includes
#include "qSlicerDataModuleWidget.h"
#include "ui_qSlicerDataModule.h"
#include "qSlicerApplication.h"
#include "qSlicerIOManager.h"

// qMRMLWidgets includes
#include <qMRMLSortFilterProxyModel.h>

// MRML includes
#include "vtkMRMLNode.h"

//-----------------------------------------------------------------------------
class qSlicerDataModuleWidgetPrivate: public Ui_qSlicerDataModule
{
public:
  qSlicerDataModuleWidgetPrivate();
  vtkMRMLNode*  MRMLNode;
};

//-----------------------------------------------------------------------------
qSlicerDataModuleWidgetPrivate::qSlicerDataModuleWidgetPrivate()
{
  this->MRMLNode = 0;
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

  d->MRMLSceneModelComboBox->addItem(QString("Transform"));
  d->MRMLSceneModelComboBox->addItem(QString("Displayable"));
//  connect(d->MRMLSceneModelComboBox, SIGNAL(currentIndexChanged(QString)),
//          d->MRMLTreeWidget, SLOT(setSceneModel(QString*)));
          
  connect(d->DisplayMRMLIDsCheckBox, SIGNAL(toggled(bool)),
          this, SLOT(setMRMLIDsVisible(bool)));
  connect(d->ShowHiddenCheckBox, SIGNAL(toggled(bool)),
          d->MRMLTreeWidget->sortFilterProxyModel(), SLOT(setShowHidden(bool)));
  connect(d->FilterLineEdit, SIGNAL(textChanged(const QString&)),
          d->MRMLTreeWidget->sortFilterProxyModel(), SLOT(setFilterFixedString(const QString&)));
  // hide the IDs by default
  d->DisplayMRMLIDsCheckBox->setChecked(false);

  // Hide the node inspector as it is possible to edit nodes via the tree
  d->MRMLNodeInspectorGroupBox->setVisible(false);
  //   connect(d->MRMLTreeWidget, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
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
  connect(d->AddDTIToolButton, SIGNAL(clicked()),
          this, SLOT(addDTI()));

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

  d->MRMLTreeWidget->setColumnHidden(1, !visible);
  const int columnCount = d->MRMLTreeWidget->header()->count();
  for(int i = 0; i < columnCount; ++i)
    {
    d->MRMLTreeWidget->resizeColumnToContents(i);
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

/* Hidden to the UI
//-----------------------------------------------------------------------------
void qSlicerDataModuleWidget::onMRMLNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerDataModuleWidget);
  qvtkDisconnect(d->MRMLNode, vtkCommand::ModifiedEvent,
                 this, SLOT(onMRMLNodeModified()));
  if (node)
    {
    qvtkConnect(node, vtkCommand::ModifiedEvent,
                this,SLOT(onMRMLNodeModified()));
    }
  d->MRMLNode = node;
  this->onMRMLNodeModified();
}

//-----------------------------------------------------------------------------
void qSlicerDataModuleWidget::onMRMLNodeModified()
{
  Q_D(qSlicerDataModuleWidget);
  d->NodeIDLineEdit->setText(d->MRMLNode ? d->MRMLNode->GetID() : "");
  d->NodeNameLineEdit->setText(d->MRMLNode ? d->MRMLNode->GetName() : "");
}

//-----------------------------------------------------------------------------
void qSlicerDataModuleWidget::validateNodeName()
{
  Q_D(qSlicerDataModuleWidget);
  this->setCurrentNodeName(d->NodeNameLineEdit->text());
}

//-----------------------------------------------------------------------------
void qSlicerDataModuleWidget::setCurrentNodeName(const QString& name)
{
  Q_D(qSlicerDataModuleWidget);
  if (d->MRMLNode == 0)
    {
    return;
    }
  d->MRMLNode->SetName(name.toLatin1().data());
}
*/
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
void qSlicerDataModuleWidget::addDTI()
{
  qSlicerApplication::application()->ioManager()->openAddDTIDialog();
}
