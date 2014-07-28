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

#include "qMRMLSortFilterPotentialSubjectHierarchyProxyModel.h"

// SubjectHierarchy includes
#include "vtkMRMLSubjectHierarchyConstants.h"
#include "vtkMRMLSubjectHierarchyNode.h"
#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyAbstractPlugin.h"

// qMRML includes
#include "qMRMLSceneModel.h"

// -----------------------------------------------------------------------------
// qMRMLSortFilterPotentialSubjectHierarchyProxyModelPrivate

// -----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SubjectHierarchy
class qMRMLSortFilterPotentialSubjectHierarchyProxyModelPrivate
{
public:
  qMRMLSortFilterPotentialSubjectHierarchyProxyModelPrivate();
};

// -----------------------------------------------------------------------------
qMRMLSortFilterPotentialSubjectHierarchyProxyModelPrivate::qMRMLSortFilterPotentialSubjectHierarchyProxyModelPrivate()
{
}

// -----------------------------------------------------------------------------
// qMRMLSortFilterPotentialSubjectHierarchyProxyModel

//------------------------------------------------------------------------------
qMRMLSortFilterPotentialSubjectHierarchyProxyModel::qMRMLSortFilterPotentialSubjectHierarchyProxyModel(QObject *vparent)
  : qMRMLSortFilterProxyModel(vparent)
  , d_ptr(new qMRMLSortFilterPotentialSubjectHierarchyProxyModelPrivate)
{
}

//------------------------------------------------------------------------------
qMRMLSortFilterPotentialSubjectHierarchyProxyModel::~qMRMLSortFilterPotentialSubjectHierarchyProxyModel()
{
}

//------------------------------------------------------------------------------
qMRMLSortFilterProxyModel::AcceptType qMRMLSortFilterPotentialSubjectHierarchyProxyModel
::filterAcceptsNode(vtkMRMLNode* node)const
{
  Q_D(const qMRMLSortFilterPotentialSubjectHierarchyProxyModel);

  if (!node)
    {
    return Accept;
    }

  AcceptType res = this->Superclass::filterAcceptsNode(node);
  if (res == Reject || res == RejectButPotentiallyAcceptable)
    {
    return res;
    }

  // Show only nodes that do not have a subject hierarchy node associated
  vtkMRMLSubjectHierarchyNode* possibleShNode = vtkMRMLSubjectHierarchyNode::GetAssociatedSubjectHierarchyNode(node);
  if (!possibleShNode && !node->GetHideFromEditors())
    {
    if (node->GetAttribute(vtkMRMLSubjectHierarchyConstants::SUBJECTHIERARCHY_EXCLUDE_FROM_POTENTIAL_NODES_LIST_ATTRIBUTE_NAME.c_str()))
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
