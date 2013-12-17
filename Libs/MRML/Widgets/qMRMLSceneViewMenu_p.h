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
#include<QSignalMapper>

// qMRML includes
#include "qMRMLSceneViewMenu.h"

// CTK includes
#include <ctkVTKObject.h>

// MRML includes
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkSmartPointer.h>

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

  /// \brief Clear and update menu given the list of existing vtkMRMLSceneViewNode
  /// associated with the current scene
  void resetMenu();

public slots:

  void onMRMLNodeAdded(vtkObject* mrmlScene, vtkObject * mrmlNode);

  /// Add menu entry corresponding to \a sceneViewNode
  void addMenuItem(vtkMRMLNode * sceneViewNode);

  void onMRMLNodeRemoved(vtkObject* mrmlScene, vtkObject * mrmlNode);

  /// Remove menu entry corresponding to \a sceneViewNode
  void removeMenuItem(vtkMRMLNode * sceneViewNode);

  void onMRMLSceneViewNodeModified(vtkObject * mrmlNode);

  bool hasNoSceneViewItem()const;

  void restoreSceneView(const QString& sceneViewNodeId);
  void deleteSceneView(const QString& sceneViewNodeId);

public:
  vtkSmartPointer<vtkMRMLScene>         MRMLScene;
  QSignalMapper                         RestoreActionMapper;
  QSignalMapper                         DeleteActionMapper;
  QString                               NoSceneViewText;

};

#endif

