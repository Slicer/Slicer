// Qt includes
#include <QDebug>
#include <QEvent>
#include <QStyle>

// CTK includes
#include <ctkLogger.h>
#include "qMRMLCollapsibleButton.h"

//--------------------------------------------------------------------------
static ctkLogger logger("org.slicer.libs.qmrmlwidgets.qMRMLCollapsibleButton");
//--------------------------------------------------------------------------
// --------------------------------------------------------------------------
// qMRMLCollapsibleButton methods

// --------------------------------------------------------------------------
qMRMLCollapsibleButton::qMRMLCollapsibleButton(QWidget* parentWidget)
  :Superclass(parentWidget)
{
  this->setAutoFillBackground(true);
  this->computePalette();
}

// --------------------------------------------------------------------------
void qMRMLCollapsibleButton::changeEvent(QEvent* event)
{
  if (event->type() == QEvent::ParentChange)
    {
    this->computePalette();
    }
  this->Superclass::changeEvent(event);
}

void qMRMLCollapsibleButton::computePalette()
{
  QColor backgroundColor =
    this->style()->standardPalette().color(QPalette::Window);
  QObject* ancestor = this;
  while(ancestor->parent())
    {
    ancestor = ancestor->parent();
    if (qobject_cast<qMRMLCollapsibleButton*>(ancestor)||
        qobject_cast<ctkCollapsibleButton*>(ancestor))
      {
      backgroundColor = backgroundColor.darker(108);
      backgroundColor = QColor::fromHsvF(backgroundColor.hueF(),
                                         backgroundColor.saturationF()*1.2,
                                         backgroundColor.valueF());
      }
    }
  QPalette newPalette = this->palette();
  newPalette.setColor(QPalette::Window, backgroundColor);
  this->setPalette(newPalette);
}
