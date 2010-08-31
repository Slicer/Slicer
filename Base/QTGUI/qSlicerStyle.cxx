// Qt includes
#include <QDebug>
#include <QWidget>
#include <QStyleOptionGroupBox>
#include <QGroupBox>
#include <QWindowsStyle>
#include <QCommonStyle>
#include <QCleanlooksStyle>

// qMRML includes
#include "qSlicerStyle.h"

// --------------------------------------------------------------------------
// qSlicerStyle methods

// --------------------------------------------------------------------------
qSlicerStyle::qSlicerStyle(QStyle* style) : Superclass(style)
{

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
      if (widget && widget->objectName() == "MRMLSlicesControllerWidget")
        {// should be done at the application level..
        return 2;
        }
      return 3; // 6 by default
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
    default:
      return Superclass::subControlRect(control, option, subControl, widget);
      break;
    }
  return rect;
}


