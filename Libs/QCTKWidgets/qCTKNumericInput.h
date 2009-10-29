#ifndef __qCTKNumericInput_h
#define __qCTKNumericInput_h

#include <QLineEdit>

#include "qCTKWidgetsWin32Header.h"

class QDoubleValidator; 

class QCTK_WIDGETS_EXPORT qCTKNumericInput : public QLineEdit
{
  Q_OBJECT
  Q_PROPERTY(double Value READ value WRITE setValue)
  Q_PROPERTY(int Decimals READ decimals WRITE setDecimals)
  
public:
  // Superclass typedef
  typedef QLineEdit Superclass;
  
  // Constructors
  qCTKNumericInput(QWidget* parent = 0);
  virtual ~qCTKNumericInput();
  
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

protected slots:
  // Description:
  // Ensuire the text is valid even if the value is set from the designer
//   void onTextChanged(const QString & text); 
  
private:
  class qInternal; 
  qInternal* Internal; 

}; 

#endif
