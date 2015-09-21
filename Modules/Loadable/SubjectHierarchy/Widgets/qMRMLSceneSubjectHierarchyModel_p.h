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

#ifndef __qMRMLSceneSubjectHierarchyModel_p_h
#define __qMRMLSceneSubjectHierarchyModel_p_h

// qMRML includes
#include "qMRMLSceneSubjectHierarchyModel.h"
#include "qMRMLSceneHierarchyModel_p.h"

// Subject Hierarchy Plugins includes
#include "qSlicerSubjectHierarchyDefaultPlugin.h"

// Qt includes
#include <QSet>

//------------------------------------------------------------------------------
// qMRMLSceneSubjectHierarchyModelPrivate
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SubjectHierarchy
class Q_SLICER_MODULE_SUBJECTHIERARCHY_WIDGETS_EXPORT qMRMLSceneSubjectHierarchyModelPrivate
  : public qMRMLSceneHierarchyModelPrivate
{
protected:
  Q_DECLARE_PUBLIC(qMRMLSceneSubjectHierarchyModel);

public:
  typedef qMRMLSceneHierarchyModelPrivate Superclass;
  qMRMLSceneSubjectHierarchyModelPrivate(qMRMLSceneSubjectHierarchyModel& object);
  virtual void init();

  int NodeTypeColumn;
  int TransformColumn;

  QIcon UnknownIcon;
  QIcon WarningIcon;

  mutable QSet<QStandardItem*>  DraggedItems;
  bool DelayedItemChangedFired;
};

#endif
