/*=========================================================================

  Library:   qCTK

  Copyright (c) Kitware Inc. 
  All rights reserved.
  Distributed under a BSD License. See LICENSE.txt file.

  This software is distributed "AS IS" WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the above copyright notice for more information.

=========================================================================*/

#ifndef __qCTKUtils_h
#define __qCTKUtils_h

// QT includes
#include <QStringList>

// STL includes
#include <vector>

#include "qCTKWidgetsExport.h"

class QCTK_WIDGETS_EXPORT qCTKUtils
{
  
public:
  typedef qCTKUtils Self;

  ///
  /// Convert a QStringList to Vector of char*
  static void stringListToArray(const QStringList& strList, std::vector<char*>& charstarList);

private:
  /// Not implemented
  qCTKUtils(){}
  virtual ~qCTKUtils(){}

};

#endif
