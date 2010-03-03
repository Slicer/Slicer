#include "qCTKRangeSlider.h"

#include <QMouseEvent>
#include <QKeyEvent>
#include <QStyleOptionSlider>
#include <QApplication>
#include <QStylePainter>
#include <QStyle>
//#include "rvCommon.h"


class qCTKRangeSliderPrivate:public qCTKPrivate<qCTKRangeSlider>
{
public:
  qCTKRangeSliderPrivate();

  // Description:
  // Copied verbatim from QSliderPrivate class (see QSlider.cpp)
  int pixelPosToRangeValue(int pos) const;

  // Description:
  // Draw the bottom and top sliders.
  void drawMinimumSlider( QStylePainter* painter ) const;
  void drawMaximumSlider( QStylePainter* painter ) const;
    
  // Description:
  // End points of the range on the Model
  int m_RangeMaximum;
  int m_RangeMinimum;

  // Description:
  // End points of the range on the GUI. This is synced with the model.
  int m_RangeMaximumValue;
  int m_RangeMinimumValue;

  // Description:
  // Controls selected ?
  QStyle::SubControl m_RangeMinimumSliderSelected;
  QStyle::SubControl m_RangeMaximumSliderSelected;

  // Description:
  // See QSliderPrivate::clickOffset. 
  // Overrides this ivar
  int m_SubclassClickOffset;
    
  // Description:
  // See QSliderPrivate::position
  // Overrides this ivar.
  int m_SubclassPosition;

  // Description:
  // Boolean indicates the selected handle
  //   True for the minimum range handle, false for the maximum range handle
  bool m_SelectedSlider;
};

// --------------------------------------------------------------------------
qCTKRangeSliderPrivate::qCTKRangeSliderPrivate()
{
  this->m_RangeMinimum = 10;
  this->m_RangeMaximum = 90;
  this->m_RangeMinimumValue = 10;
  this->m_RangeMaximumValue = 90;
  this->m_RangeMinimumSliderSelected = QStyle::SC_None;
  this->m_RangeMaximumSliderSelected = QStyle::SC_None;
  this->m_SubclassClickOffset = 0;
  this->m_SubclassPosition = 0;
}

// --------------------------------------------------------------------------
// Copied verbatim from QSliderPrivate::pixelPosToRangeValue. See QSlider.cpp
//
int qCTKRangeSliderPrivate::pixelPosToRangeValue( int pos ) const
{
  QCTK_P(const qCTKRangeSlider);
  QStyleOptionSlider option;
  p->initStyleOption( &option );

  QRect gr = p->style()->subControlRect( QStyle::CC_Slider, 
                                            &option, 
                                            QStyle::SC_SliderGroove, 
                                            p );
  QRect sr = p->style()->subControlRect( QStyle::CC_Slider, 
                                            &option, 
                                            QStyle::SC_SliderHandle, 
                                            p );

  int sliderLength = sr.width();
  int sliderMin = gr.x();
  int sliderMax = gr.right() - sliderLength + 1;

  return QStyle::sliderValueFromPosition( p->minimum(), 
                                          p->maximum(), 
                                          pos - sliderMin,
                                          sliderMax - sliderMin, 
                                          option.upsideDown );
}

//---------------------------------------------------------------------------
// Draw slider at the bottom end of the range
void qCTKRangeSliderPrivate::drawMinimumSlider( QStylePainter* painter ) const
{
  QCTK_P(const qCTKRangeSlider);
  QStyleOptionSlider option;
  p->initStyleOption( &option );

  option.subControls = QStyle::SC_SliderHandle;
  option.sliderValue = m_RangeMinimum;
  option.sliderPosition = m_RangeMinimumValue;
  if (m_RangeMinimumSliderSelected == QStyle::SC_SliderHandle)
    {
    option.activeSubControls = m_RangeMinimumSliderSelected;
    option.state |= QStyle::State_Sunken;
    }

  painter->drawComplexControl(QStyle::CC_Slider, option);
}

