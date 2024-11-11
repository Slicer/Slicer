/*=========================================================================

  Library:   CTK

  Copyright (c) Kitware Inc.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      https://www.apache.org/licenses/LICENSE-2.0.txt

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
  QString text()const;

  qSlicerTerminologyNavigatorWidget::TerminologyInfoBundle TerminologyInfo;
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
  QObject::connect(q, SIGNAL(toggled(bool)), q, SLOT(onToggled(bool)));
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

  this->update();
}

//-----------------------------------------------------------------------------
void qSlicerTerminologySelectorButton::paintEvent(QPaintEvent *)
{
  Q_D(qSlicerTerminologySelectorButton);
  QStylePainter p(this);
  QStyleOptionButton option;
  this->initStyleOption(&option);
  option.text = d->TerminologyInfo.Name;
  p.drawControl(QStyle::CE_PushButton, option);
}
