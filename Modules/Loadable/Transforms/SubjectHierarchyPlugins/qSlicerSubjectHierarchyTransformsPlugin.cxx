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
#include "vtkMRMLSubjectHierarchyNode.h"
#include "vtkMRMLSubjectHierarchyConstants.h"

// SubjectHierarchy Plugins includes
#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyTransformsPlugin.h"
#include "qSlicerSubjectHierarchyDefaultPlugin.h"

// MRML includes
#include <vtkMRMLNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLTransformNode.h>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkNew.h>
#include <vtkMatrix4x4.h>

// Qt includes
#include <QDebug>
#include <QStandardItem>
#include <QAction>

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

// MRML widgets includes
#include "qMRMLNodeComboBox.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SubjectHierarchy_Plugins
class qSlicerSubjectHierarchyTransformsPluginPrivate: public QObject
{
  Q_DECLARE_PUBLIC(qSlicerSubjectHierarchyTransformsPlugin);
protected:
  qSlicerSubjectHierarchyTransformsPlugin* const q_ptr;
public:
  qSlicerSubjectHierarchyTransformsPluginPrivate(qSlicerSubjectHierarchyTransformsPlugin& object);
  ~qSlicerSubjectHierarchyTransformsPluginPrivate();
  void init();
public:
  QIcon TransformIcon;

