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
  virtual ~qSlicerMouseModeToolBar();

public slots:

  void setMRMLScene(vtkMRMLScene* newScene);

  void switchToPersistentPickMode();
  void switchToSinglePickMode();
  void switchToPersistentPlaceMode();
  void switchToSinglePlaceMode();
  void switchToViewTransformMode();


protected:
  QScopedPointer<qSlicerMouseModeToolBarPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerMouseModeToolBar);
  Q_DISABLE_COPY(qSlicerMouseModeToolBar);
};

#endif
