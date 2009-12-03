#ifndef __qCTKColorPickerButton_h
#define __qCTKColorPickerButton_h

#include <QPushButton>
#include <QColor>

#include "qCTKWidgetsWin32Header.h"

class QCTK_WIDGETS_EXPORT qCTKColorPickerButton : public QPushButton
{
  Q_OBJECT
  Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged USER true)
public:
  qCTKColorPickerButton(QWidget* parent = 0);
  qCTKColorPickerButton(const QString& text, QWidget* parent = 0 );
  qCTKColorPickerButton(const QColor& color, const QString & text, QWidget* parent = 0 );
  virtual ~qCTKColorPickerButton(){}
  
  QColor color()const;

public slots:
  void setColor(const QColor& color);
  void changeColor(bool change = true);

signals:
  void colorChanged(QColor);

protected:
  QColor Color;
};

#endif
