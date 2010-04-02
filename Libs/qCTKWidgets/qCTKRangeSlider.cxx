#include "qCTKRangeSlider.h"

#include <QDebug>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QStyleOptionSlider>
#include <QApplication>
#include <QStylePainter>
#include <QStyle>

class qCTKRangeSliderPrivate:public qCTKPrivate<qCTKRangeSlider>
{
public:
  qCTKRangeSliderPrivate();
  void init();
  
  // Description:
  // Copied verbatim from QSliderPrivate class (see QSlider.cpp)
  int pixelPosToRangeValue(int pos) const;

  // Description:
  // Draw the bottom and top sliders.
  void drawMinimumSlider( QStylePainter* painter ) const;
  void drawMaximumSlider( QStylePainter* painter ) const;
    
  // Description:
  // End points of the range on the Model
  int m_MaximumValue;
  int m_MinimumValue;

  // Description:
  // End points of the range on the GUI. This is synced with the model.
  int m_MaximumPosition;
  int m_MinimumPosition;

  // Description:
  // Controls selected ?
  QStyle::SubControl m_MinimumSliderSelected;
  QStyle::SubControl m_MaximumSliderSelected;

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
  this->m_MinimumValue = 0;
  this->m_MaximumValue = 100;
  this->m_MinimumPosition = 0;
  this->m_MaximumPosition = 100;
  this->m_MinimumSliderSelected = QStyle::SC_None;
  this->m_MaximumSliderSelected = QStyle::SC_None;
  this->m_SubclassClickOffset = 0;
  this->m_SubclassPosition = 0;
}

