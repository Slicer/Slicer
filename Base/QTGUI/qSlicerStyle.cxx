
// Qt includes
#include <QDebug>

// qMRML includes
#include "qSlicerStyle.h"

// --------------------------------------------------------------------------
// qSlicerStyle methods

// --------------------------------------------------------------------------
qSlicerStyle::qSlicerStyle() : Superclass()
{

}

// --------------------------------------------------------------------------
int qSlicerStyle::pixelMetric(PixelMetric metric, const QStyleOption * option,
                              const QWidget * widget)const
{
  switch(metric)
    {
    case QStyle::PM_LayoutLeftMargin:
    case QStyle::PM_LayoutTopMargin:
    case QStyle::PM_LayoutRightMargin:
    case QStyle::PM_LayoutBottomMargin:
      return 6;
      break;
    case QStyle::PM_LayoutHorizontalSpacing:
    case QStyle::PM_LayoutVerticalSpacing:
      return 3;
      break;
    default:
      return Superclass::pixelMetric(metric, option, widget);
      break;
    }
}


