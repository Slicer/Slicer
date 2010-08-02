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

class qSlicerMouseModeToolBarPrivate: public QObject, public ctkPrivate<qSlicerMouseModeToolBar>
{
  Q_OBJECT
  QVTK_OBJECT

public:

  qSlicerMouseModeToolBarPrivate();
  void init();
  void setMRMLScene(vtkMRMLScene* newScene);
  void updateWidgetFromMRML();

public slots:

  void onMRMLSceneAboutToBeClosedEvent();
  void onMRMLSceneImportedEvent();

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
