/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qMRMLSceneSnapshotMenu_p_h
#define __qMRMLSceneSnapshotMenu_p_h

// Qt includes
#include<QSignalMapper>

// qMRML includes
#include "qMRMLSceneSnapshotMenu.h"

// CTK includes
#include <ctkVTKObject.h>

// MRML includes
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
class qMRMLSceneSnapshotMenuPrivate : public QObject
{
  Q_OBJECT
  QVTK_OBJECT
  Q_DECLARE_PUBLIC(qMRMLSceneSnapshotMenu);
protected:
  qMRMLSceneSnapshotMenu* const q_ptr;
public:
  typedef QObject Superclass;
  qMRMLSceneSnapshotMenuPrivate(qMRMLSceneSnapshotMenu& object);

  /// \brief Clear and update menu given the list of existing vtkMRMLSceneSnapshotNode
  /// associated with the current scene
  void resetMenu();

public slots:

  void onMRMLNodeAdded(vtkObject* mrmlScene, vtkObject * mrmlNode);

  /// Add menu entry corresponding to \a snapshotNode
  void addMenuItem(vtkMRMLNode * snapshotNode);

  void onMRMLNodeRemoved(vtkObject* mrmlScene, vtkObject * mrmlNode);

  /// Remove menu entry corresponding to \a snapshotNode
  void removeMenuItem(vtkMRMLNode * snapshotNode);

  void onMRMLSceneSnaphodeNodeModified(vtkObject * mrmlNode);

  bool hasNoSnapshotItem()const;

  void restoreSnapshot(const QString& snapshotNodeId);
  void deleteSnapshot(const QString& snapshotNodeId);

public:
  vtkSmartPointer<vtkMRMLScene>         MRMLScene;
  QSignalMapper                         RestoreActionMapper;
  QSignalMapper                         DeleteActionMapper;
  QString                               NoSnapshotText;

};

#endif

