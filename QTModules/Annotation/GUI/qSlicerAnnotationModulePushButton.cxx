#include "qSlicerAnnotationModulePushButton.h"

qSlicerAnnotationModulePushButton::qSlicerAnnotationModulePushButton(int index, QWidget *parent)
{

  Q_UNUSED(parent);

  m_index = index;

  this->connect(this, SIGNAL(clicked()), this, SLOT(mybuttonClicked()));
}

void qSlicerAnnotationModulePushButton::mybuttonClicked()
{
  m_text = this->text();
  //emit buttonClickedWithText( m_text );
  emit buttonClickedWithIndex(m_index);

}

void qSlicerAnnotationModulePushButton::setButtonIndex(int index)
{
  m_index = index;
}

