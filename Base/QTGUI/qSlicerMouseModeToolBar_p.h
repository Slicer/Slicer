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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerMouseModeToolBar_p_h
#define __qSlicerMouseModeToolBar_p_h

// Qt includes
#include <QToolBar>

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>
#include "qSlicerBaseQTGUIExport.h"

// SlicerQt includes
#include "qSlicerMouseModeToolBar.h"

// MRMLLogic includes
#include <vtkMRMLApplicationLogic.h>

// MRML includes
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkSmartPointer.h>

class qSlicerMouseModeToolBarPrivate;
class QAction;
class QActionGroup;

class qSlicerMouseModeToolBarPrivate: public QObject
{
  Q_OBJECT
  QVTK_OBJECT
  Q_DECLARE_PUBLIC(qSlicerMouseModeToolBar);

protected:
  qSlicerMouseModeToolBar* const q_ptr;

public:
  qSlicerMouseModeToolBarPrivate(qSlicerMouseModeToolBar& object);

  void init();
  void setMRMLScene(vtkMRMLScene* newScene);
  void updateWidgetFromMRML();

public slots:

  void onMRMLSceneAboutToBeClosedEvent();
  void onMRMLSceneImportedEvent();
  void onMRMLSceneClosedEvent();
  void onInteractionNodeModeChangedEvent();
  void onInteractionNodeModePersistenceChangedEvent();

public:

  vtkSmartPointer<vtkMRMLScene>            MRMLScene;
  vtkSmartPointer<vtkMRMLApplicationLogic> MRMLAppLogic;

  /// PickMode
  QAction*      SinglePickModeAction;
  QAction*      PersistentPickModeAction;

  /// PlaceMode
  QAction*      SinglePlaceModeAction;
  QAction*      PersistentPlaceModeAction;

  /// RotateMode
  QAction*      ViewTransformModeAction;

  QActionGroup* ActionGroup;

};

#endif
