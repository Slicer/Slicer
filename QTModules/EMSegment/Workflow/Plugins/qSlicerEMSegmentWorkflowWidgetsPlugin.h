/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Danielle Pace, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerEMSegmentWorkflowWidgetsPlugin_h
#define __qSlicerEMSegmentWorkflowWidgetsPlugin_h

#include "qSlicerEMSegmentWorkflowWidgetStepPlugin.h"

#include <QDesignerCustomWidgetCollectionInterface>
#include "qSlicerEMSegmentWorkflowWidgetsPluginExport.h"

// \class Group the plugins in one library
class Q_SLICER_QTMODULES_EMSEGMENT_WORKFLOW_WIDGETS_PLUGIN_EXPORT qSlicerEMSegmentWorkflowWidgetsPlugin : 
  public QObject,
  public QDesignerCustomWidgetCollectionInterface
{
  Q_OBJECT
  Q_INTERFACES(QDesignerCustomWidgetCollectionInterface);

public:
  QList<QDesignerCustomWidgetInterface*> customWidgets() const
    {
    QList<QDesignerCustomWidgetInterface *> plugins;
    plugins << new qSlicerEMSegmentWorkflowWidgetStepPlugin;
    return plugins;
    }
};

#endif
