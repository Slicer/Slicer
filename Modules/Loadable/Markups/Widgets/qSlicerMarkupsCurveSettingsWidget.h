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

#ifndef __qslicercurvesettingswidget_h_
#define __qslicercurvesettingswidget_h_

// Markups widgets includes
#include "qSlicerMarkupsAdditionalOptionsWidget.h"
#include "qSlicerMarkupsModuleWidgetsExport.h"

class qSlicerMarkupsCurveSettingsWidgetPrivate;
class vtkMRMLMarkupsNode;

class Q_SLICER_MODULE_MARKUPS_WIDGETS_EXPORT
qSlicerMarkupsCurveSettingsWidget : public qSlicerMarkupsAdditionalOptionsWidget
{
  Q_OBJECT

public:
  typedef qSlicerMarkupsAdditionalOptionsWidget Superclass;
  qSlicerMarkupsCurveSettingsWidget(QWidget* parent=nullptr);
  ~qSlicerMarkupsCurveSettingsWidget() override;

  /// Gets the name of the additional options widget type
  const QString getAdditionalOptionsWidgetTypeName() override { return "CurveSettings"; }

  /// Updates the widget based on information from MRML.
  void updateWidgetFromMRML() override;

  /// Checks whether a given node can be handled by the widget
  bool canManageMRMLMarkupsNode(vtkMRMLMarkupsNode *markupsNode) const override;

public slots:
  void onCurveTypeParameterChanged();
  void onApplyCurveResamplingPushButtonClicked();

protected:
  qSlicerMarkupsCurveSettingsWidget(qSlicerMarkupsCurveSettingsWidgetPrivate &d, QWidget* parent=nullptr);
  void setup();

private:
  Q_DECLARE_PRIVATE(qSlicerMarkupsCurveSettingsWidget);
  Q_DISABLE_COPY(qSlicerMarkupsCurveSettingsWidget);
};

#endif // __qslicercurvesettingswidget_h_
