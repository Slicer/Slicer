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

#ifndef __qSlicerMouseModeToolBar_p_h
#define __qSlicerMouseModeToolBar_p_h

// Qt includes
#include <QToolBar>
#include <QMenu>
#include <QCheckBox>

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
class QToolButton;

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
  /// given an place node class name, find the action associated with it and set it
  /// checked, update the cursor, update the icon on the button
  void updateWidgetToPlace(const char *placeNodeClassName);

public slots:

  void onMRMLSceneStartBatchProcess();
  void onMRMLSceneEndBatchProcess();

  void updateWidgetFromSelectionNode();
  void updateWidgetFromInteractionNode();

  void onActivePlaceNodeClassNameChangedEvent();
  void onPlaceNodeClassNameListModifiedEvent();
public:

  vtkSmartPointer<vtkMRMLScene>            MRMLScene;
  vtkSmartPointer<vtkMRMLApplicationLogic> MRMLAppLogic;

  /// PlaceMode button and menu
  QToolButton *CreateAndPlaceToolButton;
  QMenu*        CreateAndPlaceMenu;

  /// Place Persistence
  QAction *PersistenceAction;

  /// Group the place actions together so that they're exclusive
  QActionGroup* ActionGroup;

  QString DefaultPlaceClassName;
};

#endif
