/*==============================================================================

  Program: 3D Slicer

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#ifndef __qSlicerCornerTextModule_h
#define __qSlicerCornerTextModule_h

// Slicer includes
#include "qSlicerLoadableModule.h"

// CornerText includes
#include "qSlicerCornerTextModuleExport.h"
#include "vtkMRMLAbstractLogic.h"

class qSlicerCornerTextModulePrivate;

class Q_SLICER_QTMODULES_CORNERTEXT_EXPORT
qSlicerCornerTextModule
  : public qSlicerLoadableModule
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.slicer.modules.loadable.qSlicerLoadableModule/1.0");
  Q_INTERFACES(qSlicerLoadableModule);

public:

  typedef qSlicerLoadableModule Superclass;
  explicit qSlicerCornerTextModule(QObject *parent=nullptr);
  ~qSlicerCornerTextModule() override;

  qSlicerGetTitleMacro(tr("CornerText"));

  QString helpText()const override;
  QString acknowledgementText()const override;
  QStringList contributors()const override;

  QIcon icon()const override;

  QStringList categories()const override;
  QStringList dependencies() const override;

protected slots:
  void onLayoutChanged(int);

protected:
  /// Initialize the module
  void setup() override;

  /// Create and return the widget representation associated to this module
  qSlicerAbstractModuleRepresentation * createWidgetRepresentation() override;

  /// Create and return the logic associated to this module
  vtkMRMLAbstractLogic* createLogic() override { return nullptr; };

  void readSettings() const;
  void writeSettings() const;

protected:
  QScopedPointer<qSlicerCornerTextModulePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerCornerTextModule);
  Q_DISABLE_COPY(qSlicerCornerTextModule);
};

#endif
