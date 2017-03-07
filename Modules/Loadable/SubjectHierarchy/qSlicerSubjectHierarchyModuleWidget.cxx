/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

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

// Subject Hierarchy includes
#include "qSlicerSubjectHierarchyModuleWidget.h"
#include "ui_qSlicerSubjectHierarchyModule.h"

#include "vtkMRMLSubjectHierarchyNode.h"
#include "vtkSlicerSubjectHierarchyModuleLogic.h"

#include "qMRMLSubjectHierarchyModel.h"
#include "qMRMLSortFilterSubjectHierarchyProxyModel.h"

#include "qSlicerSubjectHierarchyPluginLogic.h"
#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyAbstractPlugin.h"

// SlicerQt includes
#include "qSlicerApplication.h"

// Qt includes
#include <QSettings>
#include <QMessageBox>
#include <QDebug>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLNode.h>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SubjectHierarchy
class qSlicerSubjectHierarchyModuleWidgetPrivate: public Ui_qSlicerSubjectHierarchyModule
{
  Q_DECLARE_PUBLIC(qSlicerSubjectHierarchyModuleWidget);
protected:
  qSlicerSubjectHierarchyModuleWidget* const q_ptr;
public:
  qSlicerSubjectHierarchyModuleWidgetPrivate(qSlicerSubjectHierarchyModuleWidget& object);
  ~qSlicerSubjectHierarchyModuleWidgetPrivate();
  vtkSlicerSubjectHierarchyModuleLogic* logic() const;
public:
  /// Subject hierarchy plugin logic
  qSlicerSubjectHierarchyPluginLogic* PluginLogic;
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyModuleWidgetPrivate::qSlicerSubjectHierarchyModuleWidgetPrivate(qSlicerSubjectHierarchyModuleWidget& object)
  : q_ptr(&object)
  , PluginLogic(NULL)
{
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyModuleWidgetPrivate::~qSlicerSubjectHierarchyModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
vtkSlicerSubjectHierarchyModuleLogic*
qSlicerSubjectHierarchyModuleWidgetPrivate::logic() const
{
  Q_Q(const qSlicerSubjectHierarchyModuleWidget);
  return vtkSlicerSubjectHierarchyModuleLogic::SafeDownCast(q->logic());
}

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyModuleWidget::qSlicerSubjectHierarchyModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerSubjectHierarchyModuleWidgetPrivate(*this) )
{
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyModuleWidget::~qSlicerSubjectHierarchyModuleWidget()
{
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyPluginLogic* qSlicerSubjectHierarchyModuleWidget::pluginLogic()
{
  Q_D(qSlicerSubjectHierarchyModuleWidget);
  return d->PluginLogic;
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyModuleWidget::setPluginLogic(qSlicerSubjectHierarchyPluginLogic* pluginLogic)
{
  Q_D(qSlicerSubjectHierarchyModuleWidget);
  d->PluginLogic = pluginLogic;
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyModuleWidget::setup()
{
  Q_D(qSlicerSubjectHierarchyModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  // Make connections for the checkboxes and buttons
  connect( d->DisplayMRMLIDsCheckBox, SIGNAL(toggled(bool)), this, SLOT(setMRMLIDsVisible(bool)) );
  connect( d->DisplayTransformsCheckBox, SIGNAL(toggled(bool)), this, SLOT(setTransformsVisible(bool)) );

  // Set up tree view
  qMRMLSubjectHierarchyModel* sceneModel = (qMRMLSubjectHierarchyModel*)d->SubjectHierarchyTreeView->model();
  d->SubjectHierarchyTreeView->expandToDepth(4);
  d->SubjectHierarchyTreeView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
  d->SubjectHierarchyTreeView->header()->resizeSection(sceneModel->transformColumn(), 60);
  // Make subject hierarchy item info label text selectable
  d->SubjectHierarchyItemInfoLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

  connect(d->SubjectHierarchyTreeView, SIGNAL(currentItemChanged(vtkIdType)),
    this, SLOT(setDataNodeFromSubjectHierarchyItem(vtkIdType)) );
  connect(d->SubjectHierarchyTreeView, SIGNAL(currentItemChanged(vtkIdType)),
    this, SLOT(setInfoLabelFromSubjectHierarchyItem(vtkIdType)) );

  this->setMRMLIDsVisible(d->DisplayMRMLIDsCheckBox->isChecked());
  this->setTransformsVisible(d->DisplayTransformsCheckBox->isChecked());

  // Assemble help text for question mark tooltip
  QString aggregatedHelpText("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">    <html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">    p, li   { white-space: pre-wrap;   }  </style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">");
  foreach (qSlicerSubjectHierarchyAbstractPlugin* plugin, qSlicerSubjectHierarchyPluginHandler::instance()->allPlugins())
    {
    // Add help text from each plugin
    QString pluginHelpText = plugin->helpText();
    if (!pluginHelpText.isEmpty())
      {
      aggregatedHelpText.append(QString("\n") + pluginHelpText);
      }
    }
  aggregatedHelpText.append(QString("</body></html>"));
  d->label_Help->setToolTip(aggregatedHelpText);
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyModuleWidget::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qSlicerSubjectHierarchyModuleWidget);

  Superclass::setMRMLScene(scene);

  // Set tree root item to be the new scene
  d->SubjectHierarchyTreeView->setRootItem(d->SubjectHierarchyTreeView->subjectHierarchyNode()->GetSceneItemID());
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyModuleWidget::setMRMLIDsVisible(bool visible)
{
  Q_D(qSlicerSubjectHierarchyModuleWidget);

  d->SubjectHierarchyTreeView->setColumnHidden(d->SubjectHierarchyTreeView->model()->idColumn(), !visible);

  d->DisplayMRMLIDsCheckBox->blockSignals(true);
  d->DisplayMRMLIDsCheckBox->setChecked(visible);
  d->DisplayMRMLIDsCheckBox->blockSignals(false);
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyModuleWidget::setTransformsVisible(bool visible)
{
  Q_D(qSlicerSubjectHierarchyModuleWidget);

  qMRMLSubjectHierarchyModel* model = qobject_cast<qMRMLSubjectHierarchyModel*>(d->SubjectHierarchyTreeView->model());
  d->SubjectHierarchyTreeView->setColumnHidden(model->transformColumn(), !visible);
  d->SubjectHierarchyTreeView->header()->resizeSection(model->transformColumn(), 60);

  d->DisplayTransformsCheckBox->blockSignals(true);
  d->DisplayTransformsCheckBox->setChecked(visible);
  d->DisplayTransformsCheckBox->blockSignals(false);
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyModuleWidget::setDataNodeFromSubjectHierarchyItem(vtkIdType itemID)
{
  Q_D(qSlicerSubjectHierarchyModuleWidget);

  vtkMRMLSubjectHierarchyNode* shNode = d->SubjectHierarchyTreeView->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid subject hierarchy";
    return;
    }

  vtkMRMLNode* dataNode = NULL;
  if (itemID != vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    dataNode = shNode->GetItemDataNode(itemID);
    }
  d->DataNodeInspectorGroupBox->setVisible(dataNode);
  d->DataNodeAttributeTableWidget->setMRMLNode(dataNode);
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyModuleWidget::setInfoLabelFromSubjectHierarchyItem(vtkIdType itemID)
{
  Q_D(qSlicerSubjectHierarchyModuleWidget);

  vtkMRMLSubjectHierarchyNode* shNode = d->SubjectHierarchyTreeView->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid subject hierarchy";
    return;
    }

  if (itemID != vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    std::stringstream infoStream;
    shNode->PrintItem(itemID, infoStream, vtkIndent(0));
    d->SubjectHierarchyItemInfoLabel->setText(QLatin1String(infoStream.str().c_str()));
    }
  else
    {
    d->SubjectHierarchyItemInfoLabel->setText("No item selected");
    }
}

//-----------------------------------------------------------------------------
qMRMLSubjectHierarchyModel* qSlicerSubjectHierarchyModuleWidget::subjectHierarchySceneModel()const
{
  Q_D(const qSlicerSubjectHierarchyModuleWidget);

  qMRMLSubjectHierarchyModel* model = qobject_cast<qMRMLSubjectHierarchyModel*>(d->SubjectHierarchyTreeView->model());
  return model;
}
