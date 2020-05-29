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
#include <QToolBar>
#include <QMenu>
#include <QCheckBox>

// CTK includes
#include <ctkPimpl.h>
#include <ctkSignalMapper.h>
#include <ctkVTKObject.h>
#include "qSlicerBaseQTGUIExport.h"

// Slicer includes
#include "qSlicerMouseModeToolBar.h"

// MRMLLogic includes
#include <vtkMRMLApplicationLogic.h>

// MRML includes
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

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

  /// update mouse cursor shape according to current interaction mode and selection
  void updateCursor();

  void updatePlaceWidgetMenuActionList();

  QCursor cursorFromIcon(QIcon& icon);

public slots:

  void onMRMLSceneStartBatchProcess();
  void onMRMLSceneEndBatchProcess();
  void updateWidgetFromMRML();

  void onActivePlaceNodeClassNameChangedEvent();
  void onPlaceNodeClassNameListModifiedEvent();

public:
  vtkSmartPointer<vtkMRMLScene>            MRMLScene;
  vtkSmartPointer<vtkMRMLApplicationLogic> MRMLAppLogic;
  vtkWeakPointer<vtkMRMLInteractionNode>   InteractionNode;

  QAction* AdjustViewAction;
  QAction* AdjustWindowLevelAction;
  QAction* PlaceWidgetAction;
  QMenu* PlaceWidgetMenu;

  QAction* AdjustWindowLevelAdjustModeAction;
  QAction* AdjustWindowLevelRegionModeAction;
  QAction* AdjustWindowLevelCenteredRegionModeAction;
  QMenu* AdjustWindowLevelMenu;

  ctkSignalMapper* AdjustWindowLevelModeMapper;

  /// Place Persistence
  QAction *PersistenceAction;

  /// Group interaction modes together so that they're exclusive
  QActionGroup* InteractionModesActionGroup;

  /// Group the place actions together so that they're exclusive
  QActionGroup* PlaceModesActionGroup;

  QString DefaultPlaceClassName;
};

#endif
