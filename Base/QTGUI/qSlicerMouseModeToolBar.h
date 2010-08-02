#ifndef __qSlicerMouseModeToolBar_h
#define __qSlicerMouseModeToolBar_h

// Qt includes
#include <QToolBar>

// CTK includes
#include <ctkPimpl.h>
#include "qSlicerBaseQTGUIExport.h"

class qSlicerMouseModeToolBarPrivate;
class vtkMRMLScene;

///
/// qSlicerMouseModeToolBar is a toolbar that can be used to switch between 
/// mouse modes: PickMode, PickModePersistent, PlaceMode, PlaceModePersistent, TransformMode
/// \note The toolbar expects qSlicerCoreApplication::mrmlApplicationLogic() to return a valid object
class Q_SLICER_BASE_QTGUI_EXPORT qSlicerMouseModeToolBar: public QToolBar
{
  Q_OBJECT
public:
  typedef QToolBar Superclass;

  /// Constructor
  /// Title is the name of the toolbar (can appear using right click on the toolbar area)
  qSlicerMouseModeToolBar(const QString& title, QWidget* parent = 0);
  qSlicerMouseModeToolBar(QWidget* parent = 0);

public slots:

  void setMRMLScene(vtkMRMLScene* newScene);

  void switchToPersistentPickMode();
  void switchToSinglePickMode();
  void switchToPersistentPlaceMode();
  void switchToSinglePlaceMode();
  void switchToViewTransformMode();


private:
  CTK_DECLARE_PRIVATE(qSlicerMouseModeToolBar);
};

#endif
