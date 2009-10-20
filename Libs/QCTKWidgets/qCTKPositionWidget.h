#ifndef __qCTKPositionWidget_h
#define __qCTKPositionWidget_h

#include "qCTKWidgetsWin32Header.h"

#include <QWidget>

class QCTK_WIDGETS_EXPORT qCTKPositionWidget : public QWidget
{
  Q_OBJECT 
  Q_PROPERTY(int Dimension READ dimension WRITE setDimension)
  Q_PROPERTY(double singleStep  READ singleStep WRITE setSingleStep STORED false)
  Q_PROPERTY(QString position READ positionAsString WRITE setPositionAsString)
    
public:
  qCTKPositionWidget(QWidget* parent = 0);
  virtual ~qCTKPositionWidget();

  // Description:
  // Set/Get the dimension of the point (3D by default)
  void setDimension(int dim);
  int dimension() const;

  // Description:
  // Set/Get the single step of the QDoubleSpinBoxes 
  void setSingleStep(double step);
  double singleStep() const;

  // Description:
  // Set/Get the position. Use commas between numbers
  void setPositionAsString(QString pos);
  QString positionAsString()const;

  // Description:
  // Set/Get the position
  void setPosition(double* pos);
  double* position()const;

signals:
  void valueChanged(double* pos);

protected slots:
  void coordinateChanged(double);
  void coordinatesChanged();

protected:
  int Dimension;
  double* Position;
};

#endif