  QAction* InvertAction;
  QAction* IdentityAction;
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyTransformsPluginPrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyTransformsPluginPrivate::qSlicerSubjectHierarchyTransformsPluginPrivate(qSlicerSubjectHierarchyTransformsPlugin& object)
: q_ptr(&object)
{
  this->TransformIcon = QIcon(":Icons/Transform.png");

  this->InvertAction = NULL;
  this->IdentityAction = NULL;
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyTransformsPluginPrivate::~qSlicerSubjectHierarchyTransformsPluginPrivate()
{
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyTransformsPlugin::qSlicerSubjectHierarchyTransformsPlugin(QObject* parent)
 : Superclass(parent)
 , d_ptr( new qSlicerSubjectHierarchyTransformsPluginPrivate(*this) )
{
  this->m_Name = QString("Transforms");

  Q_D(qSlicerSubjectHierarchyTransformsPlugin);
  d->init();
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyTransformsPluginPrivate::init()
{
  Q_Q(qSlicerSubjectHierarchyTransformsPlugin);

  this->InvertAction = new QAction("Invert transform",q);
  QObject::connect(this->InvertAction, SIGNAL(triggered()), q, SLOT(invert()));

  this->IdentityAction = new QAction("Reset transform to identity",q);
  QObject::connect(this->IdentityAction, SIGNAL(triggered()), q, SLOT(identity()));
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyTransformsPlugin::~qSlicerSubjectHierarchyTransformsPlugin()
{
}

//----------------------------------------------------------------------------
double qSlicerSubjectHierarchyTransformsPlugin::canAddNodeToSubjectHierarchy(vtkMRMLNode* node, vtkMRMLSubjectHierarchyNode* parent/*=NULL*/)const
{
  Q_UNUSED(parent);
  if (!node)
    {
    qCritical() << "qSlicerSubjectHierarchyTransformsPlugin::canAddNodeToSubjectHierarchy: Input node is NULL!";
    return 0.0;
    }
  else if (node->IsA("vtkMRMLTransformNode"))
    {
    // Node is a transform
    return 0.5;
    }
  return 0.0;
}

//---------------------------------------------------------------------------
double qSlicerSubjectHierarchyTransformsPlugin::canOwnSubjectHierarchyNode(vtkMRMLSubjectHierarchyNode* node)const
{
  if (!node)
    {
    qCritical() << "qSlicerSubjectHierarchyTransformsPlugin::canOwnSubjectHierarchyNode: Input node is NULL!";
    return 0.0;
    }

  // Transform
  vtkMRMLNode* associatedNode = node->GetAssociatedNode();
  if (associatedNode && associatedNode->IsA("vtkMRMLTransformNode"))
    {
    return 0.5; // There are other plugins that can handle special transform nodes better, thus the relatively low value
    }

  return 0.0;
}

//---------------------------------------------------------------------------
const QString qSlicerSubjectHierarchyTransformsPlugin::roleForPlugin()const
{
  return "Transform";
}

//-----------------------------------------------------------------------------
QString qSlicerSubjectHierarchyTransformsPlugin::tooltip(vtkMRMLSubjectHierarchyNode* node)const
{
  if (!node)
    {
    qCritical() << "qSlicerSubjectHierarchyTransformsPlugin::tooltip: Subject hierarchy node is NULL!";
    return QString("Invalid!");
    }

  // Get basic tooltip from abstract plugin
  QString tooltipString = Superclass::tooltip(node);

  vtkMRMLTransformNode* transformNode = vtkMRMLTransformNode::SafeDownCast(node->GetAssociatedNode());
  if (transformNode)
    {
    QString transformInfo = QString("\nTransform to parent:\n%1\nTransform from parent:\n%2").arg(
      transformNode->GetTransformToParentInfo()).arg(transformNode->GetTransformFromParentInfo());
    tooltipString.append(transformInfo);
    }

  return tooltipString;
}

//---------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchyTransformsPlugin::icon(vtkMRMLSubjectHierarchyNode* node)
{
  Q_D(qSlicerSubjectHierarchyTransformsPlugin);

  if (!node)
    {
    qCritical() << "qSlicerSubjectHierarchyTransformsPlugin::icon: NULL node given!";
    return QIcon();
    }

  if (this->canOwnSubjectHierarchyNode(node))
    {
    return d->TransformIcon;
    }

  // Node unknown by plugin
  return QIcon();
}

//---------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchyTransformsPlugin::visibilityIcon(int visible)
{
  // Have the default plugin (which is not registered) take care of this
  return qSlicerSubjectHierarchyPluginHandler::instance()->defaultPlugin()->visibilityIcon(visible);
}

//---------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyTransformsPlugin::nodeContextMenuActions()const
{
  Q_D(const qSlicerSubjectHierarchyTransformsPlugin);

  QList<QAction*> actions;
  actions << d->InvertAction << d->IdentityAction;
  return actions;
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyTransformsPlugin::showContextMenuActionsForNode(vtkMRMLSubjectHierarchyNode* node)
{
  Q_D(qSlicerSubjectHierarchyTransformsPlugin);
  this->hideAllContextMenuActions();

  if (!node)
    {
    // There are no scene actions in this plugin
    return;
    }

  if (this->canOwnSubjectHierarchyNode(node))
    {
    d->InvertAction->setVisible(true);
    vtkMRMLTransformNode* tnode = vtkMRMLTransformNode::SafeDownCast(node->GetAssociatedNode());
    if (tnode && tnode->IsLinear())
      {
      d->IdentityAction->setVisible(true);
      }
    }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyTransformsPlugin::editProperties(vtkMRMLSubjectHierarchyNode* node)
{
  // Switch to transforms module and select transform
  qSlicerAbstractModuleWidget* moduleWidget = qSlicerSubjectHierarchyAbstractPlugin::switchToModule("Transforms");
  if (moduleWidget)
    {
    // Get node selector combobox
    qMRMLNodeComboBox* nodeSelector = moduleWidget->findChild<qMRMLNodeComboBox*>("TransformNodeSelector");

    // Choose current data node
    if (nodeSelector)
      {
      nodeSelector->setCurrentNode(node->GetAssociatedNode());
      }
    }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyTransformsPlugin::invert()
{
  vtkMRMLSubjectHierarchyNode* currentNode = qSlicerSubjectHierarchyPluginHandler::instance()->currentNode();
  vtkMRMLTransformNode* transformNode = vtkMRMLTransformNode::SafeDownCast(currentNode->GetAssociatedNode());
  if (transformNode)
    {
    transformNode->Inverse();
    }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyTransformsPlugin::identity()
{
  vtkMRMLSubjectHierarchyNode* currentNode = qSlicerSubjectHierarchyPluginHandler::instance()->currentNode();
  vtkMRMLTransformNode* transformNode = vtkMRMLTransformNode::SafeDownCast(currentNode->GetAssociatedNode());
  if (transformNode && transformNode->IsLinear())
    {
    vtkNew<vtkMatrix4x4> matrix; // initialized to identity by default
    transformNode->SetMatrixTransformToParent(matrix.GetPointer());
    }
}
