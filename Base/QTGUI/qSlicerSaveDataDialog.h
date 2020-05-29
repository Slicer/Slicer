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

#ifndef __qSlicerSaveDataDialog_h
#define __qSlicerSaveDataDialog_h

// Slicer includes
#include "qSlicerFileDialog.h"
#include "qSlicerBaseQTGUIExport.h"

/// Forward declarations
class qSlicerSaveDataDialogPrivate;

//------------------------------------------------------------------------------
class Q_SLICER_BASE_QTGUI_EXPORT qSlicerSaveDataDialog : public qSlicerFileDialog
{
  Q_OBJECT
public:
  typedef QObject Superclass;
  qSlicerSaveDataDialog(QObject* parent = nullptr);
  ~qSlicerSaveDataDialog() override;

  qSlicerIO::IOFileType fileType()const override;
  QString description()const override;
  qSlicerFileDialog::IOAction action()const override;

  /// Open the data dialog and save the nodes/scene
  bool exec(const qSlicerIO::IOProperties& readerProperties =
                    qSlicerIO::IOProperties()) override;

protected:
  QScopedPointer<qSlicerSaveDataDialogPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerSaveDataDialog);
  Q_DISABLE_COPY(qSlicerSaveDataDialog);
};

#endif
