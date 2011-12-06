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

// Qt includes
#include <QApplication>
#include <QTimer>

// Volumes includes
#include "qSlicerVolumesIOOptionsWidget.h"

//-----------------------------------------------------------------------------
int qSlicerVolumesIOOptionsWidgetTest1( int argc, char * argv[] )
{
  QApplication app(argc, argv);

  qSlicerVolumesIOOptionsWidget optionsWidget;

  optionsWidget.setFileName("mylabelmap-seg.nrrd");
  if (!optionsWidget.properties()["labelmap"].toBool())
    {
    std::cerr << "Must be a labelmap" << std::endl;
    return EXIT_FAILURE;
    }

  optionsWidget.setFileName("./mylabelmap_seg_1.nrrd");
  if (!optionsWidget.properties()["labelmap"].toBool())
    {
    std::cerr << "Must be a labelmap" << std::endl;
    return EXIT_FAILURE;
    }

  optionsWidget.setFileName("./segment.nrrd");
  if (optionsWidget.properties()["labelmap"].toBool())
    {
    std::cerr << "Not a labelmap" << std::endl;
    return EXIT_FAILURE;
    }

  optionsWidget.show();

  if (argc < 2 || QString(argv[2]) != "-I")
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }
  return app.exec();
}
