/*=========================================================================

  Library:   qCTK

  Copyright (c) Kitware Inc. 
  All rights reserved.
  Distributed under a BSD License. See LICENSE.txt file.

  This software is distributed "AS IS" WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the above copyright notice for more information.

=========================================================================*/

#ifndef __qCTKMenuButton_h
#define __qCTKMenuButton_h

/// QT includes
#include <QPushButton>

/// qCTK includes
#include "qCTKPimpl.h"
#include "qCTKWidgetsExport.h"

class qCTKMenuButtonPrivate;

/// Description
/// A Menu widget that show/hide its children depending on its checked/collapsed properties
class QCTK_WIDGETS_EXPORT qCTKMenuButton : public QPushButton
{
  Q_OBJECT

public:
  qCTKMenuButton(QWidget *parent = 0);
  qCTKMenuButton(const QString& text, QWidget *parent = 0);
  virtual ~qCTKMenuButton();
  
  /// 
  /// don't use menu/setMenu but extraMenu/setExtraMenu instead.
  QMenu* extraMenu()const;
  void setExtraMenu(QMenu* menu);
  
  virtual QSize minimumSizeHint()const;
  virtual QSize sizeHint()const;
  
public slots:
  void showExtraMenu();

protected:
  virtual void paintEvent(QPaintEvent*);
  virtual void mousePressEvent(QMouseEvent* event);

  virtual bool hitButton(const QPoint & pos) const;
  virtual void initStyleOption ( QStyleOptionButton * option ) const;
private:
  QCTK_DECLARE_PRIVATE(qCTKMenuButton);
};

#endif
