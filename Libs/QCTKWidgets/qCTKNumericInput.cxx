
#include "qCTKNumericInput.h"

#include <QDoubleValidator>
#include <QDebug>

//-----------------------------------------------------------------------------
class qCTKNumericInput::qInternal
{
public:
  qInternal()
    {
    this->Text = "0";
    }
  QString Text; 
};

// --------------------------------------------------------------------------
qCTKNumericInput::qCTKNumericInput(QWidget* parent) : Superclass(parent)
{
  this->Internal = new qInternal; 
  
  this->setValidator(new QDoubleValidator(this));
  this->setDecimals(2); 
  this->setValue(0);
  //this->setText(this->Internal->Text); 
    
//   this->connect(this, SIGNAL(textChanged(const QString&)), 
//     SLOT(onTextChanged(const QString&))); 
}

// --------------------------------------------------------------------------
qCTKNumericInput::~qCTKNumericInput()
{
  delete this->Internal; 
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
void qCTKNumericInput::setValue(double value)
{
  this->Superclass::setText(QString::number(value, 'g', this->decimals()+1));
}

// --------------------------------------------------------------------------
QDoubleValidator* qCTKNumericInput::doubleValidator()
{
  Q_ASSERT(this->validator());
  QDoubleValidator* doubleValidator = qobject_cast<QDoubleValidator*>(const_cast<QValidator*>(this->validator()));
  Q_ASSERT(doubleValidator);
  return doubleValidator; 
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

