#ifndef __qCTKMatrixWidget_h
#define __qCTKMatrixWidget_h

#include <QTableWidget>

#include "qCTKWidgetsWin32Header.h"

class QCTK_WIDGETS_EXPORT qCTKMatrixWidget : public QTableWidget
{
  Q_OBJECT
    
public:
  // Superclass typedef
  typedef QTableWidget Superclass;
  
  // Constructors
  qCTKMatrixWidget(QWidget* parent = 0);
  virtual ~qCTKMatrixWidget();
  
  // Description:
  // Set / Get values 
  double value(int i, int j);
  void setValue(int i, int j, double value); 
  void setVector(const QVector<double> & vector); 
  
  // Description:
  // Overloaded - See QWidget
  virtual QSize minimumSizeHint () const; 
  virtual QSize sizeHint () const; 
  
  
public slots:
  
  // Description:
  // Reset to zero
  void reset(); 
  
protected slots:
  // Description:
  // Adjust columns/rows size according to width/height
  void adjustRowsColumnsSize(int width, int height);

protected:
  // Description:
  virtual void resizeEvent(QResizeEvent * event); 

private:
  class qInternal; 
  qInternal* Internal; 
};

#endif
