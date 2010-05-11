#ifndef __qCTKCollapsibleWidget2_h
#define __qCTKCollapsibleWidget2_h

#include <QWidget>
#include <QFrame>
#include "qCTKPimpl.h"
#include "qCTKWidgetsExport.h"

class qCTKCollapsibleWidget2Private;

class QCTK_WIDGETS_EXPORT qCTKCollapsibleWidget2 : public QWidget
{
  Q_OBJECT

  Q_PROPERTY(QString title READ title WRITE setTitle)
  Q_PROPERTY(bool collapsed READ collapsed WRITE setCollapsed)
  Q_PROPERTY(int collapsedHeight READ collapsedHeight WRITE setCollapsedHeight)

  Q_PROPERTY(QFrame::Shape contentsFrameShape READ contentsFrameShape WRITE setContentsFrameShape)
  Q_PROPERTY(QFrame::Shadow contentsFrameShadow READ contentsFrameShadow WRITE setContentsFrameShadow)
  Q_PROPERTY(int contentsLineWidth READ contentsLineWidth WRITE setContentsLineWidth)
  Q_PROPERTY(int contentsMidLineWidth READ contentsMidLineWidth WRITE setContentsMidLineWidth)

public:
  qCTKCollapsibleWidget2(QWidget *parent = 0);
  qCTKCollapsibleWidget2(const QString& title, QWidget *parent = 0);
  virtual ~qCTKCollapsibleWidget2();

  // Description:
  // Button title.
  void setTitle(QString t);
  QString title()const;

  // Description:
  // Property that describes if the widget is collapsed or not.
  // When collapsed, the children are invisible and the widget
  // has a sized defined by CollapsedHeight
  void setCollapsed(bool);
  bool collapsed()const;

  // Description:
  // Height used when the widget is collapsed
  void setCollapsedHeight(int);
  int collapsedHeight()const;

  // Description:
  // Set the frame shape of the contents
  // Hint: StyledPanel is probably the shape you are looking for
  QFrame::Shape contentsFrameShape() const;
  void setContentsFrameShape(QFrame::Shape);

  // Description:
  // Set the frame shadow of the contents
  // Hint: Raised is probably the shadow you are looking for
  QFrame::Shadow contentsFrameShadow() const;
  void setContentsFrameShadow(QFrame::Shadow);

  // Description:
  // Set the line width of the frame around the contents
  int contentsLineWidth() const;
  void setContentsLineWidth(int);

  // Description:
  // Set the mid line width of the frame around the contents
  int contentsMidLineWidth() const;
  void setContentsMidLineWidth(int);

  // Description:
  // Reimplement for internal reasons
  virtual QSize minimumSizeHint()const;

  // Description:
  // Reimplement for internal reasons
  virtual QSize sizeHint()const;

public slots:
  // Description:
  // Collapse the widget if not already collapsed.
  // Expand the widget if not already expanded.
  void toggleCollapse();

signals:
  // Description:
  // Signal emitted when the widget is collapsed or expanded
  void contentsCollapsed(bool);

protected slots:
  // Description:
  // Perform the collapse.
  virtual void collapse(bool c);

protected:
  virtual void paintEvent(QPaintEvent*);
  virtual void mousePressEvent(QMouseEvent* event);
  virtual void mouseReleaseEvent(QMouseEvent* event);
  virtual void childEvent(QChildEvent* c);

private:
  CTK_DECLARE_PRIVATE(qCTKCollapsibleWidget2);
  //struct qInternal;
  //qInternal* Internal;
};

#endif