//---------------------------------------------------------------------------
// Draw slider at the top end of the range
void qCTKRangeSliderPrivate::drawMaximumSlider( QStylePainter* painter ) const
{
  QCTK_P(const qCTKRangeSlider);
  QStyleOptionSlider option;
  p->Superclass::initStyleOption( &option );

  option.subControls = QStyle::SC_SliderHandle;
  option.sliderValue = m_RangeMaximum;
  option.sliderPosition = m_RangeMaximumValue;
  if (m_RangeMaximumSliderSelected == QStyle::SC_SliderHandle)
    {
    option.activeSubControls = m_RangeMaximumSliderSelected;
    option.state |= QStyle::State_Sunken;
    }

  painter->drawComplexControl(QStyle::CC_Slider, option);
}

// --------------------------------------------------------------------------
qCTKRangeSlider::qCTKRangeSlider(QWidget* parent) 
  : QSlider(parent)
{
  QCTK_INIT_PRIVATE(qCTKRangeSlider);
  connect(this, SIGNAL(rangeValuesChanged(int, int)), 
          this, SLOT(updateRangeBounds(int, int)));
}

// --------------------------------------------------------------------------
qCTKRangeSlider::qCTKRangeSlider( Qt::Orientation o, 
                                  QWidget* parentObject )
  :QSlider(o, parentObject)
{
  QCTK_INIT_PRIVATE(qCTKRangeSlider);
  connect(this, SIGNAL(rangeValuesChanged(int, int)), 
          this, SLOT(updateRangeBounds()));
}

// --------------------------------------------------------------------------
void qCTKRangeSlider::updateRangeBounds()
{
  QCTK_D(const qCTKRangeSlider);
  this->setRangeBounds(d->m_RangeMinimum, d->m_RangeMaximum);
}

// --------------------------------------------------------------------------
int qCTKRangeSlider::rangeMinimum() const
{
  QCTK_D(const qCTKRangeSlider);
  return qMin( d->m_RangeMinimum, d->m_RangeMaximum );
}

// --------------------------------------------------------------------------
void qCTKRangeSlider::setRangeMinimum( int rm )
{
  QCTK_D(qCTKRangeSlider);
  this->setRangeBounds( rm, d->m_RangeMaximum );
}

// --------------------------------------------------------------------------
int qCTKRangeSlider::rangeMaximum() const
{
  QCTK_D(const qCTKRangeSlider);
  return qMax( d->m_RangeMinimum, d->m_RangeMaximum );
}

// --------------------------------------------------------------------------
void qCTKRangeSlider::setRangeMaximum( int rm )
{
  QCTK_D(qCTKRangeSlider);
  this->setRangeBounds( d->m_RangeMinimum, rm );
}

// --------------------------------------------------------------------------
void qCTKRangeSlider::setRangeBounds(int l, int u)
{
  QCTK_D(qCTKRangeSlider);
  const int rangeMinimum = qBound(minimum(), qMin(l, u), maximum());
  const int rangeMaximum = qBound(minimum(), qMax(l, u), maximum());

  if (rangeMaximum != d->m_RangeMaximum)
    {
    d->m_RangeMaximum = d->m_RangeMaximumValue = rangeMaximum;

    emit rangeMaximumChanged(rangeMaximum);
    emit rangeValuesChanged(d->m_RangeMinimum, d->m_RangeMaximum);
    this->update();
    }

  if (rangeMinimum != d->m_RangeMinimum)
    {
    d->m_RangeMinimum = d->m_RangeMinimumValue = rangeMinimum;
    
    emit rangeMinimumChanged(rangeMinimum);
    emit rangeValuesChanged(d->m_RangeMinimum, d->m_RangeMaximum);
    this->update();
    }
}

// --------------------------------------------------------------------------
int qCTKRangeSlider::rangeMinimumValuePosition() const
{
  QCTK_D(const qCTKRangeSlider);
  return d->m_RangeMinimumValue;
}

// --------------------------------------------------------------------------
int qCTKRangeSlider::rangeMaximumValuePosition() const
{
  QCTK_D(const qCTKRangeSlider);
  return d->m_RangeMaximumValue;
}

