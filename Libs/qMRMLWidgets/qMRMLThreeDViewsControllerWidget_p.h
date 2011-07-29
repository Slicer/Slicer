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

#ifndef __qMRMLThreeDViewsControllerWidget_p_h
#define __qMRMLThreeDViewsControllerWidget_p_h

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// qMRML includes
#include "qMRMLThreeDViewsControllerWidget.h"
#include "ui_qMRMLThreeDViewsControllerWidget.h"

// VTK includes
#include <vtkWeakPointer.h>

class QAction;
class ctkButtonGroup;
class qMRMLActionSignalMapper;
class qMRMLSceneViewMenu;
class vtkMRMLViewNode;

//-----------------------------------------------------------------------------
class qMRMLThreeDViewsControllerWidgetPrivate: public QObject,
                                   public Ui_qMRMLThreeDViewsControllerWidget
{
  Q_OBJECT
  QVTK_OBJECT
  Q_DECLARE_PUBLIC(qMRMLThreeDViewsControllerWidget);

protected:
  qMRMLThreeDViewsControllerWidget* const q_ptr;

public:
  qMRMLThreeDViewsControllerWidgetPrivate(qMRMLThreeDViewsControllerWidget& object);
  ~qMRMLThreeDViewsControllerWidgetPrivate(){}

  /// Enumeration over display modes
  enum DisplayMode {
    NavigationDisplayMode = 0,
    MagnificationDisplayMode
  };

  void setupUi(qMRMLWidget* widget);

public slots:

  void updateWidgetFromMRML();

  void createSceneView();

  void setDisplayMode(DisplayMode newMode);

public:

  // TODO In LayoutManager, use GetActive/IsActive flag ...
  vtkWeakPointer<vtkMRMLViewNode>  ActiveMRMLThreeDViewNode;

  qMRMLActionSignalMapper*         StereoTypesMapper;
  qMRMLSceneViewMenu*              SceneViewMenu;

  bool                             DisableMagnification;
  
};

#endif
