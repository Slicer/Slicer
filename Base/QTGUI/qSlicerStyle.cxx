/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QAbstractScrollArea>
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QCleanlooksStyle>
#else
#include <QCommonStyle>
#endif
#include <QDebug>
#include <QEvent>
#include <QGroupBox>
#include <QScrollBar>
#include <QStyleOptionGroupBox>

// qMRML includes
#include "qSlicerStyle.h"

// --------------------------------------------------------------------------
// qSlicerStyle methods

// --------------------------------------------------------------------------
qSlicerStyle::qSlicerStyle()
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
  : Superclass(new QCleanlooksStyle)
#else
  : Superclass(new QCommonStyle)
#endif
{
  this->baseStyle()->setParent(this);
}

// --------------------------------------------------------------------------
qSlicerStyle::~qSlicerStyle()
{

}

//------------------------------------------------------------------------------
QStyle::SubControl qSlicerStyle::hitTestComplexControl(ComplexControl cc, const QStyleOptionComplex *opt,
                                                       const QPoint &pt, const QWidget *widget) const
{
  SubControl sc = SC_None;
  switch (cc) {
  // Hot area for the groove shall be the entire height of the widget
#ifndef QT_NO_SLIDER
    case CC_Slider:
        if (const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(opt)) {
            QRect r = proxy()->subControlRect(cc, slider, SC_SliderHandle, widget);
            if (r.isValid() && r.contains(pt))
              {
              sc = SC_SliderHandle;
              }
            else
              {
              r = proxy()->subControlRect(cc, slider, SC_SliderGroove ,widget);
              QPoint center = r.center();
              if (slider->orientation == Qt::Horizontal)
                {
                // If there is no widget, use QStyle::PM_SliderThickness
                r.setHeight(widget ? widget->height() :
                            this->pixelMetric(QStyle::PM_SliderThickness, opt, widget));
                }
              else
                {
                // If there is no widget, use QStyle::PM_SliderThickness
                r.setWidth(widget ? widget->width() :
                           this->pixelMetric(QStyle::PM_SliderThickness, opt, widget));
                }
              r.moveCenter(center);
              if (r.isValid() && r.contains(pt))
                {
                sc = SC_SliderGroove;
                }
            }
        }
        break;
#endif // QT_NO_SLIDER
    default:
      sc = Superclass::hitTestComplexControl(cc, opt, pt, widget);
      break;
    }
  return sc;
}

// --------------------------------------------------------------------------
int qSlicerStyle::pixelMetric(PixelMetric metric, const QStyleOption * option,
                              const QWidget * widget)const
{
  switch(metric)
    {
    case QStyle::PM_ButtonMargin:
      return 3; // 6 by default
      break;
    case QStyle::PM_LayoutLeftMargin:
    case QStyle::PM_LayoutTopMargin:
    case QStyle::PM_LayoutRightMargin:
    case QStyle::PM_LayoutBottomMargin:
      if (widget && widget->inherits("ctkCollapsibleButton"))
        {
        return 4; // 9 by default
        }
      return 6; // 9 by default
      break;
    case QStyle::PM_LayoutHorizontalSpacing:
    case QStyle::PM_LayoutVerticalSpacing:
      return 3; // 6 by default
      break;
    case QStyle::PM_SliderLength:
      return 12; // default to 27
      break;
    default:
      return Superclass::pixelMetric(metric, option, widget);
      break;
    }
}

