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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qMRMLSlicesControllerToolBar_h
#define __qMRMLSlicesControllerToolBar_h

// Qt includes
#include <QSignalMapper>
#include <QToolBar>

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>
#include "qMRMLWidgetsExport.h"

class qMRMLSlicesControllerToolBarPrivate;
class vtkMRMLNode;
class vtkMRMLScene;
class vtkCollection;

class QMRML_WIDGETS_EXPORT qMRMLSlicesControllerToolBar : public QToolBar
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef QToolBar Superclass;
  explicit qMRMLSlicesControllerToolBar(QWidget* parent = 0);
  virtual ~qMRMLSlicesControllerToolBar();

  vtkMRMLScene* mrmlScene()const;

public slots:
  void setMRMLScene(vtkMRMLScene* scene);
  void setMRMLSliceLogics(vtkCollection* logics);

  void setFiducialPointsVisible(bool visible);
  void setFiducialLabelsVisible(bool visible);
  void setForegroundGridVisible(bool visible);
  void setBackgroundGridVisible(bool visible);
  void setLabelGridVisible(bool visible);
  void setLabelOpacity(double value);
  void toggleLabelOpacity(bool toggle);
  void fitToWindow();
  void setNavigatorEnabled(bool enable);
  void setSliceIntersectionVisible(bool visible);
  void toggleBackgroundForeground();
  void showBackground();
  void showForeground();
  void setForegroundOpacity(double value);
  void setAnnotationMode(int mode);
  void setCompositing(int mode);
  void setCrosshairMode(int mode);
  void setCrosshairThickness(int mode);
  void setAnnotationSpace(int mode);
  void setRedSliceFOV(double fov);
  void setYellowSliceFOV(double fov);
  void setGreenSliceFOV(double fov);
protected slots:
  void onMRMLSceneChanged(vtkObject*, void*, unsigned long, void*);
  void updateFromCompositeNode(vtkObject*);
  void updateFromCrosshairNode(vtkObject*);
  void updateFromSliceNode(vtkObject*);
protected:
  void connectNode(vtkMRMLNode*);
protected:
  QScopedPointer<qMRMLSlicesControllerToolBarPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLSlicesControllerToolBar);
  Q_DISABLE_COPY(qMRMLSlicesControllerToolBar);
};

#endif
