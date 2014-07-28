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

#ifndef __qMRMLPotentialSubjectHierarchyListView_h
#define __qMRMLPotentialSubjectHierarchyListView_h

// Qt includes
#include <QListView>

// SubjectHierarchy includes
#include "qSlicerSubjectHierarchyModuleWidgetsExport.h"

class qMRMLPotentialSubjectHierarchyListViewPrivate;
class vtkMRMLScene;
class qMRMLSortFilterProxyModel;

/// \ingroup Slicer_QtModules_SubjectHierarchy
class Q_SLICER_MODULE_SUBJECTHIERARCHY_WIDGETS_EXPORT qMRMLPotentialSubjectHierarchyListView : public QListView
{
  Q_OBJECT
public:
  qMRMLPotentialSubjectHierarchyListView(QWidget *parent=0);
  virtual ~qMRMLPotentialSubjectHierarchyListView();

  vtkMRMLScene* mrmlScene()const;

  qMRMLSortFilterProxyModel* sortFilterProxyModel()const;

public slots:
  void setMRMLScene(vtkMRMLScene* scene);

protected:
  QScopedPointer<qMRMLPotentialSubjectHierarchyListViewPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLPotentialSubjectHierarchyListView);
  Q_DISABLE_COPY(qMRMLPotentialSubjectHierarchyListView);
};

#endif
