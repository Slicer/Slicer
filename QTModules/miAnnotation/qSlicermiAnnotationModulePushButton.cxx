#include "qSlicermiAnnotationModulePushButton.h"


qSlicermiAnnotationModulePushButton::qSlicermiAnnotationModulePushButton(int index, QWidget *parent)
{
    m_index = index;

    this->connect(this, SIGNAL( clicked() ), this, SLOT(mybuttonClicked()) );
}

void qSlicermiAnnotationModulePushButton::mybuttonClicked()
{
    m_text = this->text();
    //emit buttonClickedWithText( m_text );
    emit buttonClickedWithIndex( m_index );

}

void qSlicermiAnnotationModulePushButton::setButtonIndex(int index)
{
    m_index = index;
}

