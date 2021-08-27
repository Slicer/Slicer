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

==============================================================================*/

#ifndef __qSlicerExportNodeDialog_h
#define __qSlicerExportNodeDialog_h

// Slicer includes
#include "qSlicerFileDialog.h"
#include "qSlicerBaseQTGUIExport.h"

/// Forward declarations
class qSlicerExportNodeDialogPrivate;

//------------------------------------------------------------------------------
class Q_SLICER_BASE_QTGUI_EXPORT qSlicerExportNodeDialog : public qSlicerFileDialog
{
  Q_OBJECT
public:
  typedef QObject Superclass;
  qSlicerExportNodeDialog(QObject* parent = nullptr);
  ~qSlicerExportNodeDialog() override;

  qSlicerIO::IOFileType fileType()const override;
  QString description()const override;
  qSlicerFileDialog::IOAction action()const override;

  /*! Open the data dialog and save the nodes/scene */
  bool exec(const qSlicerIO::IOProperties& readerProperties =
                    qSlicerIO::IOProperties()) override;

protected:
  QScopedPointer<qSlicerExportNodeDialogPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerExportNodeDialog);
  Q_DISABLE_COPY(qSlicerExportNodeDialog);
};

#endif
