/*=========================================================================

  Library:   qCTK

  Copyright (c) Kitware Inc. 
  All rights reserved.
  Distributed under a BSD License. See LICENSE.txt file.

  This software is distributed "AS IS" WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the above copyright notice for more information.

=========================================================================*/


#include "qCTKNumericInput.h"

#include <QDoubleValidator>
#include <QDebug>

//-----------------------------------------------------------------------------
class qCTKNumericInputPrivate: public qCTKPrivate<qCTKNumericInput>
{
public:
  qCTKNumericInputPrivate()
    {
    //this->Text = "0";
    }
  //QString Text;
};

// --------------------------------------------------------------------------
qCTKNumericInput::qCTKNumericInput(QWidget* _parent) : Superclass(_parent)
{
  QCTK_INIT_PRIVATE(qCTKNumericInput);

  this->setValidator(new QDoubleValidator(this));
  this->setDecimals(2);
  this->setValue(0);
  //this->setText(this->Internal->Text);

//   this->connect(this, SIGNAL(textChanged(const QString&)),
//     SLOT(onTextChanged(const QString&)));

  this->connect(this, SIGNAL(returnPressed()), SLOT(onReturnPressed()));
}

// // --------------------------------------------------------------------------
// void qCTKNumericInput::onTextChanged(const QString & text)
// {
//   int pos = 0;
//   QString newText = this->Internal->Text;
//   if (this->doubleValidator()->validate(
//     const_cast<QString&>(text), pos) == QValidator::Acceptable)
//     {
//     newText = text;
//     this->Internal->Text = text;
//     }
//   this->blockSignals(true);
//   this->Superclass::setText(newText);
//   this->blockSignals(false);
// }

// --------------------------------------------------------------------------
double qCTKNumericInput::value()
{
  return this->text().toDouble();
}

// --------------------------------------------------------------------------
void qCTKNumericInput::setValue(double _value)
{
  this->Superclass::setText(QString::number(_value, 'g', this->decimals()+1));
}

// --------------------------------------------------------------------------
QDoubleValidator* qCTKNumericInput::doubleValidator()
{
  Q_ASSERT(this->validator());
  QDoubleValidator* _doubleValidator =
    qobject_cast<QDoubleValidator*>(const_cast<QValidator*>(this->validator()));
  Q_ASSERT(_doubleValidator);
  return _doubleValidator;
}

// --------------------------------------------------------------------------
int qCTKNumericInput::decimals()
{
  return this->doubleValidator()->decimals();
}

// --------------------------------------------------------------------------
void qCTKNumericInput::setDecimals(int count)
{
  this->doubleValidator()->setDecimals(count);
}

// --------------------------------------------------------------------------
void qCTKNumericInput::onReturnPressed()
{
  emit(this->valueEdited(this->value()));
}

