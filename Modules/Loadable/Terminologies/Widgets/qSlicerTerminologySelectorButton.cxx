/*=========================================================================

  Library:   CTK

  Copyright (c) Kitware Inc.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

=========================================================================*/

// Terminology includes
#include "qSlicerTerminologySelectorButton.h"
#include "qSlicerTerminologySelectorDialog.h"
#include "vtkSlicerTerminologyEntry.h"

// Qt includes
#include <QApplication>
#include <QDebug>
#include <QIcon>
#include <QPainter>
#include <QPixmap>
#include <QStyle>
#include <QStyleOptionButton>
#include <QStylePainter>

class qSlicerTerminologySelectorButtonPrivate
{
  Q_DECLARE_PUBLIC(qSlicerTerminologySelectorButton);
protected:
  qSlicerTerminologySelectorButton* const q_ptr;
public:
  qSlicerTerminologySelectorButtonPrivate(qSlicerTerminologySelectorButton& object);
  void init();
  void computeIcon();
  QString text()const;

  qSlicerTerminologyNavigatorWidget::TerminologyInfoBundle TerminologyInfo;
  QIcon Icon;
  mutable QSize CachedSizeHint;
};

//-----------------------------------------------------------------------------
qSlicerTerminologySelectorButtonPrivate::qSlicerTerminologySelectorButtonPrivate(qSlicerTerminologySelectorButton& object)
  : q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
void qSlicerTerminologySelectorButtonPrivate::init()
{
  Q_Q(qSlicerTerminologySelectorButton);
  q->setCheckable(true);
  QObject::connect(q, SIGNAL(toggled(bool)),
                   q, SLOT(onToggled(bool)));
  this->computeIcon();
}

//-----------------------------------------------------------------------------
void qSlicerTerminologySelectorButtonPrivate::computeIcon()
{
  Q_Q(qSlicerTerminologySelectorButton);

  QColor iconColor;
  if (this->TerminologyInfo.Color.isValid())
    {
    // If custom color was chosen then use that
    iconColor = this->TerminologyInfo.Color;
    }
  else
    {
    // If recommended color is used then show that
    iconColor = qSlicerTerminologyNavigatorWidget::recommendedColorFromTerminology(
      this->TerminologyInfo.GetTerminologyEntry() );
    }

  int _iconSize = q->style()->pixelMetric(QStyle::PM_SmallIconSize);
  QPixmap pix(_iconSize, _iconSize);
  pix.fill(iconColor.isValid() ? q->palette().button().color() : Qt::transparent);
  QPainter p(&pix);
  p.setPen(QPen(Qt::gray));
  p.setBrush(iconColor.isValid() ? iconColor : QBrush(Qt::NoBrush));
  p.drawRect(2, 2, pix.width() - 5, pix.height() - 5);

  this->Icon = QIcon(pix);
}

//-----------------------------------------------------------------------------
qSlicerTerminologySelectorButton::qSlicerTerminologySelectorButton(QWidget* _parent)
  : QPushButton(_parent)
  , d_ptr(new qSlicerTerminologySelectorButtonPrivate(*this))
{
  Q_D(qSlicerTerminologySelectorButton);
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerTerminologySelectorButton::~qSlicerTerminologySelectorButton() = default;

//-----------------------------------------------------------------------------
void qSlicerTerminologySelectorButton::changeTerminology()
{
  Q_D(qSlicerTerminologySelectorButton);
  if (qSlicerTerminologySelectorDialog::getTerminology(d->TerminologyInfo, this))
    {
    d->computeIcon();
    this->update();
    emit terminologyChanged();
    }
  else
    {
    emit canceled();
    }
}

//-----------------------------------------------------------------------------
void qSlicerTerminologySelectorButton::onToggled(bool change)
{
  if (change)
    {
    this->changeTerminology();
    this->setChecked(false);
    }
}

//-----------------------------------------------------------------------------
void qSlicerTerminologySelectorButton::terminologyInfo(
  qSlicerTerminologyNavigatorWidget::TerminologyInfoBundle &terminologyInfo )
{
  Q_D(qSlicerTerminologySelectorButton);
  terminologyInfo = d->TerminologyInfo;
}

//-----------------------------------------------------------------------------
void qSlicerTerminologySelectorButton::setTerminologyInfo(
  qSlicerTerminologyNavigatorWidget::TerminologyInfoBundle &terminologyInfo )
{
  Q_D(qSlicerTerminologySelectorButton);

  d->TerminologyInfo = terminologyInfo;

  d->computeIcon();
  this->update();
}

//-----------------------------------------------------------------------------
void qSlicerTerminologySelectorButton::paintEvent(QPaintEvent *)
{
  Q_D(qSlicerTerminologySelectorButton);
  QStylePainter p(this);
  QStyleOptionButton option;
  this->initStyleOption(&option);
  option.icon = d->Icon;
  p.drawControl(QStyle::CE_PushButton, option);
}
