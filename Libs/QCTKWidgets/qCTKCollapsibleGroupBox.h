#ifndef __qCTKCollapsibleGroupBox_h
#define __qCTKCollapsibleGroupBox_h

#include "qCTKWidgetsWin32Header.h"

#include <QGroupBox>

class QCTK_WIDGETS_EXPORT qCTKCollapsibleGroupBox : public QGroupBox
{
  Q_OBJECT
  Q_PROPERTY(bool Expanded READ isExpanded WRITE setExpanded)
  
  
public:
  qCTKCollapsibleGroupBox(QWidget* parent = 0);
  virtual ~qCTKCollapsibleGroupBox();
  
protected slots:
  virtual bool isExpanded();
  virtual void setExpanded(bool expanded);

protected:
  virtual void childEvent(QChildEvent*);

#if QT_VERSION < 0x040600
  virtual void paintEvent(QPaintEvent*);
  virtual void mousePressEvent(QMouseEvent*);
  virtual void mouseReleaseEvent(QMouseEvent*);
#endif

  int  Width;
  int  Height;
  bool Expanded;
};


/*
#include <QFrame>
class QTreeWidget;

class QCTK_WIDGETS_EXPORT qCTKCollapsibleGroupBox : public QFrame
{
  Q_OBJECT
  
  Q_PROPERTY(QString title READ title WRITE setTitle)
public:
  qCTKCollapsibleGroupBox(QWidget* parent = 0);
  virtual ~qCTKCollapsibleGroupBox();
  
  // Description:
  // Set/Get title
  void setTitle(QString);
  QString title() const;
public slots:
  void expand();
  void collapse();
                 
protected slots:
  void expandChildren();
  void collapseChildren();

protected:
  QTreeWidget* GroupBoxHeader;
};
*/

#endif
