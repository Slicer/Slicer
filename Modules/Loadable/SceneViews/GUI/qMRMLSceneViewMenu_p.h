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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qMRMLSceneViewMenu_p_h
#define __qMRMLSceneViewMenu_p_h

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Slicer API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

// Qt includes
#include <QSignalMapper>

// qMRML includes
#include "qMRMLSceneViewMenu.h"

// CTK includes
#include <ctkVTKObject.h>

// MRML includes
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkSmartPointer.h>

// Scene views logic includes
#include <vtkSlicerSceneViewsModuleLogic.h>

//-----------------------------------------------------------------------------
class qMRMLSceneViewMenuPrivate : public QObject
{
  Q_OBJECT
  QVTK_OBJECT
  Q_DECLARE_PUBLIC(qMRMLSceneViewMenu);

protected:
  qMRMLSceneViewMenu* const q_ptr;

public:
  typedef QObject Superclass;
  qMRMLSceneViewMenuPrivate(qMRMLSceneViewMenu& object);

public slots:
  /// \brief Clear and update menu given the list of existing vtkMRMLSceneViewNode
  /// associated with the current scene
  /// Rebuild the menu if it is visible. The menu is rebuilt when it is about to be shown,
  /// therefore it does not have to be kept up-to-date while it is hidden (rebuilding the menu
  /// on each scene view modification would make loading of scenes with many scene views slow).
  void resetMenu();
  /// Rebuild the menu from the scene views in the scene.
  void rebuildMenu();

  void onMRMLNodeAdded(vtkObject* mrmlScene, vtkObject* mrmlNode);

  /// Add menu entry corresponding to \a sceneViewNode
  void addMenuItem(int index);

  void onMRMLNodeRemoved(vtkObject* mrmlScene, vtkObject* mrmlNode);

  /// Remove menu entry corresponding to \a sceneViewNode
  void removeMenuItem(int index);

  bool hasNoSceneViewItem() const;

  /// Get the scene views module logic (looked up when first needed, as it may not exist when the menu is created)
  vtkSlicerSceneViewsModuleLogic* sceneViewsLogic();

  void restoreSceneView(int index);
  void deleteSceneView(int index);

public:
  vtkSmartPointer<vtkMRMLScene> MRMLScene;
  QSignalMapper RestoreActionMapper;
  QSignalMapper DeleteActionMapper;
  QString NoSceneViewText;

  vtkWeakPointer<vtkSlicerSceneViewsModuleLogic> SceneViewsLogic;
};

#endif
