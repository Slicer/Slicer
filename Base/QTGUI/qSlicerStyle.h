#ifndef __qSlicerStyle_h
#define __qSlicerStyle_h

// Qt includes
#include <QCleanlooksStyle>

#include "qSlicerBaseQTGUIExport.h"


class Q_SLICER_BASE_QTGUI_EXPORT qSlicerStyle : public QCleanlooksStyle
{
public:
  /// Superclass typedef
  typedef QCleanlooksStyle Superclass;

  /// Constructors
  explicit qSlicerStyle();
  virtual ~qSlicerStyle(){}

  virtual int pixelMetric(PixelMetric metric, const QStyleOption * option = 0,
                          const QWidget * widget = 0)const;

};

#endif

