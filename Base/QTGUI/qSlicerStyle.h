#ifndef __qSlicerStyle_h
#define __qSlicerStyle_h

// Qt includes
#include <QProxyStyle>

#include "qSlicerBaseQTGUIExport.h"


class Q_SLICER_BASE_QTGUI_EXPORT qSlicerStyle : public QProxyStyle
{
public:
  /// Superclass typedef
  typedef QProxyStyle Superclass;

  /// Constructors
  explicit qSlicerStyle(QStyle* style = 0);
  virtual ~qSlicerStyle(){}

  virtual int pixelMetric(PixelMetric metric, const QStyleOption * option = 0,
                          const QWidget * widget = 0)const;

  virtual QRect subControlRect(ComplexControl control, const QStyleOptionComplex *option,
                               SubControl subControl, const QWidget *widget) const;
};

#endif

