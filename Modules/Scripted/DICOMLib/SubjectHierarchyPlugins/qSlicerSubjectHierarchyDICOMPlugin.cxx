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

// SubjectHierarchy MRML includes
#include "vtkMRMLSubjectHierarchyConstants.h"
#include "vtkMRMLSubjectHierarchyNode.h"

// SubjectHierarchy Plugins includes
#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyDICOMPlugin.h"
#include "qSlicerSubjectHierarchyDefaultPlugin.h"

// DICOMLib includes
#include "qSlicerDICOMExportDialog.h"

// Qt includes
#include <QDebug>
#include <QAction>
#include <QStandardItem>

// MRML includes
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SubjectHierarchy_Widgets
class qSlicerSubjectHierarchyDICOMPluginPrivate: public QObject
{
  Q_DECLARE_PUBLIC(qSlicerSubjectHierarchyDICOMPlugin);
protected:
  qSlicerSubjectHierarchyDICOMPlugin* const q_ptr;
public:
  qSlicerSubjectHierarchyDICOMPluginPrivate(qSlicerSubjectHierarchyDICOMPlugin& object);
  ~qSlicerSubjectHierarchyDICOMPluginPrivate();
  void init();
public:
  QIcon PatientIcon;
  QIcon StudyIcon;

  QAction* CreatePatientAction;
  QAction* CreateStudyAction;
  QAction* ConvertFolderToPatientAction;
  QAction* ConvertFolderToStudyAction;
  QAction* OpenDICOMExportDialogAction;
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyDICOMPluginPrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyDICOMPluginPrivate::qSlicerSubjectHierarchyDICOMPluginPrivate(qSlicerSubjectHierarchyDICOMPlugin& object)
: q_ptr(&object)
{
  this->PatientIcon = QIcon(":Icons/Patient.png");
  this->StudyIcon = QIcon(":Icons/Study.png");

  this->CreatePatientAction = NULL;
  this->CreateStudyAction = NULL;
  this->ConvertFolderToPatientAction = NULL;
  this->ConvertFolderToStudyAction = NULL;
  this->OpenDICOMExportDialogAction = NULL;
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyDICOMPluginPrivate::init()
{
  Q_Q(qSlicerSubjectHierarchyDICOMPlugin);

  this->CreatePatientAction = new QAction("Create new subject",q);
  QObject::connect(this->CreatePatientAction, SIGNAL(triggered()), q, SLOT(createPatientNode()));

  this->CreateStudyAction = new QAction("Create child study",q);
  QObject::connect(this->CreateStudyAction, SIGNAL(triggered()), q, SLOT(createChildStudyUnderCurrentNode()));

  this->ConvertFolderToPatientAction = new QAction("Convert folder to subject",q);
  QObject::connect(this->ConvertFolderToPatientAction, SIGNAL(triggered()), q, SLOT(convertCurrentNodeToPatient()));

  this->ConvertFolderToStudyAction = new QAction("Convert folder to study",q);
  QObject::connect(this->ConvertFolderToStudyAction, SIGNAL(triggered()), q, SLOT(convertCurrentNodeToStudy()));

  this->OpenDICOMExportDialogAction = new QAction("Export to DICOM...",q);
  QObject::connect(this->OpenDICOMExportDialogAction, SIGNAL(triggered()), q, SLOT(openDICOMExportDialog()));
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyDICOMPluginPrivate::~qSlicerSubjectHierarchyDICOMPluginPrivate()
{
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyDICOMPlugin::qSlicerSubjectHierarchyDICOMPlugin(QObject* parent)
 : Superclass(parent)
 , d_ptr( new qSlicerSubjectHierarchyDICOMPluginPrivate(*this) )
{
  this->m_Name = QString("DICOM");

  Q_D(qSlicerSubjectHierarchyDICOMPlugin);
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyDICOMPlugin::~qSlicerSubjectHierarchyDICOMPlugin()
{
}

//---------------------------------------------------------------------------
double qSlicerSubjectHierarchyDICOMPlugin::canOwnSubjectHierarchyNode(vtkMRMLSubjectHierarchyNode* node)const
{
  if (!node)
    {
    qCritical() << "qSlicerSubjectHierarchyDICOMPlugin::canOwnSubjectHierarchyNode: Input node is NULL!";
    return 0.0;
    }

  // Patient level
  if (node->IsLevel(vtkMRMLSubjectHierarchyConstants::GetDICOMLevelPatient()))
    {
    return 0.7;
    }
  // Study level (so that creation of a generic series is possible)
  if (node->IsLevel(vtkMRMLSubjectHierarchyConstants::GetDICOMLevelStudy()))
    {
    return 0.3;
    }

  return 0.0;
}

//---------------------------------------------------------------------------
const QString qSlicerSubjectHierarchyDICOMPlugin::roleForPlugin()const
{
  // Get current node to determine tole
  vtkMRMLSubjectHierarchyNode* currentNode = qSlicerSubjectHierarchyPluginHandler::instance()->currentNode();
  if (!currentNode)
    {
    qCritical() << "qSlicerSubjectHierarchyDICOMPlugin::roleForPlugin: Invalid current node!";
    return "Error!";
    }

  // Patient level
  if (currentNode->IsLevel(vtkMRMLSubjectHierarchyConstants::GetDICOMLevelPatient()))
    {
    return "Subject"; // Show the role Subject to the user, while internally it is used for the patient notation defined in DICOM
    }
  // Study level (so that creation of a generic series is possible)
  if (currentNode->IsLevel(vtkMRMLSubjectHierarchyConstants::GetDICOMLevelStudy()))
    {
    return "Study";
    }

  return QString("Error!");
}

//---------------------------------------------------------------------------
const QString qSlicerSubjectHierarchyDICOMPlugin::helpText()const
{
  return QString(
    "<p style=\" margin-top:4px; margin-bottom:1px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
    "<span style=\" font-family:'sans-serif'; font-size:9pt; font-weight:600; color:#000000;\">"
    "Create new Subject from scratch"
    "</span>"
    "</p>"
    "<p style=\" margin-top:0px; margin-bottom:11px; margin-left:26px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
    "<span style=\" font-family:'sans-serif'; font-size:9pt; color:#000000;\">"
    "Right-click on the top-level item 'Scene' and select 'Create new subject'."
    "</span>"
    "</p>\n"
    "<p style=\" margin-top:4px; margin-bottom:1px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
    "<span style=\" font-family:'sans-serif'; font-size:9pt; font-weight:600; color:#000000;\">"
    "Create new hierarchy node"
    "</span>"
    "</p>"
    "<p style=\" margin-top:0px; margin-bottom:11px; margin-left:26px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
    "<span style=\" font-family:'sans-serif'; font-size:9pt; color:#000000;\">"
    "Right-click on an existing node and select 'Create child ...'. "
    "The type and level of the child node will be specified by the user and the possible types depend on the parent."
    "</span>"
    "</p>");
}

//---------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchyDICOMPlugin::icon(vtkMRMLSubjectHierarchyNode* node)
{
  if (!node)
    {
    qCritical() << "qSlicerSubjectHierarchyDICOMPlugin::icon: NULL node given!";
    return QIcon();
    }

  Q_D(qSlicerSubjectHierarchyDICOMPlugin);

  // Patient icon
  if (node->IsLevel(vtkMRMLSubjectHierarchyConstants::GetDICOMLevelPatient()))
    {
    return d->PatientIcon;
    }
  // Study icon
  if (node->IsLevel(vtkMRMLSubjectHierarchyConstants::GetDICOMLevelStudy()))
    {
    return d->StudyIcon;
    }

  // Node unknown by plugin
  return QIcon();
}

//---------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchyDICOMPlugin::visibilityIcon(int visible)
{
  // Have the default plugin (which is not registered) take care of this
  return qSlicerSubjectHierarchyPluginHandler::instance()->defaultPlugin()->visibilityIcon(visible);
}

//---------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyDICOMPlugin::nodeContextMenuActions()const
{
  Q_D(const qSlicerSubjectHierarchyDICOMPlugin);

  QList<QAction*> actions;
  actions << d->CreateStudyAction << d->ConvertFolderToPatientAction << d->ConvertFolderToStudyAction << d->OpenDICOMExportDialogAction;
  return actions;
}

//---------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyDICOMPlugin::sceneContextMenuActions()const
{
  Q_D(const qSlicerSubjectHierarchyDICOMPlugin);

  QList<QAction*> actions;
  actions << d->CreatePatientAction;
  return actions;
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyDICOMPlugin::showContextMenuActionsForNode(vtkMRMLSubjectHierarchyNode* node)
{
  Q_D(qSlicerSubjectHierarchyDICOMPlugin);
  this->hideAllContextMenuActions();

  // Scene
  if (!node)
    {
    d->CreatePatientAction->setVisible(true);
    return;
    }

  // Patient
  if (node->IsLevel(vtkMRMLSubjectHierarchyConstants::GetDICOMLevelPatient()))
    {
    d->CreateStudyAction->setVisible(true);
    }
  // Folder
  else if (node->IsLevel(vtkMRMLSubjectHierarchyConstants::GetSubjectHierarchyLevelFolder()))
    {
    d->ConvertFolderToPatientAction->setVisible(true);
    d->ConvertFolderToStudyAction->setVisible(true);
    }
  // Study
  else if (node->IsLevel(vtkMRMLSubjectHierarchyConstants::GetDICOMLevelStudy()))
    {
    //if (this->canBeExported(node)) //TODO:
      {
      d->OpenDICOMExportDialogAction->setVisible(true);
      }
    }
  // Series
  else if (node->IsLevel(vtkMRMLSubjectHierarchyConstants::GetDICOMLevelSeries()))
    {
    //if (this->canBeExported(node)) //TODO:
      {
      d->OpenDICOMExportDialogAction->setVisible(true);
      }
    }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyDICOMPlugin::editProperties(vtkMRMLSubjectHierarchyNode* node)
{
  Q_UNUSED(node);
  //TODO: Show DICOM tag editor?
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyDICOMPlugin::createPatientNode()
{
  vtkMRMLScene* scene = qSlicerSubjectHierarchyPluginHandler::instance()->scene();
  if (!scene)
    {
    qCritical() << "qSlicerSubjectHierarchyDICOMPlugin::createPatientNode: Invalid MRML scene!";
    return;
    }

  // It is called Subject to the user, while internally it is used for the patient notation defined in DICOM
  std::string nodeName = vtkMRMLSubjectHierarchyConstants::GetSubjectHierarchyNewNodeNamePrefix() + "Subject";
  nodeName = scene->GenerateUniqueName(nodeName);
  // Create patient subject hierarchy node
  vtkMRMLSubjectHierarchyNode* childSubjectHierarchyNode = vtkMRMLSubjectHierarchyNode::CreateSubjectHierarchyNode(
    scene, NULL, vtkMRMLSubjectHierarchyConstants::GetDICOMLevelPatient(), nodeName.c_str());
  emit requestExpandNode(childSubjectHierarchyNode);
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyDICOMPlugin::createChildStudyUnderCurrentNode()
{
  vtkMRMLScene* scene = qSlicerSubjectHierarchyPluginHandler::instance()->scene();
  vtkMRMLSubjectHierarchyNode* currentNode = qSlicerSubjectHierarchyPluginHandler::instance()->currentNode();
  if (!scene || !currentNode)
    {
    qCritical() << "qSlicerSubjectHierarchyDICOMPlugin::createPatientNode: Invalid MRML scene or current node!";
    return;
    }

  std::string nodeName = vtkMRMLSubjectHierarchyConstants::GetSubjectHierarchyNewNodeNamePrefix() + vtkMRMLSubjectHierarchyConstants::GetDICOMLevelStudy();
  nodeName = scene->GenerateUniqueName(nodeName);
  // Create patient subject hierarchy node
  vtkMRMLSubjectHierarchyNode* childSubjectHierarchyNode = vtkMRMLSubjectHierarchyNode::CreateSubjectHierarchyNode(
    scene, currentNode, vtkMRMLSubjectHierarchyConstants::GetDICOMLevelStudy(), nodeName.c_str());
  emit requestExpandNode(childSubjectHierarchyNode);
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyDICOMPlugin::convertCurrentNodeToPatient()
{
  vtkMRMLSubjectHierarchyNode* currentNode = qSlicerSubjectHierarchyPluginHandler::instance()->currentNode();
  if ( !currentNode)
    {
    qCritical() << "qSlicerSubjectHierarchyDICOMPlugin::convertCurrentNodeToPatient: Invalid current node!";
    return;
    }

  currentNode->SetLevel(vtkMRMLSubjectHierarchyConstants::GetDICOMLevelPatient());
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyDICOMPlugin::convertCurrentNodeToStudy()
{
  vtkMRMLSubjectHierarchyNode* currentNode = qSlicerSubjectHierarchyPluginHandler::instance()->currentNode();
  if ( !currentNode)
    {
    qCritical() << "qSlicerSubjectHierarchyDICOMPlugin::convertCurrentNodeToPatient: Invalid current node!";
    return;
    }

  currentNode->SetLevel(vtkMRMLSubjectHierarchyConstants::GetDICOMLevelStudy());
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyDICOMPlugin::openDICOMExportDialog()
{
  vtkMRMLSubjectHierarchyNode* currentNode = qSlicerSubjectHierarchyPluginHandler::instance()->currentNode();
  if (!currentNode)
    {
    qCritical() << "qSlicerSubjectHierarchyDICOMPlugin::openDICOMExportDialog: Invalid current node!";
    return;
    }

  qSlicerDICOMExportDialog* exportDialog = new qSlicerDICOMExportDialog(NULL);
  exportDialog->setMRMLScene(qSlicerSubjectHierarchyPluginHandler::instance()->scene());
  exportDialog->exec(currentNode);

  delete exportDialog;
}
