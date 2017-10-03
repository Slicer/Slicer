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

#ifndef __qMRMLPlotViewControllerWidget_p_h
#define __qMRMLPlotViewControllerWidget_p_h

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
#include "qMRMLPlotViewControllerWidget.h"
#include "qMRMLViewControllerBar_p.h"
#include "ui_qMRMLPlotViewControllerWidget.h"

// VTK includes
#include <vtkWeakPointer.h>

class QAction;
class qMRMLSceneViewMenu;
class vtkMRMLPlotViewNode;
class vtkMRMLPlotChartNode;
class vtkMRMLSelectionNode;
class QString;

//-----------------------------------------------------------------------------
class qMRMLPlotViewControllerWidgetPrivate
  : public qMRMLViewControllerBarPrivate
  , public Ui_qMRMLPlotViewControllerWidget
{
  Q_OBJECT
  QVTK_OBJECT
  Q_DECLARE_PUBLIC(qMRMLPlotViewControllerWidget);

public:
  typedef qMRMLViewControllerBarPrivate Superclass;
  qMRMLPlotViewControllerWidgetPrivate(qMRMLPlotViewControllerWidget& object);
  virtual ~qMRMLPlotViewControllerWidgetPrivate();

  virtual void init();

  vtkWeakPointer<vtkMRMLPlotChartNode>   PlotChartNode;
  vtkWeakPointer<vtkMRMLPlotViewNode>    PlotViewNode;
  vtkWeakPointer<vtkMRMLSelectionNode>   SelectionNode;
  qMRMLPlotView*                         PlotView;

  QString                                PlotViewLabel;
  QToolButton*                           FitToWindowToolButton;

  vtkMRMLPlotChartNode* GetPlotChartNodeFromView();

public slots:
  /// Called after a PlotChartNode is selected
  /// using the associated qMRMLNodeComboBox.
  void onPlotChartNodeSelected(vtkMRMLNode* node);

  /// Called when the Singleton SelectionNode is modified.
  void onSelectionNodeModified();

  /// Called after an PlotDataNode is selected
  /// using the associated qMRMLNodeComboBox.
  void onPlotDataNodesSelected();

  /// Called after a PlotDataNode is added
  /// using the associated qMRMLNodeComboBox.
  void onPlotDataNodeAdded(vtkMRMLNode* node);

  /// Called after a PlotDataNode is going to be edited
  /// using the associated qMRMLNodeComboBox.
  void onPlotDataNodeEdited(vtkMRMLNode* node);

  /// Called after a Plot type is selected using the qComboBox
  /// Modify all the PlotDataNode selected in the PlotChartNode.
  void onPlotTypeChanged(const QString& Type);

  /// Called after a Column is selected using the qComboBox
  /// Modify all the PlotDataNode selected in the PlotChartNode.
  void onXAxisChanged(const QString& Column);

  /// Called after a Markers' option is selected using the qComboBox
  /// Modify all the PlotDataNode selected in the PlotChartNode.
  void onMarkersChanged(const QString& str);

protected:
  virtual void setupPopupUi();

public:

};

#endif
