/*=========================================================================

  Library:   qCTK

  Copyright (c) Kitware Inc. 
  All rights reserved.
  Distributed under a BSD License. See LICENSE.txt file.

  This software is distributed "AS IS" WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the above copyright notice for more information.

=========================================================================*/

#include "qCTKFittedTextBrowser.h"

#include <QDebug>
#include <QTextBlock>
#include <QAbstractTextDocumentLayout>

qCTKFittedTextBrowser::qCTKFittedTextBrowser(QWidget* _parent)
  : QTextBrowser(_parent)
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

int qCTKFittedTextBrowser::heightForWidth(int _width) const 
{
  QTextDocument* doc = this->document();
  qreal savedWidth = doc->textWidth();
  
  // Fudge factor. This is the difference between the frame and the 
  // viewport.
  int fudge = 2 * this->frameWidth();
  
  // Do the calculation assuming no scrollbars
  doc->setTextWidth(_width - fudge);
  int noScrollbarHeight =
    doc->documentLayout()->documentSize().height() + fudge;
  
  // (If noScrollbarHeight is greater than the maximum height we'll be
  // allowed, then there will be scrollbars, and the actual required
  // height will be even higher. But since in this case we've already
  // hit the maximum height, it doesn't matter that we underestimate.)
  
  // Get minimum height (even if string is empty): one line of text
  int _minimumHeight = QFontMetrics(doc->defaultFont()).lineSpacing() + fudge;
  int ret = qMax(noScrollbarHeight, _minimumHeight);

  doc->setTextWidth(savedWidth);
  return ret;
}

QSize qCTKFittedTextBrowser::minimumSizeHint() const {
  QSize s(this->size().width(), 0);
  if (s.width() == 0)
    {
    //s.setWidth(400); // arbitrary value
    return QTextBrowser::minimumSizeHint();
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
