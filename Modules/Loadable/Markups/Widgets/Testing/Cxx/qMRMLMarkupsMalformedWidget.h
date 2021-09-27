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

#ifndef __qMRMLMalformedWidget_h_
#define __qMRMLMalformedWidget_h_

// Markups widgets includes
#include "qMRMLMarkupsAbstractOptionsWidget.h"

//------------------------------------------------------------------------------
class qMRMLMarkupsMalformedWidgetPrivate;
class vtkMRMLMarkupsNode;

//------------------------------------------------------------------------------
class qMRMLMarkupsMalformedWidget : public qMRMLMarkupsAbstractOptionsWidget
{
  Q_OBJECT

  Q_PROPERTY(QString className READ className CONSTANT);

public:
  typedef qMRMLMarkupsAbstractOptionsWidget Superclass;
  qMRMLMarkupsMalformedWidget(QWidget* parent=nullptr);

  /// Gets the name of the additional options widget type
  const QString className() const override {return "";}

  void updateWidgetFromMRML() {}

  bool canManageMRMLMarkupsNode(vtkMRMLMarkupsNode *) const override {return false;}  /// Set the MRML node of interest

  void setMRMLMarkupsNode(vtkMRMLMarkupsNode* node) override;

  /// Returns an instance of the widget
  qMRMLMarkupsAbstractOptionsWidget* createInstance() const override
  { return new qMRMLMarkupsMalformedWidget(); }
};

#endif // __qMRMLMalformedWidget_h_
