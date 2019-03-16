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

#ifndef __qSlicerFileWriterOptionsWidget_h
#define __qSlicerFileWriterOptionsWidget_h

/// QtGUI includes
#include "qSlicerIOOptionsWidget.h"
class qSlicerFileWriterOptionsWidgetPrivate;

/// Base class for all the Writer Options widget.
class Q_SLICER_BASE_QTGUI_EXPORT qSlicerFileWriterOptionsWidget
  : public qSlicerIOOptionsWidget
{
  Q_OBJECT

public:
  typedef qSlicerIOOptionsWidget Superclass;
  explicit qSlicerFileWriterOptionsWidget(QWidget* parent = nullptr);
  ~qSlicerFileWriterOptionsWidget() override;

public slots:
  /// Set the object to write (typically a scene or a MRML node)
  virtual void setObject(vtkObject* object);

protected:
  qSlicerFileWriterOptionsWidget(qSlicerIOOptionsPrivate* pimpl,
                                 QWidget* parent);
};

#endif
