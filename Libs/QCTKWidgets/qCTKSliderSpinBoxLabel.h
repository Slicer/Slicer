#ifndef __qCTKSliderSpinBoxLabel_h
#define __qCTKSliderSpinBoxLabel_h

#include <QSlider>

#include "qCTKWidgetsWin32Header.h"

class QCTK_WIDGETS_EXPORT qCTKSliderSpinBoxLabel : public QWidget
{
  Q_OBJECT
  Q_PROPERTY(bool LabelTextVisible READ isLabelTextVisible WRITE setLabelTextVisible)
  
public:
  // Superclass typedef
  typedef QWidget Superclass;
  
  // Constructors
  qCTKSliderSpinBoxLabel(QWidget* parent = 0);
  virtual ~qCTKSliderSpinBoxLabel();

  // Description:
  // Set built-in label text visibility
  void setLabelTextVisible(bool visible);
  bool isLabelTextVisible(); 
  
  // Description:
  // Set/Get label text
  QString labelText(); 
  void setLabelText(const QString & label); 
   
  // Description:
  // Set/Get range
  void setRange(double min, double max); 
  double minimum();
  double maximum();
  
  // Description:
  // Set/Get slider position
  double sliderPosition();
  void setSliderPosition(double position); 
  double previousSliderPosition(); 
  
  // Description:
  // Set/Get value
  double value(); 
  void setValue(double value);
  
  // Description:
  // Set/Get single step
  double singleStep();
  void setSingleStep(double step); 
  
  // Description:
  // Set/Get tick interval 
  double tickInterval(); 
  void setTickInterval(double ti);

public slots:
  // Description:
  // Reset the slider and spinbox to zero (value and position)
  void reset(); 
  
signals:
  void sliderMoved(double position); 
  void valueChanged(double value); 

protected slots:
  void onSpinBoxValueChanged(double value); 

private:
  class qInternal; 
  qInternal* Internal; 

}; 

#endif
