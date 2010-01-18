/*=========================================================================

  Library:   qCTK

  Copyright (c) Kitware Inc. 
  All rights reserved.
  Distributed under a BSD License. See LICENSE.txt file.

  This software is distributed "AS IS" WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the above copyright notice for more information.

=========================================================================*/

#ifndef __qCTKMatrixWidget_h
#define __qCTKMatrixWidget_h

// qCTK includes
#include "qCTKPimpl.h"

// QT includes
#include <QTableWidget>

#include "qCTKWidgetsExport.h"

class qCTKMatrixWidgetPrivate;

class QCTK_WIDGETS_EXPORT qCTKMatrixWidget : public QTableWidget
{
  Q_OBJECT

public:
  // Superclass typedef
  typedef QTableWidget Superclass;

  // Constructors
  explicit qCTKMatrixWidget(QWidget* parent = 0);
  virtual ~qCTKMatrixWidget(){}

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
  QCTK_DECLARE_PRIVATE(qCTKMatrixWidget);
};

#endif
