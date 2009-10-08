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
  
  virtual QSize minimumSizeHint() const;
  virtual QSize sizeHint() const;
                                
 public slots:
  
  // Description:
  // Reset to zero
  void reset(); 
  
 private:
  class qInternal; 
  qInternal* Internal; 
};

#endif