// --------------------------------------------------------------------------
QRect qSlicerStyle::subControlRect(ComplexControl control, const QStyleOptionComplex *option,
                                   SubControl subControl, const QWidget *widget) const
{
  const QCommonStyle * commonStyle = qobject_cast<const QCommonStyle*>(this->proxy());
  Q_ASSERT(commonStyle);
  QRect rect = commonStyle->QCommonStyle::subControlRect(
      control, option, subControl, widget);

  /// Using QCleanLookStyle, a extra margin of 16 is added to the groubBox,
  /// the following code aims at overriding that value by setting it to 4.
  switch(control)
    {
    case CC_GroupBox:
      if (const QStyleOptionGroupBox *groupBox =
          qstyleoption_cast<const QStyleOptionGroupBox *>(option))
        {
        int topMargin = 0;
        int topHeight = 0;
        int verticalAlignment = this->proxy()->styleHint(
            SH_GroupBox_TextLabelVerticalAlignment, groupBox, widget);
        bool flat = groupBox->features & QStyleOptionFrameV2::Flat;
        if (!groupBox->text.isEmpty())
          {
          topHeight = groupBox->fontMetrics.height();
          if (verticalAlignment & Qt::AlignVCenter)
            {
            topMargin = topHeight / 2;
            }
          else if (verticalAlignment & Qt::AlignTop)
            {
            topMargin = topHeight;
            }
          }
        QRect frameRect = groupBox->rect;
        frameRect.setTop(topMargin);
        if (subControl == SC_GroupBoxFrame)
          {
          return rect;
          }
        else if (subControl == SC_GroupBoxContents)
          {
          if(flat)
            {
            int margin = 0;
            int leftMarginExtension = 4; // default 16
            rect = frameRect.adjusted(leftMarginExtension + margin, margin + topHeight, -margin, -margin);
            }
          break;
          }
        if(flat)
          {
          if (const QGroupBox *groupBoxWidget = qobject_cast<const QGroupBox *>(widget))
            {
            //Prepare metrics for a bold font
            QFont font = widget->font();
            font.setBold(true);
            QFontMetrics fontMetrics(font);

            QSize textRect = fontMetrics.boundingRect(groupBoxWidget->title()).size() + QSize(2, 2);
            if (subControl == SC_GroupBoxCheckBox)
              {
              int indicatorWidth = proxy()->pixelMetric(PM_IndicatorWidth, option, widget);
              int indicatorHeight = proxy()->pixelMetric(PM_IndicatorHeight, option, widget);
              rect.setWidth(indicatorWidth);
              rect.setHeight(indicatorHeight);
              rect.moveTop((fontMetrics.height() - indicatorHeight) / 2 + 2);
              }
            else if (subControl == SC_GroupBoxLabel)
              {
              rect.setSize(textRect);
              }
            }
          }
        }
      break;
#ifndef QT_NO_SLIDER
    // <HACK>
    // Reimplemented to work around bug: http://bugreports.qt.nokia.com/browse/QTBUG-13318
    case CC_Slider:
        if (const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(option))
          {
          int tickSize = proxy()->pixelMetric(PM_SliderTickmarkOffset, option, widget);
          if (subControl == SC_SliderHandle)
            {
            if (slider->orientation == Qt::Horizontal)
              {
              rect.setHeight(this->pixelMetric(PM_SliderThickness));
              rect.setWidth(this->pixelMetric(PM_SliderLength));
              int centerY = slider->rect.center().y() - rect.height() / 2;
              if (slider->tickPosition & QSlider::TicksAbove)
                {
                centerY += tickSize;
                }
              if (slider->tickPosition & QSlider::TicksBelow)
                {
                centerY -= tickSize;
                }
              rect.moveTop(centerY);
              }
            else
              {
              rect.setWidth(this->pixelMetric(PM_SliderThickness));
              rect.setHeight(this->pixelMetric(PM_SliderLength));
              int centerX = slider->rect.center().x() - rect.width() / 2;
              if (slider->tickPosition & QSlider::TicksAbove)
                {
                centerX += tickSize;
                }
              if (slider->tickPosition & QSlider::TicksBelow)
                {
                centerX -= tickSize;
                }
              rect.moveLeft(centerX);
              }
            break;
            }
          }
        // </HACK>
#endif // QT_NO_SLIDER
    default:
      rect = Superclass::subControlRect(control, option, subControl, widget);
      break;
    }
  return rect;
}

//------------------------------------------------------------------------------
QPalette qSlicerStyle::standardPalette()const
{
  QPalette palette = this->Superclass::standardPalette();
  palette.setColor(QPalette::Button, "#fcfcfc");
  palette.setColor(QPalette::Light, "#c8c8c8");
  palette.setColor(QPalette::Midlight, "#e6e6e6");
  palette.setColor(QPalette::Dark, "#aaaaaa");
  palette.setColor(QPalette::Mid, "#c8c8c8");
  palette.setColor(QPalette::Base, Qt::white);
  palette.setColor(QPalette::Window, Qt::white);
  palette.setColor(QPalette::Shadow, "#5a5a5a");
  palette.setColor(QPalette::AlternateBase, QColor("#e4e4fe"));
  return palette;
}

//------------------------------------------------------------------------------
int qSlicerStyle::styleHint(StyleHint hint, const QStyleOption *opt, const QWidget *widget,
                             QStyleHintReturn *returnData) const
{
  int res;
  switch(hint)
    {
    case QStyle::SH_ItemView_ActivateItemOnSingleClick:
      if (widget && widget->property("SH_ItemView_ActivateItemOnSingleClick").isValid())
        {
        res = widget->property("SH_ItemView_ActivateItemOnSingleClick").toBool();
        break;
        }
    default:
      res = this->Superclass::styleHint(hint, opt, widget, returnData);
    }
  return res;
}

//------------------------------------------------------------------------------
bool qSlicerStyle::eventFilter(QObject* obj, QEvent* event)
{
  QWidget* widget = qobject_cast<QWidget*>(obj);
  switch (event->type())
    {
    case QEvent::Wheel:
      if (qobject_cast<QAbstractScrollArea*>(widget) ||
          qobject_cast<QScrollBar*>(widget) ||
          qobject_cast<QAbstractScrollArea*>(widget->parentWidget()))
        {
        break;
        }
      for (QWidget* ancestor = widget->parentWidget();
           ancestor; ancestor = ancestor->parentWidget())
        {
        if (QAbstractScrollArea* scrollArea =
            qobject_cast<QAbstractScrollArea*>(ancestor))
          {
          if (scrollArea->verticalScrollBar()->minimum() !=
              scrollArea->verticalScrollBar()->maximum())
            {
            event->ignore();
            return true;
            }
          }
        }
      break;
    default:
      break;
    }
  return this->Superclass::eventFilter(obj, event);
}

