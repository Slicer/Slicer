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

#ifndef __qMRMLMarkupsTestLineWidget_h
#define __qMRMLMarkupsTestLineWidget_h

// Markups widgets includes
#include "qMRMLMarkupsAbstractOptionsWidget.h"
#include "qSlicerTemplateKeyModuleWidgetsExport.h"

class qMRMLMarkupsTestLineWidgetPrivate;
class vtkMRMLMarkupsNode;

class Q_SLICER_MODULE_TEMPLATEKEY_WIDGETS_EXPORT qMRMLMarkupsTestLineWidget : public qMRMLMarkupsAbstractOptionsWidget
{
  Q_OBJECT

  Q_PROPERTY(QString className READ className CONSTANT);

public:
  typedef qMRMLMarkupsAbstractOptionsWidget Superclass;
  qMRMLMarkupsTestLineWidget(QWidget* parent = nullptr);
  ~qMRMLMarkupsTestLineWidget() override;

  /// Gets the name of the additional options widget type
  const QString className() const override { return "qMRMLMarkupsTestLineWidget"; }

  /// Updates the widget based on information from MRML.
  void updateWidgetFromMRML() override;

  /// Checks whether a given node can be handled by the widget
  bool canManageMRMLMarkupsNode(vtkMRMLMarkupsNode* markupsNode) const override;

  /// Returns an instance of the widget
  qMRMLMarkupsAbstractOptionsWidget* createInstance() const override { return new qMRMLMarkupsTestLineWidget(); }

public slots:
  /// Set the MRML node of interest
  void setMRMLMarkupsNode(vtkMRMLMarkupsNode* node) override;

protected:
  void setup();

protected:
  QScopedPointer<qMRMLMarkupsTestLineWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLMarkupsTestLineWidget);
  Q_DISABLE_COPY(qMRMLMarkupsTestLineWidget);
};

#endif
