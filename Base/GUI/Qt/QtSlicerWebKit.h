
#ifndef __QtSlicerWebKit_h
#define __QtSlicerWebKit_h

#include <QWidget>
#include "vtkSlicerBaseGUIQtWin32Header.h"
  
class QSlicerWebKit;
class QLineEdit;
class QGroupBox;
class QVBoxLayout;
class QWebView;
class QSlicerFactory;
class vtkKWApplication;

/// a widget with a tied slider and line edit for editing a double property
class VTK_SLICER_BASE_GUI_QT_EXPORT QtSlicerWebKit : public QWidget
{
  Q_OBJECT
  //Q_PROPERTY(double value READ value WRITE setValue USER true)
  //Q_PROPERTY(double minimum READ minimum WRITE setMinimum)
  //Q_PROPERTY(double maximum READ maximum WRITE setMaximum)
  //Q_PROPERTY(bool   strictRange READ strictRange WRITE setStrictRange)
  //Q_PROPERTY(int resolution READ resolution WRITE setResolution)
public:
  /// constructor requires the proxy, property
  QtSlicerWebKit(QWidget* parent = NULL, vtkKWApplication *kwapp = 0);
  ~QtSlicerWebKit();

  /// get the current url
  const char *url() const;
  
  // get the min range value
  double minimum() const;
  // get the max range value
  double maximum() const;
 
  // returns whether the line edit is also limited 
  bool strictRange() const;

  // returns the resolution.
  int resolution() const;
  
signals:
  /// signal the url was edited
  //void urlEdited(const QString& url);

public slots:
  /// set the value
  void setURL(const char *url);

private:
  QGroupBox* group;
  QVBoxLayout* boxLayout;
  QWebView* webView;
  QSlicerFactory* factory;
  QLineEdit* lineEdit;

  vtkKWApplication *kwapp;
};

#endif
