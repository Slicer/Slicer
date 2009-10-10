#ifndef __qSlicerKwUIWrapper_h
#define __qSlicerKwUIWrapper_h 

#include <QWidget>

#include "qSlicerBaseGUI2Win32Header.h"

class vtkMRMLNode; 

class Q_SLICER_BASE_GUI2_EXPORT qSlicerKwUIWrapper : public QWidget
{ 
  Q_OBJECT

public:

  typedef QWidget Superclass;
  qSlicerKwUIWrapper(QWidget *parent=0);
  virtual ~qSlicerKwUIWrapper(); 
  
  virtual void dumpObjectInfo();


private:
  class qInternal;
  qInternal* Internal;
};

#endif
