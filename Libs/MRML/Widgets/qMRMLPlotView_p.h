/*==============================================================================

  Copyright (c) Kapteyn Astronomical Institute
  University of Groningen, Groningen, Netherlands. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Davide Punzo, Kapteyn Astronomical Institute,
  and was supported through the European Research Council grant nr. 291531.

==============================================================================*/

#ifndef __qMRMLPlotView_p_h
#define __qMRMLPlotView_p_h

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
class QToolButton;

// VTK includes
#include <vtkWeakPointer.h>

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>
class ctkPopupWidget;

// qMRML includes
#include "qMRMLPlotView.h"

// vtk includes
#include <vtkSmartPointer.h>

class vtkMRMLPlotViewNode;
class vtkMRMLPlotChartNode;
class vtkMRMLColorLogic;
class vtkMRMLColorNode;
class vtkObject;
class vtkStringArray;

//-----------------------------------------------------------------------------
class qMRMLPlotViewPrivate: public QObject
{
  Q_OBJECT
  QVTK_OBJECT
  Q_DECLARE_PUBLIC(qMRMLPlotView);
protected:
  qMRMLPlotView* const q_ptr;
public:
  qMRMLPlotViewPrivate(qMRMLPlotView& object);
  ~qMRMLPlotViewPrivate();

  virtual void init();

  void setMRMLScene(vtkMRMLScene* scene);
  vtkMRMLScene *mrmlScene();

public slots:
  /// Handle MRML scene event
  void startProcessing();
  void endProcessing();

  void updateWidgetFromMRML();
  void onPlotChartNodeChanged();

  void RecalculateBounds();
  void switchSelectionMode();
  void switchLeftAndMiddleClick();

  void emitSelection();

protected:

  vtkMRMLScene*                      MRMLScene;
  vtkMRMLPlotViewNode*               MRMLPlotViewNode;
  vtkMRMLPlotChartNode*              MRMLPlotChartNode;

  vtkWeakPointer<vtkMRMLColorLogic>  ColorLogic;

  QToolButton*                       PinButton;
  ctkPopupWidget*                    PopupWidget;
};

#endif
