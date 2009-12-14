#ifndef __qCTKCoordinatesWidget_h
#define __qCTKCoordinatesWidget_h

#include "qCTKWidgetsExport.h"

#include <QWidget>

class QCTK_WIDGETS_EXPORT qCTKCoordinatesWidget : public QWidget
{
  Q_OBJECT 
  Q_PROPERTY(int Dimension READ dimension WRITE setDimension)
  Q_PROPERTY(double minimum READ minimum WRITE setMinimum)
  Q_PROPERTY(double maximum READ maximum WRITE setMaximum)
  Q_PROPERTY(double singleStep  READ singleStep WRITE setSingleStep STORED false)
  Q_PROPERTY(QString coordinates READ coordinatesAsString WRITE setCoordinatesAsString)

    
public:
  explicit qCTKCoordinatesWidget(QWidget* parent = 0);
  virtual ~qCTKCoordinatesWidget();

  // Description:
  // Set/Get the dimension of the point (3D by default)
  void setDimension(int dim);
  int dimension() const;

  // Description:
  // Set/Get the single step of the QDoubleSpinBoxes 
  void setSingleStep(double step);
  double singleStep() const;

  // Description:
  // Set/Get the minimum value of the QDoubleSpinBoxes 
  void setMinimum(double minimum);
  double minimum() const;

  // Description:
  // Set/Get the maximum value of the QDoubleSpinBoxes 
  void setMaximum(double minimum);
  double maximum() const;

  // Description:
  // Set/Get the coordinates. Use commas between numbers
  void setCoordinatesAsString(QString pos);
  QString coordinatesAsString()const;

  // Description:
  // Set/Get the coordinates
  void setCoordinates(double* pos);
  double* coordinates()const;

signals:
  void valueChanged(double* pos);

protected slots:
  void coordinateChanged(double);
  void coordinatesChanged();

protected:
  void AddSpinBox();

  double  Minimum;
  double  Maximum;
  double  SingleStep;
  int     Dimension;
  double* Coordinates;
};

#endif
