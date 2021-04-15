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

#ifndef __qslicertestlinewidget_h_
#define __qslicertestlinewidget_h_

// Markups widgets includes
#include "qSlicerMarkupsAdditionalOptionsWidget.h"
#include "qSlicerTemplateKeyModuleWidgetsExport.h"

class qSlicerMarkupsTestLineWidgetPrivate;
class vtkMRMLMarkupsNode;

class Q_SLICER_MODULE_TEMPLATEKEY_WIDGETS_EXPORT
qSlicerMarkupsTestLineWidget : public qSlicerMarkupsAdditionalOptionsWidget
{
  Q_OBJECT

public:

  typedef qSlicerMarkupsAdditionalOptionsWidget Superclass;
  qSlicerMarkupsTestLineWidget(QWidget* parent=nullptr);
  ~qSlicerMarkupsTestLineWidget() override;

  /// Updates the widget based on information from MRML.
  void updateWidgetFromMRML() override;

  /// Gets the name of the additional options widget type
  const QString getAdditionalOptionsWidgetTypeName() override { return "TestLine"; }

  /// Checks whether a given node can be handled by the widget
  bool canManageMRMLMarkupsNode(vtkMRMLMarkupsNode *markupsNode) const override;


protected:
  qSlicerMarkupsTestLineWidget(qSlicerMarkupsTestLineWidgetPrivate &d, QWidget* parent=nullptr);
  void setup();

private:
  Q_DECLARE_PRIVATE(qSlicerMarkupsTestLineWidget);
  Q_DISABLE_COPY(qSlicerMarkupsTestLineWidget);
};

#endif // __qslicertestlinewidget_h_
