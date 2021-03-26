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

#include "qSlicerMarkupsMalformedWidget.h"
#include "qSlicerMarkupsAdditionalOptionsWidget_p.h"

// --------------------------------------------------------------------------
class qSlicerMarkupsMalformedWidgetPrivate
  : public qSlicerMarkupsAdditionalOptionsWidgetPrivate
{
  Q_DECLARE_PUBLIC(qSlicerMarkupsMalformedWidget);

protected:
  qSlicerMarkupsMalformedWidget* const q_ptr;

public:
  qSlicerMarkupsMalformedWidgetPrivate(qSlicerMarkupsMalformedWidget* object);
  ~qSlicerMarkupsMalformedWidgetPrivate();
};

// --------------------------------------------------------------------------
qSlicerMarkupsMalformedWidgetPrivate::
qSlicerMarkupsMalformedWidgetPrivate(qSlicerMarkupsMalformedWidget* object)
  : q_ptr(object)
{
}

// --------------------------------------------------------------------------
qSlicerMarkupsMalformedWidgetPrivate::~qSlicerMarkupsMalformedWidgetPrivate() = default;

// --------------------------------------------------------------------------
qSlicerMarkupsMalformedWidget::
qSlicerMarkupsMalformedWidget(QWidget *parent)
  : Superclass(*new qSlicerMarkupsMalformedWidgetPrivate(this), parent)
{
}

// --------------------------------------------------------------------------
qSlicerMarkupsMalformedWidget::
qSlicerMarkupsMalformedWidget(qSlicerMarkupsMalformedWidgetPrivate &d,QWidget *parent)
  : Superclass(d, parent)
{
}
