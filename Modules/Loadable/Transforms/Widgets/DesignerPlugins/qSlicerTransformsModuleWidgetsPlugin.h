/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Andras Lasso and Franklin King at
  PerkLab, Queen's University and was supported through the Applied Cancer
  Research Unit program of Cancer Care Ontario with funds provided by the
  Ontario Ministry of Health and Long-Term Care.

==============================================================================*/


#ifndef __qSlicerTransformsModuleWidgetsPlugin_h
#define __qSlicerTransformsModuleWidgetsPlugin_h

#include "vtkSlicerConfigure.h" // For Slicer_HAVE_QT5

// Qt includes
#ifdef Slicer_HAVE_QT5
#include <QtUiPlugin/QDesignerCustomWidgetCollectionInterface>
#else
#include <QDesignerCustomWidgetCollectionInterface>
#endif

// Transforms includes
#include "qMRMLTransformDisplayNodeWidgetPlugin.h"
#include "qMRMLTransformInfoWidgetPlugin.h"

// \class Group the plugins in one library
class Q_SLICER_MODULE_TRANSFORMS_WIDGETS_PLUGINS_EXPORT qSlicerTransformsModuleWidgetsPlugin
  : public QObject
  , public QDesignerCustomWidgetCollectionInterface
{
  Q_OBJECT
#ifdef Slicer_HAVE_QT5
  Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QDesignerCustomWidgetCollectionInterface")
#endif
  Q_INTERFACES(QDesignerCustomWidgetCollectionInterface);

public:
  QList<QDesignerCustomWidgetInterface*> customWidgets() const
    {
    QList<QDesignerCustomWidgetInterface *> plugins;
    plugins << new qMRMLTransformDisplayNodeWidgetPlugin;
    plugins << new qMRMLTransformInfoWidgetPlugin;

    return plugins;
    }
};

#endif
