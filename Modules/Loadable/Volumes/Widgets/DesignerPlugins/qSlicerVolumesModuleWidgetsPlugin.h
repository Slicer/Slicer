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

#ifndef __qSlicerVolumesModuleWidgetsPlugin_h
#define __qSlicerVolumesModuleWidgetsPlugin_h

// Qt includes
#include <QtUiPlugin/QDesignerCustomWidgetCollectionInterface>

// Volumes includes
#include "qSlicerVolumesModuleWidgetsPlugin.h"
#include "qSlicerDiffusionTensorVolumeDisplayWidgetPlugin.h"
#include "qSlicerDiffusionWeightedVolumeDisplayWidgetPlugin.h"
#include "qSlicerDTISliceDisplayWidgetPlugin.h"
#include "qSlicerLabelMapVolumeDisplayWidgetPlugin.h"
#include "qSlicerScalarVolumeDisplayWidgetPlugin.h"
#include "qSlicerVolumeDisplayWidgetPlugin.h"

// \class Group the plugins in one library
class Q_SLICER_MODULE_VOLUMES_WIDGETS_PLUGINS_EXPORT qSlicerVolumesModuleWidgetsPlugin
  : public QObject
  , public QDesignerCustomWidgetCollectionInterface
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QDesignerCustomWidgetCollectionInterface")
  Q_INTERFACES(QDesignerCustomWidgetCollectionInterface);

public:
  QList<QDesignerCustomWidgetInterface*> customWidgets() const override
    {
    QList< QDesignerCustomWidgetInterface* > plugins;
    plugins << new qSlicerDiffusionTensorVolumeDisplayWidgetPlugin;
    plugins << new qSlicerDiffusionWeightedVolumeDisplayWidgetPlugin;
    plugins << new qSlicerDTISliceDisplayWidgetPlugin;
    plugins << new qSlicerLabelMapVolumeDisplayWidgetPlugin;
    plugins << new qSlicerScalarVolumeDisplayWidgetPlugin;
    plugins << new qSlicerVolumeDisplayWidgetPlugin;

    return plugins;
    }
};

#endif
