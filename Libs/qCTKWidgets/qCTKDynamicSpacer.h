/*=========================================================================

  Library:   qCTK

  Copyright (c) Kitware Inc. 
  All rights reserved.
  Distributed under a BSD License. See LICENSE.txt file.

  This software is distributed "AS IS" WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the above copyright notice for more information.

=========================================================================*/

#ifndef __qCTKDynamicSpacer_h
#define __qCTKDynamicSpacer_h

/// QT includes
#include <QWidget>

/// qCTK includes
#include "qCTKPimpl.h"
#include "qCTKWidgetsExport.h"

class qCTKDynamicSpacerPrivate;

/// Description
/// A Menu widget that show/hide its children depending on its checked/collapsed properties
class QCTK_WIDGETS_EXPORT qCTKDynamicSpacer : public QWidget
{
  Q_OBJECT
  Q_PROPERTY(QSizePolicy activeSizePolicy READ activeSizePolicy WRITE setActiveSizePolicy);
  Q_PROPERTY(QSizePolicy inactiveSizePolicy READ inactiveSizePolicy WRITE setInactiveSizePolicy);

public:
  qCTKDynamicSpacer(QWidget *parent = 0);
  virtual ~qCTKDynamicSpacer();

  QSizePolicy activeSizePolicy() const;
  void setActiveSizePolicy(QSizePolicy sizePolicy);
  inline void setActiveSizePolicy(QSizePolicy::Policy horizontal, QSizePolicy::Policy vertical);

  QSizePolicy inactiveSizePolicy() const;
  void setInactiveSizePolicy(QSizePolicy sizePolicy);
  inline void setInactiveSizePolicy(QSizePolicy::Policy horizontal, QSizePolicy::Policy vertical);

public slots:
  void activate(bool enable);

private:
  QCTK_DECLARE_PRIVATE(qCTKDynamicSpacer);
};

void qCTKDynamicSpacer::setActiveSizePolicy(QSizePolicy::Policy horizontal, QSizePolicy::Policy vertical)
{
  this->setActiveSizePolicy(QSizePolicy(horizontal, vertical));
}

#endif
