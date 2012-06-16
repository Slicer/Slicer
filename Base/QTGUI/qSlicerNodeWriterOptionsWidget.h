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

#ifndef __qSlicerNodeWriterOptionsWidget_h
#define __qSlicerNodeWriterOptionsWidget_h

/// QtCore includes
#include "qSlicerFileWriterOptionsWidget.h"
class qSlicerNodeWriterOptionsWidgetPrivate;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerNodeWriterOptionsWidget
  : public qSlicerFileWriterOptionsWidget
{
  Q_OBJECT
  /// If the storage node doesn't support compression, the option can be hidden
  Q_PROPERTY(bool showUseCompression READ showUseCompression WRITE setShowUseCompression)

public:
  typedef qSlicerFileWriterOptionsWidget Superclass;
  explicit qSlicerNodeWriterOptionsWidget(QWidget* parent = 0);
  virtual ~qSlicerNodeWriterOptionsWidget();

  bool showUseCompression()const;
  void setShowUseCompression(bool show);

  virtual bool isValid()const;

public slots:
  virtual void setObject(vtkObject* object);

protected slots:
  virtual void setUseCompression(bool use);

private:
  Q_DECLARE_PRIVATE_D(qGetPtrHelper(qSlicerIOOptions::d_ptr), qSlicerNodeWriterOptionsWidget);
};

#endif
