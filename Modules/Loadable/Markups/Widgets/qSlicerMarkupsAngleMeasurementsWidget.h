/*==============================================================================

  Copyright (c) The Intervention Centre
  Oslo University Hospital, Oslo, Norway. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Rafael Palomar (The Intervention Centre,
  Oslo University Hospital) and was supported by The Research Council of Norway
  through the ALive project (grant nr. 311393).

==============================================================================*/

#ifndef __qslicermarkupsanglemeasurementswidget_h_
#define __qslicermarkupsanglemeasurementswidget_h_

// Markups widgets includes
#include "qSlicerMarkupsAdditionalOptionsWidget.h"
#include "qSlicerMarkupsModuleWidgetsExport.h"

class qSlicerMarkupsAngleMeasurementsWidgetPrivate;
class vtkMRMLMarkupsNode;

class Q_SLICER_MODULE_MARKUPS_WIDGETS_EXPORT
qSlicerMarkupsAngleMeasurementsWidget : public qSlicerMarkupsAdditionalOptionsWidget
{
  Q_OBJECT

public:

  typedef qSlicerMarkupsAdditionalOptionsWidget Superclass;
  qSlicerMarkupsAngleMeasurementsWidget(QWidget* parent=nullptr);
  ~qSlicerMarkupsAngleMeasurementsWidget() override;

  /// Updates the widget based on information from MRML.
  void updateWidgetFromMRML() override;

  /// Gets the name of the additional options widget type
  const QString getAdditionalOptionsWidgetTypeName() override { return "AngleMeasurements"; }

  /// Checks whether a given node can be handled by the widget
  bool canManageMRMLMarkupsNode(vtkMRMLMarkupsNode *markupsNode) const override;

public slots:

  /// Change angle mode of current angle markup if combobox selection is made.
  void onAngleMeasurementModeChanged();
  /// Update angle measurement rotation axis if the user edits the column vector
  void onRotationAxisChanged();

protected:
  qSlicerMarkupsAngleMeasurementsWidget(qSlicerMarkupsAngleMeasurementsWidgetPrivate &d, QWidget* parent=nullptr);
  void setup();

private:
  Q_DECLARE_PRIVATE(qSlicerMarkupsAngleMeasurementsWidget);
  Q_DISABLE_COPY(qSlicerMarkupsAngleMeasurementsWidget);
};

#endif // __qslicermarkupsanglemeasurementswidget_h_
