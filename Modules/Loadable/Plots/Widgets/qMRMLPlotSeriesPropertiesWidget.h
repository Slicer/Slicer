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

#ifndef __qMRMLPlotSeriesPropertiesWidget_h
#define __qMRMLPlotSeriesPropertiesWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkPimpl.h>

// Plots Widgets includes
#include "qSlicerPlotsModuleWidgetsExport.h"
#include "ui_qMRMLPlotSeriesPropertiesWidget.h"

class qMRMLPlotSeriesPropertiesWidgetPrivate;
class vtkMRMLNode;
class vtkMRMLPlotSeriesNode;

class vtkMRMLPlotViewLogic;

class Q_SLICER_MODULE_PLOTS_WIDGETS_EXPORT qMRMLPlotSeriesPropertiesWidget : public qMRMLWidget
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef qMRMLWidget Superclass;

  /// Constructors
  explicit qMRMLPlotSeriesPropertiesWidget(QWidget* parent = nullptr);
  ~qMRMLPlotSeriesPropertiesWidget() override;

  /// Get \a PlotViewNode
  vtkMRMLPlotSeriesNode* mrmlPlotSeriesNode()const;

public slots:

  /// Select plot series node to edit.
  void setMRMLPlotSeriesNode(vtkMRMLNode* node);

  /// Select plot series node to edit.
  void setMRMLPlotSeriesNode(vtkMRMLPlotSeriesNode* plotSeriesNode);

protected:
  QScopedPointer<qMRMLPlotSeriesPropertiesWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLPlotSeriesPropertiesWidget);
  Q_DISABLE_COPY(qMRMLPlotSeriesPropertiesWidget);
};

#endif
