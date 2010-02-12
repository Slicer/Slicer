//Qt includes  
#include <QAbstractButton>
#include <QDebug>
#include <QWeakPointer>

#include "qCTKButtonGroup.h"

//-----------------------------------------------------------------------------
class qCTKButtonGroupPrivate : public qCTKPrivate<qCTKButtonGroup>
{
public:
  QCTK_DECLARE_PUBLIC(qCTKButtonGroup);
  bool IsLastButtonPressedChecked;
};

//------------------------------------------------------------------------------
qCTKButtonGroup::qCTKButtonGroup(QObject* parent)
  :QButtonGroup(parent)
{
  QCTK_INIT_PRIVATE(qCTKButtonGroup);
  connect(this, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(onButtonClicked(QAbstractButton*)));
  connect(this, SIGNAL(buttonPressed(QAbstractButton*)), this, SLOT(onButtonPressed(QAbstractButton*)));
}

//------------------------------------------------------------------------------
void qCTKButtonGroup::onButtonClicked(QAbstractButton *clickedButton)
{
  QCTK_D(qCTKButtonGroup);
  if (!this->exclusive() || !d->IsLastButtonPressedChecked)
    {
    return;
    }
  this->removeButton(clickedButton);
  clickedButton->setChecked(false);
  this->addButton(clickedButton);
}

//------------------------------------------------------------------------------
void qCTKButtonGroup::onButtonPressed(QAbstractButton *pressedButton)
{
  QCTK_D(qCTKButtonGroup);
  Q_ASSERT(pressedButton);
  d->IsLastButtonPressedChecked = pressedButton->isChecked();
}


