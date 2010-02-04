/*=========================================================================

  Library:   qCTK

  Copyright (c) Kitware Inc. 
  All rights reserved.
  Distributed under a BSD License. See LICENSE.txt file.

  This software is distributed "AS IS" WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the above copyright notice for more information.

=========================================================================*/

#include "qCTKUtils.h"

//------------------------------------------------------------------------------
void qCTKUtils::stringListToArray(const QStringList& strList,
                                        std::vector<char*>& charstarList)
{
  // Resize if required
  if (strList.count() != static_cast<int>(charstarList.size()))
    {
    charstarList.resize(strList.count());
    }
  for (int i = 0; i < strList.count(); ++i)
    {
    charstarList[i] = strList[i].toLatin1().data();
    }
}
