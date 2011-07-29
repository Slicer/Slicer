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

#ifndef __qMRMLThreeDViewsControllerWidget_h
#define __qMRMLThreeDViewsControllerWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkAxesWidget.h>
class ctkVTKMagnifyView;

// qMRMLWidget includes
#include "qMRMLWidget.h"

#include "qMRMLWidgetsExport.h"

class qMRMLThreeDViewsControllerWidgetPrivate;
class vtkMRMLViewNode;
class vtkRenderer;

class QMRML_WIDGETS_EXPORT qMRMLThreeDViewsControllerWidget : public qMRMLWidget
{
  Q_OBJECT
  Q_PROPERTY(bool disableMagnification READ disableMagnification WRITE setDisableMagnification)
public:
  /// Superclass typedef
  typedef qMRMLWidget Superclass;

  /// Constructors
  explicit qMRMLThreeDViewsControllerWidget(QWidget* parent = 0);
  virtual ~qMRMLThreeDViewsControllerWidget();

  /// Provide access to the magnify widget
  ctkVTKMagnifyView * magnifyView();

  /// Boolean property to determine if magnification mode 
  /// on entering a view should be ignored
  /// (false by default)
  bool disableMagnification()const;
  void setDisableMagnification(const bool);

public slots:

  virtual void setMRMLScene(vtkMRMLScene* newScene);

  void setActiveMRMLThreeDViewNode(vtkMRMLViewNode * newActiveMRMLThreeDViewNode);
  void setActiveThreeDRenderer(vtkRenderer* newActiveRenderer);
  void setDisplayModeToNavigation();
  void setDisplayModeToMagnification();

signals:

  void screenshotButtonClicked();
  void sceneViewButtonClicked();

protected:
  QScopedPointer<qMRMLThreeDViewsControllerWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLThreeDViewsControllerWidget);
  Q_DISABLE_COPY(qMRMLThreeDViewsControllerWidget);
};

#endif
