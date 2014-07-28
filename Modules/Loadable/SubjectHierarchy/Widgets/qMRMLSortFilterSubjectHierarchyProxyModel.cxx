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

#include "qMRMLSortFilterSubjectHierarchyProxyModel.h"

// Subject Hierarchy MRML includes
#include "vtkMRMLSubjectHierarchyNode.h"
#include "vtkMRMLSubjectHierarchyConstants.h"
#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyAbstractPlugin.h"

// MRML Widgets includes
#include "qMRMLSceneModel.h"

// -----------------------------------------------------------------------------
// qMRMLSortFilterSubjectHierarchyProxyModelPrivate

// -----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SubjectHierarchy
class qMRMLSortFilterSubjectHierarchyProxyModelPrivate
{
public:
  qMRMLSortFilterSubjectHierarchyProxyModelPrivate();

  bool showPotentialNodes;
};

// -----------------------------------------------------------------------------
qMRMLSortFilterSubjectHierarchyProxyModelPrivate::qMRMLSortFilterSubjectHierarchyProxyModelPrivate()
: showPotentialNodes(true)
{
}

// -----------------------------------------------------------------------------
// qMRMLSortFilterSubjectHierarchyProxyModel

//------------------------------------------------------------------------------
qMRMLSortFilterSubjectHierarchyProxyModel::qMRMLSortFilterSubjectHierarchyProxyModel(QObject *vparent)
 : qMRMLSortFilterProxyModel(vparent)
 , d_ptr(new qMRMLSortFilterSubjectHierarchyProxyModelPrivate)
{
}

//------------------------------------------------------------------------------
qMRMLSortFilterSubjectHierarchyProxyModel::~qMRMLSortFilterSubjectHierarchyProxyModel()
{
}

//------------------------------------------------------------------------------
void qMRMLSortFilterSubjectHierarchyProxyModel::setPotentialNodesVisible(bool visible)
{
  Q_D(qMRMLSortFilterSubjectHierarchyProxyModel);

  d->showPotentialNodes = visible;
}

//------------------------------------------------------------------------------
qMRMLSortFilterProxyModel::AcceptType qMRMLSortFilterSubjectHierarchyProxyModel
::filterAcceptsNode(vtkMRMLNode* node)const
{
  if (!node)
    {
    return Accept;
    }

  AcceptType res = this->Superclass::filterAcceptsNode(node);
  if (res == Reject || res == RejectButPotentiallyAcceptable)
    {
    return res;
    }

  Q_D(const qMRMLSortFilterSubjectHierarchyProxyModel);

  // Show all subject hierarchy nodes and potential subject hierarchy nodes (regular data nodes
  // for which there is a subject hierarchy plugin that can add it to the subject hierarchy tree)
  vtkMRMLSubjectHierarchyNode* subjectHierarchyNode = vtkMRMLSubjectHierarchyNode::SafeDownCast(node);
  if (subjectHierarchyNode)
    {
    return Accept;
    }
  else if (!node->GetHideFromEditors() && d->showPotentialNodes)
    {
    // Hide the node if it's explicitly excluded from the tree as a potential node,
    // or if there is a subject hierarchy node associated to it (i.e. it is in the hierarchy already)
    vtkMRMLSubjectHierarchyNode* associatedShNode = vtkMRMLSubjectHierarchyNode::GetAssociatedSubjectHierarchyNode(node);
    if ( associatedShNode
      || node->GetAttribute(vtkMRMLSubjectHierarchyConstants::SUBJECTHIERARCHY_EXCLUDE_FROM_POTENTIAL_NODES_LIST_ATTRIBUTE_NAME.c_str()) )
      {
      return Reject;
      }

    // Show only if the node is a potential subject hierarchy node according the the plugins
    QList<qSlicerSubjectHierarchyAbstractPlugin*> foundPlugins
      = qSlicerSubjectHierarchyPluginHandler::instance()->pluginsForAddingToSubjectHierarchyForNode(node);
    if (!foundPlugins.empty())
      {
      return Accept;
      }
    }

  return Reject;
}
