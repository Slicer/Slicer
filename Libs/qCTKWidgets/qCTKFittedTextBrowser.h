#ifndef __qCTKFittedTextBrowser_h
#define __qCTKFittedTextBrowser_h

// QT includes
#include <QTextBrowser>

#include "qCTKWidgetsExport.h"

class QCTK_WIDGETS_EXPORT qCTKFittedTextBrowser : public QTextBrowser
{
  Q_OBJECT

public:
  qCTKFittedTextBrowser(QWidget* parent = 0);
  virtual ~qCTKFittedTextBrowser();

  virtual QSize sizeHint() const;
  virtual QSize minimumSizeHint() const;
  virtual int heightForWidth(int width) const;

protected slots:
  void heightForWidthMayHaveChanged();

protected:
  virtual void resizeEvent(QResizeEvent* e);
};

#endif
