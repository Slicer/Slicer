/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

#ifndef __qSlicerDICOMLibModuleWidgetsPlugin_h
#define __qSlicerDICOMLibModuleWidgetsPlugin_h

// Qt includes
#include <QtUiPlugin/QDesignerCustomWidgetCollectionInterface>

// DICOMLib includes
#include "qSlicerDICOMTagEditorWidgetPlugin.h"

// \class Group the plugins in one library
class Q_SLICER_MODULE_DICOMLIB_WIDGETS_PLUGINS_EXPORT qSlicerDICOMLibModuleWidgetsPlugin
  : public QObject
  , public QDesignerCustomWidgetCollectionInterface
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QDesignerCustomWidgetCollectionInterface")
  Q_INTERFACES(QDesignerCustomWidgetCollectionInterface);

public:
  QList<QDesignerCustomWidgetInterface*> customWidgets() const override
    {
    QList<QDesignerCustomWidgetInterface *> plugins;
    plugins << new qSlicerDICOMTagEditorWidgetPlugin;
    return plugins;
    }
};

#endif
