#ifndef __qCTKNumericInput_h
#define __qCTKNumericInput_h

// qCTK includes
#include "qCTKPimpl.h"

// QT includes
#include <QLineEdit>

#include "qCTKWidgetsExport.h"

class QDoubleValidator;
class qCTKNumericInputPrivate;

class QCTK_WIDGETS_EXPORT qCTKNumericInput : public QLineEdit
{
  Q_OBJECT
  Q_PROPERTY(double Value READ value WRITE setValue)
  Q_PROPERTY(int Decimals READ decimals WRITE setDecimals)

public:
  // Superclass typedef
  typedef QLineEdit Superclass;

  // Constructors
  explicit qCTKNumericInput(QWidget* parent = 0);
  virtual ~qCTKNumericInput(){}

  // Description:
  // Set/Get value
  double value();
  void setValue(double value);

  // Description:
  // Set/Get number of decimals
  int decimals();
  void setDecimals(int count);

  // Description:
  // Return the double validator associated with the LineEdit
  QDoubleValidator* doubleValidator();

signals:
  // Description:
  // This signal is emitted whenever the value is edited, The value argument is the new value.
  // This signal is not emitted when the value is changed programmatically, for example,
  // by calling setValue().
  void valueEdited(double value);

protected slots:
  // Description:
  // Make sure that the text is valid even if the value is set from the designer
//   void onTextChanged(const QString & text);

  // Description:
  // Triggered after the user hit 'Enter' and the input is a valid numeric data
  // Responsible to emit the signal 'valueEdited'.
  void onReturnPressed();

private:
  QCTK_DECLARE_PRIVATE(qCTKNumericInput);

};

#endif
