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

#ifndef __qSlicerCurveSettingsWidget_h_
#define __qSlicerCurveSettingsWidget_h_

// Markups widgets includes
#include "qMRMLMarkupsAbstractOptionsWidget.h"
#include "qSlicerMarkupsModuleWidgetsExport.h"

// ------------------------------------------------------------------------------
class qMRMLMarkupsCurveSettingsWidgetPrivate;
class vtkMRMLMarkupsNode;

// ------------------------------------------------------------------------------
class Q_SLICER_MODULE_MARKUPS_WIDGETS_EXPORT qMRMLMarkupsCurveSettingsWidget
  : public qMRMLMarkupsAbstractOptionsWidget
{
  Q_OBJECT

public:
  typedef qMRMLMarkupsAbstractOptionsWidget Superclass;
  qMRMLMarkupsCurveSettingsWidget(QWidget* parent=nullptr);
  ~qMRMLMarkupsCurveSettingsWidget() override;

  /// Gets the name of the additional options widget type
  const QString className() const override {return "qMRMLMarkupsCurveSettingsWidget";}

  /// Checks whether a given node can be handled by the widget
  bool canManageMRMLMarkupsNode(vtkMRMLMarkupsNode *markupsNode) const override;

  /// Updates the widget on MRML changes
  void updateWidgetFromMRML() override;

  /// Set the MRML node of interest
  void setMRMLMarkupsNode(vtkMRMLMarkupsNode* node) override;

  /// Returns an instance of the widget
  qMRMLMarkupsAbstractOptionsWidget* createInstance() const override
  { return new qMRMLMarkupsCurveSettingsWidget(); }

public slots:
  void onCurveTypeParameterChanged();
  void onProjectCurveMaximumSearchRadiusChanged();
  void onApplyCurveResamplingPushButtonClicked();
  void setMRMLScene(vtkMRMLScene* scene) override;

protected:
  qMRMLMarkupsCurveSettingsWidget(QWidget* parent, qMRMLMarkupsCurveSettingsWidgetPrivate &d);

protected:
  void setup();

protected:
  QScopedPointer<qMRMLMarkupsCurveSettingsWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLMarkupsCurveSettingsWidget);
  Q_DISABLE_COPY(qMRMLMarkupsCurveSettingsWidget);
};

#endif // __qSlicerCurveSettingsWidget_h_
