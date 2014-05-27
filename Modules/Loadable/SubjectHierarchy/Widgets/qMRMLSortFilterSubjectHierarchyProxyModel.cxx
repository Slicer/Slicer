/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

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
#include <vtkMRMLSubjectHierarchyNode.h>

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
};

// -----------------------------------------------------------------------------
qMRMLSortFilterSubjectHierarchyProxyModelPrivate::qMRMLSortFilterSubjectHierarchyProxyModelPrivate()
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

  vtkMRMLSubjectHierarchyNode* hNode = vtkMRMLSubjectHierarchyNode::SafeDownCast(node);
  if (hNode)
    {
    // Observe node in the model so that the item is updated on node changes
    this->sceneModel()->observeNode(node);

    return Accept;
    }

  return Reject;
}
