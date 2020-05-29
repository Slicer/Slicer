/*==============================================================================

  Program: 3D Slicer

  Copyright (c) BWH

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#ifndef __qSlicerMarkupsReader
#define __qSlicerMarkupsReader

// Slicer includes
#include "qSlicerFileReader.h"

class qSlicerMarkupsReaderPrivate;
class vtkSlicerMarkupsLogic;

//----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Markups
class qSlicerMarkupsReader
  : public qSlicerFileReader
{
  Q_OBJECT
public:
  typedef qSlicerFileReader Superclass;
  qSlicerMarkupsReader(QObject* parent = nullptr);
  qSlicerMarkupsReader(vtkSlicerMarkupsLogic* logic, QObject* parent = nullptr);
  ~qSlicerMarkupsReader() override;

  vtkSlicerMarkupsLogic* markupsLogic()const;
  void setMarkupsLogic(vtkSlicerMarkupsLogic* logic);

  QString description()const override;
  IOFileType fileType()const override;
  QStringList extensions()const override;

  bool load(const IOProperties& properties) override;

protected:
  QScopedPointer<qSlicerMarkupsReaderPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerMarkupsReader);
  Q_DISABLE_COPY(qSlicerMarkupsReader);
};

#endif
