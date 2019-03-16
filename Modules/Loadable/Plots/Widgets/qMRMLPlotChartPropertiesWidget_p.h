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

#ifndef __qMRMLPlotChartPropertiesWidget_p_h
#define __qMRMLPlotChartPropertiesWidget_p_h

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

/// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// qMRML includes
#include "qMRMLPlotChartPropertiesWidget.h"
#include "ui_qMRMLPlotChartPropertiesWidget.h"

/// VTK includes
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>
#include <vtkImageData.h>

class QAction;
class vtkMRMLPlotChartNode;
class vtkMRMLPlotSeriesNode;
class vtkMRMLPlotViewNode;
class vtkObject;

//-----------------------------------------------------------------------------
class qMRMLPlotChartPropertiesWidgetPrivate
 : public QObject
 , public Ui_qMRMLPlotChartPropertiesWidget
{
  Q_OBJECT
  QVTK_OBJECT
  Q_DECLARE_PUBLIC(qMRMLPlotChartPropertiesWidget);
protected:
  qMRMLPlotChartPropertiesWidget* const q_ptr;
public:
  qMRMLPlotChartPropertiesWidgetPrivate(qMRMLPlotChartPropertiesWidget& object);
  ~qMRMLPlotChartPropertiesWidgetPrivate() override;

  void setupUi(qMRMLWidget* widget);

  vtkMRMLPlotChartNode* GetPlotChartNodeFromView();

public slots:
  /// Update widget state using the associated MRML PlotView node.
  void updateWidgetFromMRML();

  /// Called after an PlotSeriesNode is selected
  /// using the associated qMRMLNodeComboBox.
  void onPlotSeriesNodesSelected();

  /// Called after a PlotSeriesNode is added
  /// using the associated qMRMLNodeComboBox.
  void onPlotSeriesNodeAdded(vtkMRMLNode* node);

public:
  vtkWeakPointer<vtkMRMLPlotChartNode> PlotChartNode;

};

#endif
