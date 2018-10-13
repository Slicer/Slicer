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

#ifndef __qSlicerIOOptions_h
#define __qSlicerIOOptions_h

// Qt includes
#include <QScopedPointer>

/// QtCore includes
#include "qSlicerIO.h"
#include "qSlicerBaseQTCoreExport.h"
class qSlicerIOOptionsPrivate;

class Q_SLICER_BASE_QTCORE_EXPORT qSlicerIOOptions
{
public:
  /// Constructor initialize empty properties
  explicit qSlicerIOOptions();
  virtual ~qSlicerIOOptions();

  /// Returns true if the options have been set and if they are
  /// meaningful. By default, checks that there is at least 1 option.
  /// To be reimplemented in subclasses
  virtual bool isValid()const;

  const qSlicerIO::IOProperties& properties()const;
protected:
  qSlicerIOOptions(qSlicerIOOptionsPrivate* pimpl);
  QScopedPointer<qSlicerIOOptionsPrivate> d_ptr;

  /// Must be called anytime the result of isValid() can change
  virtual void updateValid();

private:
  Q_DECLARE_PRIVATE(qSlicerIOOptions);
  Q_DISABLE_COPY(qSlicerIOOptions);
};

#endif
