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

// SubjectHierarchy Plugins includes
#include "qSlicerSubjectHierarchyDefaultPlugin.h"
#include "qSlicerSubjectHierarchyPluginHandler.h"

// SubjectHierarchy MRML includes
#include "vtkMRMLSubjectHierarchyConstants.h"
#include "vtkMRMLSubjectHierarchyNode.h"

// Qt includes
#include <QDebug>
#include <QIcon>
#include <QAction>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

//----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SubjectHierarchy_Widgets
class qSlicerSubjectHierarchyDefaultPluginPrivate: public QObject
{
  Q_DECLARE_PUBLIC(qSlicerSubjectHierarchyDefaultPlugin);
protected:
  qSlicerSubjectHierarchyDefaultPlugin* const q_ptr;
public:
  qSlicerSubjectHierarchyDefaultPluginPrivate(qSlicerSubjectHierarchyDefaultPlugin& object);
  ~qSlicerSubjectHierarchyDefaultPluginPrivate() override;
  void init();
public:
  QIcon UnknownIcon;

  QIcon VisibleIcon;
  QIcon HiddenIcon;
  QIcon PartiallyVisibleIcon;

  QAction* ShowAllChildrenAction;
  QAction* HideAllChildrenAction;
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyDefaultPluginPrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyDefaultPluginPrivate::qSlicerSubjectHierarchyDefaultPluginPrivate(qSlicerSubjectHierarchyDefaultPlugin& object)
: q_ptr(&object)
, ShowAllChildrenAction(nullptr)
, HideAllChildrenAction(nullptr)
{
  this->UnknownIcon = QIcon(":Icons/Unknown.png");
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyDefaultPluginPrivate::~qSlicerSubjectHierarchyDefaultPluginPrivate() = default;

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyDefaultPluginPrivate::init()
{
  Q_Q(qSlicerSubjectHierarchyAbstractPlugin);

  this->ShowAllChildrenAction = new QAction("Show all children",q);
  QObject::connect(this->ShowAllChildrenAction, SIGNAL(triggered()), q, SLOT(showAllChildren()));

  this->HideAllChildrenAction = new QAction("Hide all children",q);
  QObject::connect(this->HideAllChildrenAction, SIGNAL(triggered()), q, SLOT(hideAllChildren()));
  }

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyDefaultPlugin methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyDefaultPlugin::qSlicerSubjectHierarchyDefaultPlugin(QObject* parent)
 : Superclass(parent)
 , d_ptr( new qSlicerSubjectHierarchyDefaultPluginPrivate(*this) )
{
  this->m_Name = QString("Default");

  Q_D(qSlicerSubjectHierarchyDefaultPlugin);
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyDefaultPlugin::~qSlicerSubjectHierarchyDefaultPlugin() = default;

//----------------------------------------------------------------------------
void qSlicerSubjectHierarchyDefaultPlugin::setDefaultVisibilityIcons(QIcon visibleIcon, QIcon hiddenIcon, QIcon partiallyVisibleIcon)
{
  Q_D(qSlicerSubjectHierarchyDefaultPlugin);

  d->VisibleIcon = visibleIcon;
  d->HiddenIcon = hiddenIcon;
  d->PartiallyVisibleIcon = partiallyVisibleIcon;
}

//---------------------------------------------------------------------------
double qSlicerSubjectHierarchyDefaultPlugin::canOwnSubjectHierarchyItem(vtkIdType itemID)const
{
  Q_UNUSED(itemID);

  // The default Subject Hierarchy plugin is never selected by confidence number it returns
  return 0.0;
}

//---------------------------------------------------------------------------
const QString qSlicerSubjectHierarchyDefaultPlugin::roleForPlugin()const
{
  return "Unknown";
}

//---------------------------------------------------------------------------
const QString qSlicerSubjectHierarchyDefaultPlugin::helpText()const
{
  return QString(
    "<p style=\" margin-top:4px; margin-bottom:1px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
    "<span style=\" font-family:'sans-serif'; font-size:9pt; font-weight:600; color:#000000;\">"
    "Rename item"
    "</span>"
    "</p>"
    "<p style=\" margin-top:0px; margin-bottom:11px; margin-left:26px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
    "<span style=\" font-family:'sans-serif'; font-size:9pt; color:#000000;\">"
    "Double-click the item name, or right-click the item and select 'Rename'"
    "</span>"
    "</p>"
    "<p style=\" margin-top:4px; margin-bottom:1px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
    "<span style=\" font-family:'sans-serif'; font-size:9pt; font-weight:600; color:#000000;\">"
    "Deform any branch using a transform (registration result)"
    "</span>"
    "</p>"
    "<p style=\" margin-top:0px; margin-bottom:11px; margin-left:26px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
    "<span style=\" font-size:8pt;\">"
    "Make sure the transform column is shown using the 'Transforms' checkbox. "
    "To transform a branch, double click on the cell in the transform column of the row in question, and choose a transform."
    "</span>"
    "</p>");
}

//---------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchyDefaultPlugin::icon(vtkIdType itemID)
{
  if (itemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Input item is invalid";
    return QIcon();
    }

  Q_D(qSlicerSubjectHierarchyDefaultPlugin);

  // Unknown icon
  // This role is only used when there is no plugin to claim a node, which is an erroneous
  // scenario, as only those nodes can be added to subject hierarchy for which there is at
  // least one plugin that can claim them.
  return d->UnknownIcon;
}

//---------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchyDefaultPlugin::visibilityIcon(int visible)
{
  Q_D(qSlicerSubjectHierarchyDefaultPlugin);

  // Default icon is the eye icon that shows the visibility of the whole branch
  switch (visible)
    {
  case 0:
    return d->HiddenIcon;
  case 1:
    return d->VisibleIcon;
  case 2:
    return d->PartiallyVisibleIcon;
  default:
    return QIcon();
    }
}

//-----------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyDefaultPlugin::visibilityContextMenuActions()const
{
  Q_D(const qSlicerSubjectHierarchyDefaultPlugin);

  QList<QAction*> actions;
  return actions;
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyDefaultPlugin::showVisibilityContextMenuActionsForItem(vtkIdType itemID)
{
  Q_D(qSlicerSubjectHierarchyDefaultPlugin);

  if (itemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    return;
    }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }

  // Show child-related actions only if there are children to the item
  std::vector<vtkIdType> childItems;
  shNode->GetItemChildren(itemID, childItems);
  d->ShowAllChildrenAction->setVisible(childItems.size());
  d->HideAllChildrenAction->setVisible(childItems.size());
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyDefaultPlugin::toggleVisibility()
{
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }
  vtkIdType currentItemID = qSlicerSubjectHierarchyPluginHandler::instance()->currentItem();
  if (currentItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid current item";
    return;
    }
  qSlicerSubjectHierarchyAbstractPlugin* ownerPlugin =
    qSlicerSubjectHierarchyPluginHandler::instance()->getOwnerPluginForSubjectHierarchyItem(currentItemID);
  if (!ownerPlugin)
    {
    qCritical() << Q_FUNC_INFO << ": Subject hierarchy item " << currentItemID << " (named " << shNode->GetItemName(currentItemID).c_str() << ") is not owned by any plugin";
    return;
    }

  // Toggle current item visibility
  int visible = (ownerPlugin->getDisplayVisibility(currentItemID) > 0 ? 0 : 1);
  ownerPlugin->setDisplayVisibility(currentItemID, visible);
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyDefaultPlugin::showAllChildren()
{
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }
  vtkIdType currentItemID = qSlicerSubjectHierarchyPluginHandler::instance()->currentItem();
  if (currentItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid current item";
    return;
    }

  std::vector<vtkIdType> childItemIDs;
  shNode->GetItemChildren(currentItemID, childItemIDs, true);
  std::vector<vtkIdType>::iterator childIt;
  for (childIt=childItemIDs.begin(); childIt!=childItemIDs.end(); ++childIt)
    {
    vtkIdType childItemID = (*childIt);
    qSlicerSubjectHierarchyAbstractPlugin* ownerPlugin =
      qSlicerSubjectHierarchyPluginHandler::instance()->getOwnerPluginForSubjectHierarchyItem(childItemID);
    if (!ownerPlugin)
      {
      qCritical() << Q_FUNC_INFO << ": Subject hierarchy item " << childItemID << " (named "
        << shNode->GetItemName(childItemID).c_str() << ") is not owned by any plugin";
      return;
      }

    ownerPlugin->setDisplayVisibility(childItemID, 1);
    }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyDefaultPlugin::hideAllChildren()
{
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }
  vtkIdType currentItemID = qSlicerSubjectHierarchyPluginHandler::instance()->currentItem();
  if (currentItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid current item";
    return;
    }

  std::vector<vtkIdType> childItemIDs;
  shNode->GetItemChildren(currentItemID, childItemIDs, true);
  std::vector<vtkIdType>::iterator childIt;
  for (childIt=childItemIDs.begin(); childIt!=childItemIDs.end(); ++childIt)
    {
    vtkIdType childItemID = (*childIt);
    qSlicerSubjectHierarchyAbstractPlugin* ownerPlugin =
      qSlicerSubjectHierarchyPluginHandler::instance()->getOwnerPluginForSubjectHierarchyItem(childItemID);
    if (!ownerPlugin)
      {
      qCritical() << Q_FUNC_INFO << ": Subject hierarchy item " << childItemID << " (named "
        << shNode->GetItemName(childItemID).c_str() << ") is not owned by any plugin";
      return;
      }

    ownerPlugin->setDisplayVisibility(childItemID, 0);
    }
}
