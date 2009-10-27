#ifndef __qCTKTitleComboBox_h
#define __qCTKTitleComboBox_h

#include "qCTKWidgetsWin32Header.h"

#include <QComboBox>

class QCTK_WIDGETS_EXPORT qCTKTitleComboBox : public QComboBox
{
  Q_OBJECT
  Q_PROPERTY(QString title READ title WRITE setTitle)
  Q_PROPERTY(QIcon icon READ icon WRITE setIcon)

public:
  qCTKTitleComboBox(QWidget* parent = 0);
  virtual ~qCTKTitleComboBox();

  void setTitle(const QString&);
  QString title()const;
  
  void setIcon(const QIcon&);
  QIcon icon()const;

protected:
  virtual void paintEvent(QPaintEvent*);

  QString Title;
  QIcon   Icon;
  
};

#endif
