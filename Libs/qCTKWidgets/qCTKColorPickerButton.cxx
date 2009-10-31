#include "qCTKColorPickerButton.h"

#include <QColorDialog>
#include <QIcon>
#include <QPainter>
#include <QPixmap>
#include <QStyle>

qCTKColorPickerButton::qCTKColorPickerButton(QWidget* parent)
  :QPushButton(parent)
{
  connect(this, SIGNAL(toggled(bool)), this, SLOT(changeColor(bool)));
  this->setColor(Qt::black);

  this->setCheckable(true);
}

qCTKColorPickerButton::qCTKColorPickerButton(const QString& text, QWidget* parent)
  :QPushButton(text, parent)
{
  connect(this, SIGNAL(clicked), this, SLOT(changeColor));
  this->setColor(Qt::black);

  // Customize
  this->setCheckable(true);
}

qCTKColorPickerButton::qCTKColorPickerButton(const QColor& color, const QString& text, QWidget* parent)
  :QPushButton(text, parent)
{
  connect(this, SIGNAL(clicked), this, SLOT(changeColor));
  this->setColor(color);

  // Customize
  this->setCheckable(true);
}

void qCTKColorPickerButton::changeColor(bool change)
{
  if (change)
    {
    this->setColor(QColorDialog::getColor(this->Color));

    this->setChecked(false);
    }
}

void qCTKColorPickerButton::setColor(const QColor& color)
{
  if (color == this->Color)
    {
    return;
    }

  int iconSize = style()->pixelMetric(QStyle::PM_SmallIconSize);
  QPixmap pix(iconSize, iconSize);
  pix.fill(palette().button().color());
  QPainter p(&pix);
  p.setPen(QPen(Qt::gray));
  p.setBrush(color);
  p.drawRect(2, 2, pix.width() - 5, pix.height() - 5);

  this->setIcon(QIcon(pix));
  this->setText(color.name());
  
  this->Color = color;
  emit colorChanged(this->Color);
}

QColor qCTKColorPickerButton::color()const
{
  return this->Color;
}
