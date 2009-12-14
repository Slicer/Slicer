#ifndef __qCTKCollapsibleGroupBox_h
#define __qCTKCollapsibleGroupBox_h

#include "qCTKWidgetsExport.h"

#include <QGroupBox>

class QCTK_WIDGETS_EXPORT qCTKCollapsibleGroupBox : public QGroupBox
{
  Q_OBJECT
public:
  explicit qCTKCollapsibleGroupBox(QWidget* parent = 0);
  virtual ~qCTKCollapsibleGroupBox();
  virtual int heightForWidth(int w) const;
  virtual QSize minimumSizeHint()const;
  virtual QSize sizeHint()const;

protected slots:
  virtual void expand(bool expand);

protected:
  virtual void childEvent(QChildEvent*);

#if QT_VERSION < 0x040600
  virtual void paintEvent(QPaintEvent*);
  virtual void mousePressEvent(QMouseEvent*);
  virtual void mouseReleaseEvent(QMouseEvent*);
#endif
  virtual void resizeEvent(QResizeEvent*);

  QSize OldSize;
  int   MaxHeight;
};

#endif