// --------------------------------------------------------------------------
void qCTKRangeSlider::setRangeMinimumPosition(int l)
{
  QCTK_D(qCTKRangeSlider);
  if (d->m_RangeMinimumValue == l)
    {
    return;
    }
  d->m_RangeMinimumValue = l;

  if (this->isSliderDown())
    {
    emit rangeMinimumValuePositionChanged(l);
    }

  this->setRangeBounds(l, d->m_RangeMaximum);
}

// --------------------------------------------------------------------------
void qCTKRangeSlider::setRangeMaximumPosition(int u)
{
  QCTK_D(qCTKRangeSlider);
  if (d->m_RangeMaximumValue == u)
    {
    return;
    }
  d->m_RangeMaximumValue = u;
    
  if (this->isSliderDown())
    {
    emit rangeMaximumValuePositionChanged(u);
    }

  this->setRangeBounds(d->m_RangeMinimum, u);
}

// --------------------------------------------------------------------------
// Render
void qCTKRangeSlider::paintEvent( QPaintEvent* )
{
  QCTK_D(qCTKRangeSlider);
  QStyleOptionSlider option;
  this->initStyleOption(&option);

  QStylePainter painter(this);
  option.subControls = QStyle::SC_SliderGroove;
  painter.drawComplexControl(QStyle::CC_Slider, option);

  option.sliderPosition = d->m_RangeMinimumValue;
  const QRect lr = style()->subControlRect( QStyle::CC_Slider, 
                                            &option, 
                                            QStyle::SC_SliderHandle, 
                                            this);
  option.sliderPosition = d->m_RangeMaximumValue;
  const QRect ur = style()->subControlRect( QStyle::CC_Slider, 
                                            &option, 
                                            QStyle::SC_SliderHandle, 
                                            this);

  QRect sr = style()->subControlRect( QStyle::CC_Slider, 
                                      &option, 
                                      QStyle::SC_SliderGroove, 
                                      this);

  QRect rangeBox = QRect( 
      QPoint( qMin( lr.center().x(), ur.center().x() ), sr.center().y() - 2), 
      QPoint(qMax( lr.center().x(), ur.center().x() ), sr.center().y() + 1));


  // -----------------------------
  // Render the range
  //
  QRect groove = this->style()->subControlRect( QStyle::CC_Slider, 
                                                &option, 
                                                QStyle::SC_SliderGroove, 
                                                this );
  groove.adjust(0, 0, -1, 0);

  painter.setPen( QPen( this->palette().color(QPalette::Dark).light(90), 0));

  // Create default colors based on the transfer function.
  //
  QColor highlight = this->palette().color(QPalette::Highlight);
  QLinearGradient gradient( groove.center().x(), groove.top(),
                            groove.center().x(), groove.bottom());

  // TODO: Set this based on the supplied transfer function
  QColor l = Qt::darkGray;
  QColor u = Qt::black;

  gradient.setColorAt(0, l);
  gradient.setColorAt(1, u);

  painter.setBrush(gradient);
  painter.setPen(QPen(highlight.dark(140), 0));

  painter.drawRect( rangeBox.intersected(groove) );

  //  -----------------------------------
  // Render the sliders
  //
  if (d->m_SelectedSlider)
    {
    d->drawMaximumSlider( &painter );
    d->drawMinimumSlider( &painter );
    }
  else
    {
    d->drawMinimumSlider( &painter );
    d->drawMaximumSlider( &painter );
    }
}

