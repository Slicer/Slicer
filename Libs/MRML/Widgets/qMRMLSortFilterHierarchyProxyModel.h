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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qMRMLSortFilterHierarchyProxyModel_h
#define __qMRMLSortFilterHierarchyProxyModel_h

// qMRML includes
#include "qMRMLWidgetsExport.h"
#include "qMRMLSortFilterProxyModel.h"

class qMRMLSortFilterHierarchyProxyModelPrivate;

class QMRML_WIDGETS_EXPORT qMRMLSortFilterHierarchyProxyModel
  : public qMRMLSortFilterProxyModel
{
  Q_OBJECT
public:
  typedef qMRMLSortFilterProxyModel Superclass;
  qMRMLSortFilterHierarchyProxyModel(QObject *parent=nullptr);
  ~qMRMLSortFilterHierarchyProxyModel() override;

protected:
  // Don't show vtkMRMLHierarchyNode if they are tied to a vtkMRMLModelNode
  // The only vtkMRMLHierarchyNode to display are the ones who reference other
  // vtkMRMLHierarchyNode (tree parent) or empty (tree parent to be)
  AcceptType filterAcceptsNode(vtkMRMLNode* node)const override;

protected:
  QScopedPointer<qMRMLSortFilterHierarchyProxyModelPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLSortFilterHierarchyProxyModel);
  Q_DISABLE_COPY(qMRMLSortFilterHierarchyProxyModel);
};

#endif
