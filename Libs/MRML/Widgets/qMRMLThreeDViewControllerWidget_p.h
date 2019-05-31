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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qMRMLThreeDViewControllerWidget_p_h
#define __qMRMLThreeDViewControllerWidget_p_h

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

// qMRML includes
#include "qMRMLThreeDViewControllerWidget.h"
#include "qMRMLViewControllerBar_p.h"
#include "ui_qMRMLThreeDViewControllerWidget.h"

// MRMLLogic includes
#include <vtkMRMLViewLogic.h>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

class QAction;
class ctkButtonGroup;
class ctkSignalMapper;
class qMRMLSceneViewMenu;
class vtkMRMLCameraNode;
class vtkMRMLViewNode;
class QString;

//-----------------------------------------------------------------------------
class qMRMLThreeDViewControllerWidgetPrivate
  : public qMRMLViewControllerBarPrivate
  , public Ui_qMRMLThreeDViewControllerWidget
{
  Q_DECLARE_PUBLIC(qMRMLThreeDViewControllerWidget);
public:
  typedef qMRMLViewControllerBarPrivate Superclass;
  qMRMLThreeDViewControllerWidgetPrivate(qMRMLThreeDViewControllerWidget& object);
  ~qMRMLThreeDViewControllerWidgetPrivate() override;

  void init() override;

  vtkMRMLViewLogic* viewNodeLogic(vtkMRMLViewNode* node);

  vtkWeakPointer<vtkMRMLViewNode>     ViewNode;
  vtkWeakPointer<vtkMRMLCameraNode>   CameraNode;
  qMRMLThreeDView*                    ThreeDView;

  vtkSmartPointer<vtkMRMLViewLogic>   ViewLogic;
  vtkCollection*                      ViewLogics;

  ctkSignalMapper*                    StereoTypesMapper;
  ctkButtonGroup*                     AnimateViewButtonGroup;
  ctkSignalMapper*                    OrientationMarkerTypesMapper;
  ctkSignalMapper*                    OrientationMarkerSizesMapper;
  ctkSignalMapper*                    RulerTypesMapper;
  ctkSignalMapper*                    RulerColorMapper;

  QString                             ThreeDViewLabel;
  QToolButton*                        CenterToolButton;

protected:
  void setupPopupUi() override;
};

#endif
