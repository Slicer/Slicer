#ifndef __qCTKSliderSpinBoxLabel_h
#define __qCTKSliderSpinBoxLabel_h

// qCTK includes
#include "qCTKPimpl.h"

// QT includes
#include <QSlider>

#include "qCTKWidgetsWin32Header.h"

class qCTKSliderSpinBoxLabelPrivate;

class QCTK_WIDGETS_EXPORT qCTKSliderSpinBoxLabel : public QWidget
{
  Q_OBJECT
  Q_PROPERTY(bool LabelTextVisible READ isLabelTextVisible WRITE setLabelTextVisible)
  Q_PROPERTY(double minimum READ minimum WRITE setMinimum)
  Q_PROPERTY(double maximum READ maximum WRITE setMaximum)

public:
  // Superclass typedef
  typedef QWidget Superclass;

  // Constructors
  qCTKSliderSpinBoxLabel(QWidget* parent = 0);
  virtual ~qCTKSliderSpinBoxLabel(){}

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
  double minimum()const;
  double maximum()const;
  void setMinimum(double minimum);
  void setMaximum(double maximum);
  // Description
  // Utility function that set the min/max in once
  void setRange(double min, double max);

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
  QCTK_DECLARE_PRIVATE(qCTKSliderSpinBoxLabel);

};

#endif
