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

#ifndef __qSlicerModelsDialog_h
#define __qSlicerModelsDialog_h

// CTK includes
#include <ctkPimpl.h>

// Slicer includes
#include "qSlicerFileDialog.h"
#include "qSlicerBaseQTGUIExport.h"

class qSlicerModelsDialogPrivate;

//------------------------------------------------------------------------------
class Q_SLICER_BASE_QTGUI_EXPORT qSlicerModelsDialog : public qSlicerFileDialog
{
  Q_OBJECT
public:
  typedef qSlicerFileDialog Superclass;
  qSlicerModelsDialog(QObject* parent =nullptr);
  ~qSlicerModelsDialog() override;

  qSlicerIO::IOFileType fileType()const override;
  QString description()const override;
  qSlicerFileDialog::IOAction action()const override;

  /// run the dialog to select the file/files/directory
  bool exec(const qSlicerIO::IOProperties& readerProperties =
                    qSlicerIO::IOProperties()) override;

  QStringList loadedNodes()const override;
protected:
  QScopedPointer<qSlicerModelsDialogPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerModelsDialog);
  Q_DISABLE_COPY(qSlicerModelsDialog);
};

#endif
