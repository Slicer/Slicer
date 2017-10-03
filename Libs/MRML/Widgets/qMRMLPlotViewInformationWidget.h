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

#ifndef __qMRMLPlotViewInformationWidget_h
#define __qMRMLPlotViewInformationWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkPimpl.h>

// qMRMLWidget includes
#include "qMRMLWidget.h"

#include "qMRMLWidgetsExport.h"

class qMRMLPlotViewInformationWidgetPrivate;
class vtkMRMLNode;
class vtkMRMLPlotViewNode;

class vtkMRMLPlotViewLogic;

class QMRML_WIDGETS_EXPORT qMRMLPlotViewInformationWidget : public qMRMLWidget
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef qMRMLWidget Superclass;

  /// Constructors
  explicit qMRMLPlotViewInformationWidget(QWidget* parent = 0);
  virtual ~qMRMLPlotViewInformationWidget();

  /// Get \a PlotViewNode
  vtkMRMLPlotViewNode* mrmlPlotViewNode()const;

public slots:

  /// Set a new PlotViewNode.
  void setMRMLPlotViewNode(vtkMRMLNode* newNode);

  /// Set a new PlotViewNode.
  void setMRMLPlotViewNode(vtkMRMLPlotViewNode* newPlotViewNode);

  /// Set view group
  void setViewGroup(int viewGroup);

protected:
  QScopedPointer<qMRMLPlotViewInformationWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLPlotViewInformationWidget);
  Q_DISABLE_COPY(qMRMLPlotViewInformationWidget);
};

#endif
