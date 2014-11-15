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

// SlicerQt includes
#include "qSlicerSubjectHierarchyModuleWidget.h"
#include "ui_qSlicerSubjectHierarchyModule.h"

// Subject Hierarchy includes
#include "vtkMRMLSubjectHierarchyNode.h"
#include "vtkSlicerSubjectHierarchyModuleLogic.h"

#include "qMRMLSceneSubjectHierarchyModel.h"
#include "qMRMLSortFilterSubjectHierarchyProxyModel.h"

#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyAbstractPlugin.h"

// MRML includes
#include <vtkMRMLScene.h>

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
  /// Using this flag prevents overriding the parameter set node contents when the
  ///   QMRMLCombobox selects the first instance of the specified node type when initializing
  bool ModuleWindowInitialized;
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyModuleWidgetPrivate::qSlicerSubjectHierarchyModuleWidgetPrivate(qSlicerSubjectHierarchyModuleWidget& object)
  : q_ptr(&object)
  , ModuleWindowInitialized(false)
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
void qSlicerSubjectHierarchyModuleWidget::enter()
{
  this->onEnter();
  this->Superclass::enter();
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyModuleWidget::exit()
{
  this->Superclass::exit();

  Q_D(qSlicerSubjectHierarchyModuleWidget);
  d->SubjectHierarchyTreeView->setMRMLScene(NULL);
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyModuleWidget::onEnter()
{
  if (!this->mrmlScene())
    {
    return;
    }

  Q_D(qSlicerSubjectHierarchyModuleWidget);

  d->ModuleWindowInitialized = true;
  d->SubjectHierarchyTreeView->setMRMLScene(this->mrmlScene());

  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyModuleWidget::setup()
{
  Q_D(qSlicerSubjectHierarchyModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  // Make connection for the checkboxes
  connect( d->DisplayMRMLIDsCheckBox, SIGNAL(toggled(bool)), this, SLOT(setMRMLIDsVisible(bool)) );
  connect( d->DisplayTransformsCheckBox, SIGNAL(toggled(bool)), this, SLOT(setTransformsVisible(bool)) );

  // Set up tree view
  d->SubjectHierarchyTreeView->expandToDepth(4);
  d->SubjectHierarchyTreeView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);

  connect( d->SubjectHierarchyTreeView, SIGNAL(currentNodeChanged(vtkMRMLNode*)), d->MRMLNodeAttributeTableWidget, SLOT(setMRMLNode(vtkMRMLNode*)) );
  connect( d->SubjectHierarchyTreeView->sceneModel(), SIGNAL(invalidateFilter()), d->SubjectHierarchyTreeView->model(), SLOT(invalidate()) );

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
void qSlicerSubjectHierarchyModuleWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerSubjectHierarchyModuleWidget);

  //d->SubjectHierarchyTreeView->sortFilterProxyModel()->invalidate();
  qMRMLSceneSubjectHierarchyModel* sceneModel = (qMRMLSceneSubjectHierarchyModel*)d->SubjectHierarchyTreeView->sceneModel();
  d->SubjectHierarchyTreeView->header()->resizeSection(sceneModel->transformColumn(), 60);
  d->SubjectHierarchyTreeView->expandToDepth(4);
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyModuleWidget::setMRMLIDsVisible(bool visible)
{
  Q_D(qSlicerSubjectHierarchyModuleWidget);

  d->SubjectHierarchyTreeView->setColumnHidden(d->SubjectHierarchyTreeView->sceneModel()->idColumn(), !visible);

  d->DisplayMRMLIDsCheckBox->blockSignals(true);
  d->DisplayMRMLIDsCheckBox->setChecked(visible);
  d->DisplayMRMLIDsCheckBox->blockSignals(false);
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyModuleWidget::setTransformsVisible(bool visible)
{
  Q_D(qSlicerSubjectHierarchyModuleWidget);

  qMRMLSceneSubjectHierarchyModel* sceneModel = qobject_cast<qMRMLSceneSubjectHierarchyModel*>(d->SubjectHierarchyTreeView->sceneModel());
  d->SubjectHierarchyTreeView->setColumnHidden(sceneModel->transformColumn(), !visible);
  d->SubjectHierarchyTreeView->header()->resizeSection(sceneModel->transformColumn(), 60);

  d->DisplayTransformsCheckBox->blockSignals(true);
  d->DisplayTransformsCheckBox->setChecked(visible);
  d->DisplayTransformsCheckBox->blockSignals(false);
}

//-----------------------------------------------------------------------------
qMRMLSceneSubjectHierarchyModel* qSlicerSubjectHierarchyModuleWidget::subjectHierarchySceneModel()const
{
  Q_D(const qSlicerSubjectHierarchyModuleWidget);

  qMRMLSceneSubjectHierarchyModel* sceneModel = qobject_cast<qMRMLSceneSubjectHierarchyModel*>(d->SubjectHierarchyTreeView->sceneModel());
  return sceneModel;
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyAbstractPlugin* qSlicerSubjectHierarchyModuleWidget::subjectHierarchyPluginByName(QString name)const
{
  return qSlicerSubjectHierarchyPluginHandler::instance()->pluginByName(name);
}

//-----------------------------------------------------------------------------
vtkMRMLSubjectHierarchyNode* qSlicerSubjectHierarchyModuleWidget::currentSubjectHierarchyNode()const
{
  return qSlicerSubjectHierarchyPluginHandler::instance()->currentNode();
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyModuleWidget::setCurrentSubjectHierarchyNode(vtkMRMLSubjectHierarchyNode* node)
{
  qSlicerSubjectHierarchyPluginHandler::instance()->setCurrentNode(node);
}
