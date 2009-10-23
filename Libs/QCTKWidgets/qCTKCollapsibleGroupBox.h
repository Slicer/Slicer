#ifndef __qCTKCollapsibleGroupBox_h
#define __qCTKCollapsibleGroupBox_h

#include "qCTKWidgetsWin32Header.h"

#include <QGroupBox>

class QCTK_WIDGETS_EXPORT qCTKCollapsibleGroupBox : public QGroupBox
{
  Q_OBJECT
public:
  qCTKCollapsibleGroupBox(QWidget* parent = 0);
  virtual ~qCTKCollapsibleGroupBox();
  
protected slots:
  virtual void expand(bool expand);

protected:
  virtual void childEvent(QChildEvent*);

#if QT_VERSION < 0x040600
  virtual void paintEvent(QPaintEvent*);
  virtual void mousePressEvent(QMouseEvent*);
  virtual void mouseReleaseEvent(QMouseEvent*);
#endif
  QSize OldSize;
  int   MaxHeight;
};

#endif
