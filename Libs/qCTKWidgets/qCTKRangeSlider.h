/*=========================================================================

  Library:   qCTK

  Copyright (c) Kitware Inc. 
  All rights reserved.
  Distributed under a BSD License. See LICENSE.txt file.

  This software is distributed "AS IS" WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the above copyright notice for more information.

=========================================================================*/

#ifndef __qCTKRangeSlider_h
#define __qCTKRangeSlider_h

#include <QSlider>

#include "qCTKPimpl.h"
#include "qCTKWidgetsExport.h"

class QStylePainter;
class qCTKRangeSliderPrivate;

class QCTK_WIDGETS_EXPORT qCTKRangeSlider : public QSlider
{
    Q_OBJECT
    Q_PROPERTY(int RangeMinimumValuePosition READ rangeMinimumValuePosition WRITE setRangeMinimumPosition)
    Q_PROPERTY(int RangeMaximumValuePosition READ rangeMaximumValuePosition WRITE setRangeMaximumPosition)
    Q_PROPERTY(int RangeMinimum READ rangeMinimum WRITE setRangeMinimum)
    Q_PROPERTY(int RangeMaximum READ rangeMaximum WRITE setRangeMaximum)

public:
    // Superclass typedef
    typedef QSlider Superclass;

    // Constructors
    explicit qCTKRangeSlider( Qt::Orientation o, QWidget* par= 0 );
    explicit qCTKRangeSlider( QWidget* par = 0 );
    virtual ~qCTKRangeSlider() {};

    int rangeMinimum() const;
    int rangeMaximum() const;

    int rangeMinimumValuePosition() const;
    int rangeMaximumValuePosition() const;

signals:
    void rangeValuesChanged(int rangeMinimum, int rangeMaximum);
    void rangeMinimumChanged(int rangeMinimum);
    void rangeMaximumChanged(int rangeMaximum);

    void rangeMinimumValuePositionChanged(int rangeMinimum);
    void rangeMaximumValuePositionChanged(int rangeMaximum);

public slots:
    void setRangeMinimum(int rangeMinimum);
    void setRangeMaximum(int rangeMaximum);
    void setRangeBounds(int rangeMinimum, int rangeMaximum);

    void setRangeMinimumPosition(int rangeMinimum);
    void setRangeMaximumPosition(int rangeMaximum);

protected:

    // Description:
    // Standard Qt UI events
    virtual void keyPressEvent( QKeyEvent* ) {}
    virtual void mousePressEvent(QMouseEvent* ev);
    virtual void mouseMoveEvent(QMouseEvent* ev);
    virtual void mouseReleaseEvent(QMouseEvent* ev);

    // Description:
    // Rendering is done here.
    virtual void paintEvent(QPaintEvent* ev);
protected slots:
   void updateRangeBounds();

private:
    QCTK_DECLARE_PRIVATE(qCTKRangeSlider);
};

#endif

