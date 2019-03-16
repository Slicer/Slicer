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

#ifndef __qSlicerViewersToolBar_h
#define __qSlicerViewersToolBar_h

// Qt includes
#include <QToolBar>
#include <QMenu>

// CTK includes
#include "qSlicerBaseQTGUIExport.h"

class qSlicerViewersToolBarPrivate;
class vtkMRMLScene;
class vtkSlicerApplicationLogic;

///
/// qSlicerViewersToolBar is a toolbar that can be used to switch
/// modes of the various viewers. For example, toggling frequently
/// used modes like the Crosshair on the 2D Viewers.
/// \note The toolbar expects qSlicerCoreApplication::mrmlApplicationLogic() to return a valid object
class Q_SLICER_BASE_QTGUI_EXPORT qSlicerViewersToolBar: public QToolBar
{
  Q_OBJECT
public:
  typedef QToolBar Superclass;

  /// Constructor
  /// Title is the name of the toolbar (can appear using right click on the toolbar area)
  qSlicerViewersToolBar(const QString& title, QWidget* parent = nullptr);
  qSlicerViewersToolBar(QWidget* parent = nullptr);
  ~qSlicerViewersToolBar() override;

public slots:

  void setApplicationLogic(vtkSlicerApplicationLogic* logic);

  void setMRMLScene(vtkMRMLScene* newScene);

protected:
  QScopedPointer<qSlicerViewersToolBarPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerViewersToolBar);
  Q_DISABLE_COPY(qSlicerViewersToolBar);
};

#endif
