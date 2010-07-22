// Qt includes
#include <QDebug>
#include <QWidget>

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
      return 3; // 6 by default
      break;
    default:
      return Superclass::pixelMetric(metric, option, widget);
      break;
    }
}


