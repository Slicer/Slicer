#include "qCTKFittedTextBrowser.h"

#include <QDebug>
#include <QTextBlock>
#include <QAbstractTextDocumentLayout>

qCTKFittedTextBrowser::qCTKFittedTextBrowser(QWidget* parent)
  : QTextBrowser(parent)
{
  this->connect(this, SIGNAL(textChanged()), SLOT(heightForWidthMayHaveChanged()));
}

qCTKFittedTextBrowser::~qCTKFittedTextBrowser()
{

}

void qCTKFittedTextBrowser::heightForWidthMayHaveChanged() 
{
  this->updateGeometry();
}

int qCTKFittedTextBrowser::heightForWidth(int width) const 
{
  QTextDocument* doc = this->document();
  qreal savedWidth = doc->textWidth();
  
  // Fudge factor. This is the difference between the frame and the 
  // viewport.
  int fudge = 2 * this->frameWidth();
  
  // Do the calculation assuming no scrollbars
  doc->setTextWidth(width - fudge);
  int noScrollbarHeight =
    doc->documentLayout()->documentSize().height() + fudge;
  
  // (If noScrollbarHeight is greater than the maximum height we'll be
  // allowed, then there will be scrollbars, and the actual required
  // height will be even higher. But since in this case we've already
  // hit the maximum height, it doesn't matter that we underestimate.)
  
  // Get minimum height (even if string is empty): one line of text
  int minimumHeight = QFontMetrics(doc->defaultFont()).lineSpacing() + fudge;
  int ret = qMax(noScrollbarHeight, minimumHeight);

  doc->setTextWidth(savedWidth);
  return ret;
}

QSize qCTKFittedTextBrowser::minimumSizeHint() const {
  QSize s(this->size().width(), 0);
  if (s.width() == 0)
    {
    s.setWidth(400); // arbitrary value
    }
  s.setHeight(this->heightForWidth(s.width()));
  return s;
}

QSize qCTKFittedTextBrowser::sizeHint() const {
  return this->minimumSizeHint();
}

void qCTKFittedTextBrowser::resizeEvent(QResizeEvent* e)
{
  this->QTextBrowser::resizeEvent(e);
  if (e->size().height() != this->heightForWidth(e->size().width()))
    {
    this->heightForWidthMayHaveChanged();
    }
}
