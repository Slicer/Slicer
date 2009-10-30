#ifndef __qSlicerKwUIWrapper_h
#define __qSlicerKwUIWrapper_h 

#include <QWidget>

#include "qSlicerBaseGUIQTWin32Header.h"

class vtkMRMLNode; 

class Q_SLICER_BASE_GUIQT_EXPORT qSlicerKwUIWrapper : public QWidget
{ 
  Q_OBJECT

public:

  typedef QWidget Superclass;
  qSlicerKwUIWrapper(QWidget *parent=0);
  virtual ~qSlicerKwUIWrapper(); 
  
  virtual void printAdditionalInfo();


private:
  class qInternal;
  qInternal* Internal;
};

#endif
