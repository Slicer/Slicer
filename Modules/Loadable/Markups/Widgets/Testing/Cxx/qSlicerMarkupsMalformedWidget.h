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

#ifndef __qslicermalformedwidget_h_
#define __qslicermalformedwidget_h_

// Markups widgets includes
#include "qSlicerMarkupsAdditionalOptionsWidget.h"

class qSlicerMarkupsMalformedWidgetPrivate;
class vtkMRMLMarkupsNode;

class qSlicerMarkupsMalformedWidget : public qSlicerMarkupsAdditionalOptionsWidget
{
  Q_OBJECT

public:
  typedef qSlicerMarkupsAdditionalOptionsWidget Superclass;
  qSlicerMarkupsMalformedWidget(QWidget* parent=nullptr);
  ~qSlicerMarkupsMalformedWidget()=default;

  const QString getAdditionalOptionsWidgetTypeName() override { return ""; }
  void updateWidgetFromMRML() override {}
  bool canManageMRMLMarkupsNode(vtkMRMLMarkupsNode *) const override {return false;}

protected:
  qSlicerMarkupsMalformedWidget(qSlicerMarkupsMalformedWidgetPrivate &d, QWidget* parent=nullptr);

private:
  Q_DECLARE_PRIVATE(qSlicerMarkupsMalformedWidget);
  Q_DISABLE_COPY(qSlicerMarkupsMalformedWidget);
};

#endif // __qslicermalformedwidget_h_
