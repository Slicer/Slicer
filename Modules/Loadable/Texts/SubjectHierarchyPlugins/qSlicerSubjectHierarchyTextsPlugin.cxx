/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, and Cancer
  Care Ontario.

==============================================================================*/

// SubjectHierarchy Plugins includes
#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyTextsPlugin.h"
#include "qSlicerSubjectHierarchyDefaultPlugin.h"

// Subject Hierarchy includes
#include <vtkSlicerSubjectHierarchyModuleLogic.h>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLTextNode.h>

// Qt includes
#include <QDebug>

// Slicer includes
#include "qSlicerApplication.h"
#include "qSlicerAbstractModuleWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SubjectHierarchy_Plugins
class qSlicerSubjectHierarchyTextsPluginPrivate: public QObject
{
  Q_DECLARE_PUBLIC(qSlicerSubjectHierarchyTextsPlugin);
protected:
  qSlicerSubjectHierarchyTextsPlugin* const q_ptr;
public:
  qSlicerSubjectHierarchyTextsPluginPrivate(qSlicerSubjectHierarchyTextsPlugin& object);
  ~qSlicerSubjectHierarchyTextsPluginPrivate() override;
public:
  QIcon TextIcon;
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyTextsPluginPrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyTextsPluginPrivate::qSlicerSubjectHierarchyTextsPluginPrivate(qSlicerSubjectHierarchyTextsPlugin& object)
: q_ptr(&object)
{
  this->TextIcon = QIcon(":Icons/Text.png");
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyTextsPluginPrivate::~qSlicerSubjectHierarchyTextsPluginPrivate() = default;

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyTextsPlugin methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyTextsPlugin::qSlicerSubjectHierarchyTextsPlugin(QObject* parent)
 : Superclass(parent)
 , d_ptr( new qSlicerSubjectHierarchyTextsPluginPrivate(*this) )
{
  this->m_Name = QString("Texts");
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyTextsPlugin::~qSlicerSubjectHierarchyTextsPlugin() = default;

//----------------------------------------------------------------------------
double qSlicerSubjectHierarchyTextsPlugin::canAddNodeToSubjectHierarchy(
  vtkMRMLNode* node, vtkIdType parentItemID/*=vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID*/)const
{
  Q_UNUSED(parentItemID);
  if (!node)
    {
    qCritical() << Q_FUNC_INFO << ": Input node is nullptr!";
    return 0.0;
    }
  else if (node->IsA("vtkMRMLTextNode"))
    {
    return 0.5;
    }
  return 0.0;
}

//---------------------------------------------------------------------------
double qSlicerSubjectHierarchyTextsPlugin::canOwnSubjectHierarchyItem(vtkIdType itemID) const
{
  if (itemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return 0.0;
    }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return 0.0;
    }

  // Text
  vtkMRMLNode* associatedNode = shNode->GetItemDataNode(itemID);
  if (associatedNode && associatedNode->IsA("vtkMRMLTextNode"))
    {
    return 0.5;
    }

  return 0.0;
}

//---------------------------------------------------------------------------
const QString qSlicerSubjectHierarchyTextsPlugin::roleForPlugin()const
{
  return "Text";
}

//-----------------------------------------------------------------------------
QString qSlicerSubjectHierarchyTextsPlugin::tooltip(vtkIdType itemID)const
{
  if (itemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return QString("Invalid!");
    }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return QString("Error!");
    }

  // Get basic tooltip from abstract plugin
  QString tooltipString = Superclass::tooltip(itemID);

  vtkMRMLTextNode* textNode = vtkMRMLTextNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  if (textNode)
    {
    std::stringstream textNodeInfo;
    textNodeInfo << " (Encoding: " << textNode->GetEncodingAsString() << ")" << std::endl << textNode->GetText();
    QString textInfo = QString::fromStdString(textNodeInfo.str());
    tooltipString.append(textInfo);
    }

  return tooltipString;
}

//---------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchyTextsPlugin::icon(vtkIdType itemID)
{
  Q_D(qSlicerSubjectHierarchyTextsPlugin);

  if (itemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return QIcon();
    }

  // Text
  if (this->canOwnSubjectHierarchyItem(itemID))
    {
    return d->TextIcon;
    }

  // Item unknown by plugin
  return QIcon();
}
