#ifndef __qCTKCollapsibleWidget_h
#define __qCTKCollapsibleWidget_h

#include <QFrame>

#include "qCTKWidgetsWin32Header.h"

class QPushButton;
class QStackedWidget;
class QVBoxLayout;

class QCTK_WIDGETS_EXPORT qCTKCollapsibleWidget : public QFrame
{
  Q_OBJECT
  
  Q_PROPERTY(QString title READ title WRITE setTitle)
  Q_PROPERTY(bool collapsed READ collapsed WRITE setCollapsed)
  Q_PROPERTY(bool collapseChildren READ collapseChildren WRITE setCollapseChildren)
  Q_PROPERTY(int collapsedHeight READ collapsedHeight WRITE setCollapsedHeight)

  Q_PROPERTY(Shape contentsFrameShape READ contentsFrameShape WRITE setContentsFrameShape)
  Q_PROPERTY(Shadow contentsFrameShadow READ contentsFrameShadow WRITE setContentsFrameShadow)
  Q_PROPERTY(int contentsLineWidth READ contentsLineWidth WRITE setContentsLineWidth)
  Q_PROPERTY(int contentsMidLineWidth READ contentsMidLineWidth WRITE setContentsMidLineWidth)

public:
  qCTKCollapsibleWidget(QWidget *parent = 0);
  ~qCTKCollapsibleWidget();
  
  QWidget *widget();
  QFrame* contents();

  void setTitle(QString t);
  QString title()const;
  
  void setCollapsed(bool);
  bool collapsed()const;

  void setCollapseChildren(bool);
  bool collapseChildren()const;

  void setCollapsedHeight(int);
  int collapsedHeight()const;

  Shape contentsFrameShape() const;
  void setContentsFrameShape(Shape);
  Shadow contentsFrameShadow() const;
  void setContentsFrameShadow(Shadow);
  
  int contentsLineWidth() const;
  void setContentsLineWidth(int);

  int contentsMidLineWidth() const;
  void setContentsMidLineWidth(int);

public slots:
  void setWidget(QWidget *);

protected slots:
  virtual void collapse(bool c);

private:
  bool  CollapseChildren;
  int   CollapsedHeight;
  QSize OldSize;
  int   MaxHeight;

  QPushButton* Header;
  QStackedWidget *StackWidget;
};

#endif