// --------------------------------------------------------------------------
void qCTKRangeSliderPrivate::init()
{
  QCTK_P(qCTKRangeSlider);
  this->m_MinimumValue = p->minimum();
  this->m_MaximumValue = p->maximum();
  this->m_MinimumPosition = p->minimum();
  this->m_MaximumPosition = p->maximum();
  p->connect(p, SIGNAL(rangeChanged(int, int)), p, SLOT(onRangeChanged(int, int)));
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
  option.sliderValue = m_MinimumValue;
  option.sliderPosition = m_MinimumPosition;
  if (m_MinimumSliderSelected == QStyle::SC_SliderHandle)
    {
    option.activeSubControls = m_MinimumSliderSelected;
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
  option.sliderValue = m_MaximumValue;
  option.sliderPosition = m_MaximumPosition;
  if (m_MaximumSliderSelected == QStyle::SC_SliderHandle)
    {
    option.activeSubControls = m_MaximumSliderSelected;
    option.state |= QStyle::State_Sunken;
    }

  painter->drawComplexControl(QStyle::CC_Slider, option);
}

// --------------------------------------------------------------------------
qCTKRangeSlider::qCTKRangeSlider(QWidget* parent) 
  : QSlider(parent)
{
  QCTK_INIT_PRIVATE(qCTKRangeSlider);
  qctk_d()->init();
}

// --------------------------------------------------------------------------
qCTKRangeSlider::qCTKRangeSlider( Qt::Orientation o, 
                                  QWidget* parentObject )
  :QSlider(o, parentObject)
{
  QCTK_INIT_PRIVATE(qCTKRangeSlider);
  qctk_d()->init();
}

// --------------------------------------------------------------------------
qCTKRangeSlider::~qCTKRangeSlider()
{
}

// --------------------------------------------------------------------------
int qCTKRangeSlider::minimumValue() const
{
  QCTK_D(const qCTKRangeSlider);
  return d->m_MinimumValue;
}

// --------------------------------------------------------------------------
void qCTKRangeSlider::setMinimumValue( int min )
{
  QCTK_D(qCTKRangeSlider);
  this->setValues( min, qMax(d->m_MaximumValue,min) );
}

// --------------------------------------------------------------------------
int qCTKRangeSlider::maximumValue() const
{
  QCTK_D(const qCTKRangeSlider);
  return d->m_MaximumValue;
}

// --------------------------------------------------------------------------
void qCTKRangeSlider::setMaximumValue( int max )
{
  QCTK_D(qCTKRangeSlider);
  this->setValues( qMin(d->m_MinimumValue, max), max );
}

// --------------------------------------------------------------------------
void qCTKRangeSlider::setValues(int l, int u)
{
  QCTK_D(qCTKRangeSlider);
  const int minimumValue = 
    qBound(this->minimum(), qMin(l,u), this->maximum());
  const int maximumValue = 
    qBound(this->minimum(), qMax(l,u), this->maximum());
  bool emitMinValChanged = (minimumValue != d->m_MinimumValue);
  bool emitMaxValChanged = (maximumValue != d->m_MaximumValue);
  
  d->m_MinimumValue = minimumValue;
  d->m_MaximumValue = maximumValue;
  
  bool emitMinPosChanged = 
    (minimumValue == d->m_MinimumPosition);
  bool emitMaxPosChanged = 
    (maximumValue == d->m_MaximumPosition);
  d->m_MinimumPosition = minimumValue;
  d->m_MaximumPosition = maximumValue;
  
  if (isSliderDown())
    {
    if (emitMinPosChanged)
      {
      emit minimumPositionChanged(minimumValue);
      }
    if (emitMaxPosChanged)
      {
      emit maximumPositionChanged(maximumValue);
      }
    if (emitMinPosChanged || emitMaxPosChanged)
      {
      emit positionsChanged(minimumValue, maximumValue);
      }
    }
  if (emitMinValChanged)
    {
    emit minimumValueChanged(minimumValue);
    }
  if (emitMaxValChanged)
    {
    emit maximumValueChanged(maximumValue);
    }
  if (emitMinValChanged || emitMaxValChanged)
    {
    emit valuesChanged(d->m_MinimumValue, 
                       d->m_MaximumValue);
    }
  if (emitMinPosChanged || emitMaxPosChanged || 
      emitMinValChanged || emitMaxValChanged)
    {
    this->update();
    }
}

// --------------------------------------------------------------------------
int qCTKRangeSlider::minimumPosition() const
{
  QCTK_D(const qCTKRangeSlider);
  return d->m_MinimumPosition;
}

// --------------------------------------------------------------------------
int qCTKRangeSlider::maximumPosition() const
{
  QCTK_D(const qCTKRangeSlider);
  return d->m_MaximumPosition;
}

// --------------------------------------------------------------------------
void qCTKRangeSlider::setMinimumPosition(int l)
{
  QCTK_D(const qCTKRangeSlider);
  this->setPositions(l, d->m_MaximumPosition);
}

// --------------------------------------------------------------------------
void qCTKRangeSlider::setMaximumPosition(int u)
{
  QCTK_D(const qCTKRangeSlider);
  this->setPositions(d->m_MinimumPosition, u);
}

// --------------------------------------------------------------------------
void qCTKRangeSlider::setPositions(int min, int max)
{
  QCTK_D(qCTKRangeSlider);
  const int minPosition = 
    qBound(this->minimum(), qMin(min, max), this->maximum());
  const int maxPosition = 
    qBound(this->minimum(), qMax(min, max), this->maximum());

  bool emitMinPosChanged = (minPosition == d->m_MinimumPosition);
  bool emitMaxPosChanged = (maxPosition == d->m_MaximumPosition);

  if (!emitMinPosChanged && !emitMaxPosChanged)
    {
    return;
    }

  d->m_MinimumPosition = minPosition;
  d->m_MaximumPosition = maxPosition;

  if (!this->hasTracking())
    {
    this->update();
    }
  if (isSliderDown())
    {
    if (emitMinPosChanged)
      {
      emit minimumPositionChanged(d->m_MinimumPosition);
      }
    if (emitMaxPosChanged)
      {
      emit maximumPositionChanged(d->m_MaximumPosition);
      }
    if (emitMinPosChanged || emitMaxPosChanged)
      {
      emit positionsChanged(d->m_MinimumPosition, d->m_MaximumPosition);
      }
    }
  if (this->hasTracking())
    {
    this->triggerAction(SliderMove);
    this->setValues(d->m_MinimumPosition, d->m_MaximumPosition);
    }
}

// --------------------------------------------------------------------------
void qCTKRangeSlider::onRangeChanged(int minimum, int maximum)
{
  QCTK_D(qCTKRangeSlider);
  this->setValues(d->m_MinimumValue, d->m_MaximumValue);
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

  option.sliderPosition = d->m_MinimumPosition;
  const QRect lr = style()->subControlRect( QStyle::CC_Slider, 
                                            &option, 
                                            QStyle::SC_SliderHandle, 
                                            this);
  option.sliderPosition = d->m_MaximumPosition;

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
    option.sliderPosition = d->m_MinimumPosition;
    option.sliderValue    = d->m_MinimumValue;

    QStyle::SubControl& control = d->m_MinimumSliderSelected;
    
    control = this->style()->hitTestComplexControl( QStyle::CC_Slider, 
                                                    &option, 
                                                    mouseEvent->pos(), 
                                                    this);

    if (control == QStyle::SC_SliderHandle)
      {
      d->m_SelectedSlider = true;
      d->m_SubclassPosition = d->m_MinimumValue;

      const QRect sr = this->style()->subControlRect( QStyle::CC_Slider, 
                                                      &option, 
                                                      QStyle::SC_SliderHandle, 
                                                      this);
      
      d->m_SubclassClickOffset = mouseEvent->pos().x() - sr.topLeft().x();

      this->setSliderDown(true);

      if (control != d->m_MinimumSliderSelected)
        {
        this->update(sr);
        }
      }
    }


  // Check if the other slider is pressed

  if (!this->isSliderDown())
    {
    option.sliderPosition = d->m_MaximumPosition;
    option.sliderValue    = d->m_MaximumValue;

    QStyle::SubControl& control = d->m_MaximumSliderSelected;
    
    control = this->style()->hitTestComplexControl( QStyle::CC_Slider, 
                                                    &option, 
                                                    mouseEvent->pos(), 
                                                    this);

    if (control == QStyle::SC_SliderHandle)
      {
      d->m_SelectedSlider = false;
      d->m_SubclassPosition = d->m_MaximumValue;

      const QRect sr = this->style()->subControlRect( QStyle::CC_Slider, 
                                                      &option, 
                                                      QStyle::SC_SliderHandle, 
                                                      this);

      d->m_SubclassClickOffset = mouseEvent->pos().x() - sr.topLeft().x();

      this->setSliderDown(true);

      if (d->m_MaximumSliderSelected != control)
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
  if (d->m_MinimumSliderSelected == QStyle::SC_SliderHandle ||
      d->m_MaximumSliderSelected == QStyle::SC_SliderHandle)
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

    if (d->m_MinimumSliderSelected == QStyle::SC_SliderHandle)
      {
      this->setMinimumPosition(qMin(newPosition,d->m_MaximumPosition));
      }
    else if (d->m_MaximumSliderSelected == QStyle::SC_SliderHandle)
      {
      this->setMaximumPosition(qMax(d->m_MinimumPosition, newPosition));
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
  
  d->m_MinimumSliderSelected = QStyle::SC_None;
  d->m_MaximumSliderSelected = QStyle::SC_None;

  this->update();
}

