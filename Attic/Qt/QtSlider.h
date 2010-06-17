
#ifndef __QtSlider_h
#define __QtSlider_h

#include <QWidget>
#include "vtkSlicerBaseGUIQtWin32Header.h"
  
class QSlider;
class QLineEdit;

/// a widget with a tied slider and line edit for editing a double property
class VTK_SLICER_BASE_GUI_QT_EXPORT QtSlider : public QWidget
{
  Q_OBJECT
  Q_PROPERTY(double value READ value WRITE setValue USER true)
  Q_PROPERTY(double minimum READ minimum WRITE setMinimum)
  Q_PROPERTY(double maximum READ maximum WRITE setMaximum)
  Q_PROPERTY(bool   strictRange READ strictRange WRITE setStrictRange)
  Q_PROPERTY(int resolution READ resolution WRITE setResolution)
public:
  /// constructor requires the proxy, property
  QtSlider(QWidget* parent = NULL);
  ~QtSlider();

  /// get the value
  double value() const;
  
  // get the min range value
  double minimum() const;
  // get the max range value
  double maximum() const;
 
  // returns whether the line edit is also limited 
  bool strictRange() const;

  // returns the resolution.
  int resolution() const;
  
signals:
  /// signal the value changed
  void valueChanged(double);

  /// signal the value was edited
  /// this means the user is done changing text
  /// or the slider was moved
  void valueEdited(double);

public slots:
  /// set the value
  void setValue(double);

  // set the min range value
  void setMinimum(double);
  // set the max range value
  void setMaximum(double);

  // set the range on both the slider and line edit's validator
  // whereas other methods just do it on the slider
  void setStrictRange(bool);

  // set the resolution.
  void setResolution(int);

private slots:
  void sliderChanged(int);
  void textChanged(const QString&);
  void editingFinished();
  void updateValidator();
  void updateSlider();

private:
  int Resolution;
  double Value;
  double Minimum;
  double Maximum;
  QSlider* Slider;
  QLineEdit* LineEdit;
  bool BlockUpdate;
  bool StrictRange;
};

#endif
