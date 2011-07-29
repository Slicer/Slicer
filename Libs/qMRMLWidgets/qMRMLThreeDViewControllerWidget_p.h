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

#ifndef __qMRMLThreeDViewControllerWidget_p_h
#define __qMRMLThreeDViewControllerWidget_p_h

// qMRML includes
#include "qMRMLThreeDViewControllerWidget.h"
#include "ui_qMRMLThreeDViewControllerWidget.h"

// VTK includes
#include <vtkWeakPointer.h>

class QAction;
class ctkButtonGroup;
class qMRMLActionSignalMapper;
class qMRMLSceneViewMenu;
class vtkMRMLViewNode;

//-----------------------------------------------------------------------------
class qMRMLThreeDViewControllerWidgetPrivate
  : public Ui_qMRMLThreeDViewControllerWidget
{
  Q_DECLARE_PUBLIC(qMRMLThreeDViewControllerWidget);

protected:
  qMRMLThreeDViewControllerWidget* const q_ptr;

public:
  qMRMLThreeDViewControllerWidgetPrivate(qMRMLThreeDViewControllerWidget& object);
  virtual ~qMRMLThreeDViewControllerWidgetPrivate();

  virtual void setupUi(QWidget* widget);

  vtkWeakPointer<vtkMRMLViewNode>  ViewNode;
  qMRMLThreeDView*                 ThreeDView;

  qMRMLActionSignalMapper*         StereoTypesMapper;
  ctkButtonGroup*                  AnimateViewButtonGroup;
};

#endif
