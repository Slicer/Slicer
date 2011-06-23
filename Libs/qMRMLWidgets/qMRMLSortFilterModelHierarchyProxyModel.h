/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

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

#ifndef __qMRMLSortFilterModelHierarchyProxyModel_h
#define __qMRMLSortFilterModelHierarchyProxyModel_h

// qMRML includes
#include "qMRMLWidgetsExport.h"
#include "qMRMLSortFilterProxyModel.h"

class qMRMLSortFilterModelHierarchyProxyModelPrivate;

class QMRML_WIDGETS_EXPORT qMRMLSortFilterModelHierarchyProxyModel
  : public qMRMLSortFilterProxyModel
{
  Q_OBJECT
public:
  typedef qMRMLSortFilterProxyModel Superclass;
  qMRMLSortFilterModelHierarchyProxyModel(QObject *parent=0);
  virtual ~qMRMLSortFilterModelHierarchyProxyModel();

protected:
  // Don't show vtkMRMLModelHierarchyNode if they are tied to a vtkMRMLModelNode
  // The only vtkMRMLModelHierarchyNode to display are the ones who reference other
  // vtkMRMLModelHierarchyNode (tree parent) or empty (tree parent to be)
  virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent)const;
  
protected:
  QScopedPointer<qMRMLSortFilterModelHierarchyProxyModelPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLSortFilterModelHierarchyProxyModel);
  Q_DISABLE_COPY(qMRMLSortFilterModelHierarchyProxyModel);
};

#endif
