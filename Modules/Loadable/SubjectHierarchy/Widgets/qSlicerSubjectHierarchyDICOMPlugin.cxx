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

// Qt includes
#include <QDebug>
#include <QAction>
#include <QStandardItem>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLStorableNode.h>
#include <vtkMRMLStorageNode.h>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkCollection.h>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SubjectHierarchy_Plugins
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

  QAction* CreateGenericSeriesAction;
  QAction* CreateGenericSubseriesAction;
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyDICOMPluginPrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyDICOMPluginPrivate::qSlicerSubjectHierarchyDICOMPluginPrivate(qSlicerSubjectHierarchyDICOMPlugin& object)
: q_ptr(&object)
{
  this->PatientIcon = QIcon(":Icons/Patient.png");

  this->CreateGenericSeriesAction = NULL;
  this->CreateGenericSubseriesAction = NULL;
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyDICOMPluginPrivate::init()
{
  Q_Q(qSlicerSubjectHierarchyDICOMPlugin);

  this->CreateGenericSeriesAction = new QAction("Create child generic series",q);
  QObject::connect(this->CreateGenericSeriesAction, SIGNAL(triggered()), q, SLOT(createChildForCurrentNode()));

  this->CreateGenericSubseriesAction = new QAction("Create child generic subseries",q);
  QObject::connect(this->CreateGenericSubseriesAction, SIGNAL(triggered()), q, SLOT(createChildForCurrentNode()));
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

  // Scene (empty level) -> Subject
  qSlicerSubjectHierarchyAbstractPlugin::m_ChildLevelMap.insert( QString(),
    vtkMRMLSubjectHierarchyConstants::SUBJECTHIERARCHY_LEVEL_SUBJECT );
  // Subject -> Study
  qSlicerSubjectHierarchyAbstractPlugin::m_ChildLevelMap.insert( vtkMRMLSubjectHierarchyConstants::SUBJECTHIERARCHY_LEVEL_SUBJECT,
    vtkMRMLSubjectHierarchyConstants::SUBJECTHIERARCHY_LEVEL_STUDY );
  // Study -> Series
  qSlicerSubjectHierarchyAbstractPlugin::m_ChildLevelMap.insert( vtkMRMLSubjectHierarchyConstants::SUBJECTHIERARCHY_LEVEL_STUDY,
    vtkMRMLSubjectHierarchyConstants::DICOMHIERARCHY_LEVEL_SERIES );
  // Series -> Subseries
  qSlicerSubjectHierarchyAbstractPlugin::m_ChildLevelMap.insert( vtkMRMLSubjectHierarchyConstants::DICOMHIERARCHY_LEVEL_SERIES,
    vtkMRMLSubjectHierarchyConstants::DICOMHIERARCHY_LEVEL_SUBSERIES );
  // Subseries -> Subseries
  qSlicerSubjectHierarchyAbstractPlugin::m_ChildLevelMap.insert( vtkMRMLSubjectHierarchyConstants::DICOMHIERARCHY_LEVEL_SUBSERIES,
    vtkMRMLSubjectHierarchyConstants::DICOMHIERARCHY_LEVEL_SUBSERIES );

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

  // Subject level
  if (node->IsLevel(vtkMRMLSubjectHierarchyConstants::SUBJECTHIERARCHY_LEVEL_SUBJECT))
    {
    return 0.7;
    }
  // Study level (so that creation of a generic series is possible)
  if (node->IsLevel(vtkMRMLSubjectHierarchyConstants::SUBJECTHIERARCHY_LEVEL_STUDY))
    {
    return 0.3;
    }
  // Series level
  if (node->IsLevel(vtkMRMLSubjectHierarchyConstants::DICOMHIERARCHY_LEVEL_SERIES))
    {
    return 0.3;
    }

  return 0.0;
}

//---------------------------------------------------------------------------
const QString qSlicerSubjectHierarchyDICOMPlugin::roleForPlugin()const
{
  // Get current node to determine tole
  //TODO: This is a workaround, needs to be fixed (each plugin should provide only one role!)
  vtkMRMLSubjectHierarchyNode* currentNode = qSlicerSubjectHierarchyPluginHandler::instance()->currentNode();
  if (!currentNode)
    {
    qCritical() << "qSlicerSubjectHierarchyDICOMPlugin::roleForPlugin: Invalid current node!";
    return "Error!";
    }

  // Subject level
  if (currentNode->IsLevel(vtkMRMLSubjectHierarchyConstants::SUBJECTHIERARCHY_LEVEL_SUBJECT))
    {
    return "Patient";
    }
  // Study level (so that creation of a generic series is possible)
  if (currentNode->IsLevel(vtkMRMLSubjectHierarchyConstants::SUBJECTHIERARCHY_LEVEL_STUDY))
    {
    return "Study";
    }
  // Series level
  if (currentNode->IsLevel(vtkMRMLSubjectHierarchyConstants::DICOMHIERARCHY_LEVEL_SERIES))
    {
    return "Generic series";
    }

  return QString("Error!");
}

//---------------------------------------------------------------------------
const QString qSlicerSubjectHierarchyDICOMPlugin::helpText()const
{
  return QString(
    "<p style=\" margin-top:4px; margin-bottom:1px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
    "<span style=\" font-family:'sans-serif'; font-size:9pt; font-weight:600; color:#000000;\">"
    "Create new generic Subject hierarchy node from scratch"
    "</span>"
    "</p>"
    "<p style=\" margin-top:0px; margin-bottom:11px; margin-left:26px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
    "<span style=\" font-family:'sans-serif'; font-size:9pt; color:#000000;\">"
    "Right-click on an existing node and select 'Create generic child node'. "
    "The level of the child node will be one under the parent node if available (e.g. 'Subject' -&gt; 'Study', 'Subseries' -&gt; 'Subseries')."
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
  if (node->IsLevel(vtkMRMLSubjectHierarchyConstants::SUBJECTHIERARCHY_LEVEL_SUBJECT))
    {
    return d->PatientIcon;
    }
  // Study icon
  if (node->IsLevel(vtkMRMLSubjectHierarchyConstants::SUBJECTHIERARCHY_LEVEL_STUDY))
    {
    return qSlicerSubjectHierarchyPluginHandler::instance()->defaultPlugin()->icon(node);
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
  actions << d->CreateGenericSeriesAction << d->CreateGenericSubseriesAction;
  return actions;
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyDICOMPlugin::showContextMenuActionsForNode(vtkMRMLSubjectHierarchyNode* node)
{
  Q_D(qSlicerSubjectHierarchyDICOMPlugin);
  this->hideAllContextMenuActions();

  if (!node)
    {
    // There are no scene actions in this plugin
    return;
    }

  // Study
  if (node->IsLevel(vtkMRMLSubjectHierarchyConstants::SUBJECTHIERARCHY_LEVEL_STUDY))
    {
    d->CreateGenericSeriesAction->setVisible(true);
    }
  // Series or Subseries
  else if ( node->IsLevel(vtkMRMLSubjectHierarchyConstants::DICOMHIERARCHY_LEVEL_SERIES)
    || node->IsLevel(vtkMRMLSubjectHierarchyConstants::DICOMHIERARCHY_LEVEL_SUBSERIES) )
    {
    d->CreateGenericSubseriesAction->setVisible(true);
    }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyDICOMPlugin::editProperties(vtkMRMLSubjectHierarchyNode* node)
{
  Q_UNUSED(node);
  //TODO: Show DICOM tag editor when implemented
}
