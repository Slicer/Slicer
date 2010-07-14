#ifndef __qMRMLRangeWidget_h
#define __qMRMLRangeWidget_h

// CTK includes
#include <ctkDoubleRangeSlider.h>
#include <ctkRangeSlider.h>
#include <ctkRangeWidget.h>

// qMRML includes
#include "qMRMLWidgetsExport.h"

class qMRMLRangeSliderPrivate;

///
/// qMRMLRangeWidget is a wrapper around a ctkRangewidget
class QMRML_WIDGETS_EXPORT qMRMLRangeWidget : public ctkRangeWidget
{
  Q_OBJECT
  Q_PROPERTY(QPalette minimumHandlePalette READ minimumHandlePalette WRITE setMinimumHandlePalette)
  Q_PROPERTY(QPalette maximumHandlePalette READ maximumHandlePalette WRITE setMaximumHandlePalette)

public:
  /// Constructor
  /// If \li parent is null, qMRMLRangeWidget will be a top-leve widget
  /// \note The \li parent can be set later using QWidget::setParent()
  explicit qMRMLRangeWidget(QWidget* parent = 0);

  QPalette minimumHandlePalette()const;
  QPalette maximumHandlePalette()const;

public slots:
  /// Set the palette of the minimum handle
  void setMinimumHandlePalette(const QPalette& palette);

  /// Set the palette of the minimum handle
  void setMaximumHandlePalette(const QPalette& palette);
};

class QMRML_WIDGETS_EXPORT qMRMLDoubleRangeSlider : public ctkDoubleRangeSlider
{
  Q_OBJECT;
public:
  qMRMLDoubleRangeSlider(QWidget* parentWidget);
  QPalette minimumHandlePalette()const;
  QPalette maximumHandlePalette()const;

public slots:
  /// Set the palette of the minimum handle
  void setMinimumHandlePalette(const QPalette& palette);

  /// Set the palette of the minimum handle
  void setMaximumHandlePalette(const QPalette& palette);
};

class QMRML_WIDGETS_EXPORT qMRMLRangeSlider : public ctkRangeSlider
{
  Q_OBJECT;
public:
  qMRMLRangeSlider(QWidget* parentWidget);
  QPalette minimumHandlePalette()const;
  QPalette maximumHandlePalette()const;

public slots:
  /// Set the palette of the minimum handle
  void setMinimumHandlePalette(const QPalette& palette);

  /// Set the palette of the minimum handle
  void setMaximumHandlePalette(const QPalette& palette);
protected:
  virtual void initMinimumSliderStyleOption(QStyleOptionSlider* option) const;
  virtual void initMaximumSliderStyleOption(QStyleOptionSlider* option) const;

private:
  CTK_DECLARE_PRIVATE(qMRMLRangeSlider);
};

#endif
