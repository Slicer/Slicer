/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#include "qSlicerIO.h"

#ifndef __qSlicerIOOptionsWidget_p_h
#define __qSlicerIOOptionsWidget_p_h

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Slicer API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

//-----------------------------------------------------------------------------
class Q_SLICER_BASE_QTCORE_EXPORT qSlicerIOOptionsPrivate
{
public:
  virtual ~qSlicerIOOptionsPrivate();
  qSlicerIO::IOProperties Properties;
  bool ArePropertiesValid;
};

#endif