// --------------------------------------------------------------------------
// Standard Qt UI events
void qCTKRangeSlider::mousePressEvent(QMouseEvent* mouseEvent)
{
  QCTK_D(qCTKRangeSlider);
  if (minimum() == maximum() || (mouseEvent->buttons() ^ mouseEvent->button()))
    {
    mouseEvent->ignore();
    return;
    }
  
  QStyleOptionSlider option;
  this->initStyleOption( &option );

  // Check if the first slider is pressed
  
  if (!this->isSliderDown())
    {
    option.sliderPosition = d->m_RangeMinimumValue;
    option.sliderValue    = d->m_RangeMinimum;

    QStyle::SubControl& control = d->m_RangeMinimumSliderSelected;
    
    control = this->style()->hitTestComplexControl( QStyle::CC_Slider, 
                                                    &option, 
                                                    mouseEvent->pos(), 
                                                    this);

    if (control == QStyle::SC_SliderHandle)
      {
      d->m_SelectedSlider = true;
      d->m_SubclassPosition = d->m_RangeMinimum;

      const QRect sr = this->style()->subControlRect( QStyle::CC_Slider, 
                                                      &option, 
                                                      QStyle::SC_SliderHandle, 
                                                      this);
      
      d->m_SubclassClickOffset = mouseEvent->pos().x() - sr.topLeft().x();

      this->setSliderDown(true);

      if (control != d->m_RangeMinimumSliderSelected)
        {
        this->update(sr);
        }
      }
    }


  // Check if the other slider is pressed

  if (!this->isSliderDown())
    {
    option.sliderPosition = d->m_RangeMaximumValue;
    option.sliderValue    = d->m_RangeMaximum;

    QStyle::SubControl& control = d->m_RangeMaximumSliderSelected;
    
    control = this->style()->hitTestComplexControl( QStyle::CC_Slider, 
                                                    &option, 
                                                    mouseEvent->pos(), 
                                                    this);

    if (control == QStyle::SC_SliderHandle)
      {
      d->m_SelectedSlider = false;
      d->m_SubclassPosition = d->m_RangeMaximum;

      const QRect sr = this->style()->subControlRect( QStyle::CC_Slider, 
                                                      &option, 
                                                      QStyle::SC_SliderHandle, 
                                                      this);

      d->m_SubclassClickOffset = mouseEvent->pos().x() - sr.topLeft().x();

      this->setSliderDown(true);

      if (d->m_RangeMaximumSliderSelected != control)
        {
        this->update(sr);
        }
      }
    }


  // Accept the mouseEvent
  mouseEvent->accept();
}

// --------------------------------------------------------------------------
// Standard Qt UI events
void qCTKRangeSlider::mouseMoveEvent(QMouseEvent* mouseEvent)
{
  QCTK_D(qCTKRangeSlider);
  if (d->m_RangeMinimumSliderSelected == QStyle::SC_SliderHandle ||
      d->m_RangeMaximumSliderSelected == QStyle::SC_SliderHandle)
    {

    QStyleOptionSlider option;
    this->initStyleOption(&option);

    const int m = style()->pixelMetric( QStyle::PM_MaximumDragDistance, &option, this );

    int newPosition = d->pixelPosToRangeValue(
        mouseEvent->pos().x() - d->m_SubclassClickOffset);

    if (m >= 0)
      {
      const QRect r = rect().adjusted(-m, -m, m, m);
      if (!r.contains(mouseEvent->pos()))
        {
        newPosition = d->m_SubclassPosition;
        }
      }

    if (d->m_RangeMinimumSliderSelected == QStyle::SC_SliderHandle)
      {
      newPosition = qMin(newPosition, this->rangeMaximum());
      this->setRangeMinimumPosition(newPosition);
      }
    else if (d->m_RangeMaximumSliderSelected == QStyle::SC_SliderHandle)
      {
      newPosition = qMax(newPosition, this->rangeMinimum());
      this->setRangeMaximumPosition(newPosition);
      }
    mouseEvent->accept();
    }

  mouseEvent->ignore();  
}

// --------------------------------------------------------------------------
// Standard Qt UI mouseEvents
void qCTKRangeSlider::mouseReleaseEvent(QMouseEvent* mouseEvent)
{
  QCTK_D(qCTKRangeSlider);
  QSlider::mouseReleaseEvent(mouseEvent);
  setSliderDown(false);
  
  d->m_RangeMinimumSliderSelected = QStyle::SC_None;
  d->m_RangeMaximumSliderSelected = QStyle::SC_None;

  this->update();
}

