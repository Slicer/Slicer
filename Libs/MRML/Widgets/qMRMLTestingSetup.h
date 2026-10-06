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

==============================================================================*/

#ifndef __qMRMLTestingSetup_h
#define __qMRMLTestingSetup_h

// Qt includes
#include <QString>
#include <QStringList>

#include "qMRMLWidgetsExport.h"

/// \brief Sets up a widget test based on the command-line arguments of the test function.
///
/// The constructor parses the arguments and applies the testing settings that they specify,
/// such as the QtTesting event playback delay (this changes application-wide testing state).
///
/// Options are recognized anywhere on the command line and they are not included in positionalArguments():
/// - `-I`: interactive mode. The test should not quit or play back recorded events automatically.
/// - `--event-playback-delay <ms>`: delay between played back QtTesting events, in milliseconds.
///   Zero delay makes tests run faster, while the QtTesting default delay allows watching the playback.
///
/// Example:
/// \code
/// int qMRMLSliceWidgetEventTranslatorPlayerTest1(int argc, char* argv[])
/// {
///   QApplication app(argc, argv);
///   qMRMLTestingSetup testingSetup(argc, argv);
///   CHECK_BOOL(testingSetup.isValid(), true);
///   QString sourceDirectory = testingSetup.positionalArgument(0);
///   ...
///   if (!testingSetup.interactive())
///   {
///     QTimer::singleShot(0, &etpWidget, SLOT(play()));
///   }
/// \endcode
class QMRML_WIDGETS_EXPORT qMRMLTestingSetup
{
public:
  /// Parse arguments of a test function and apply the specified testing settings.
  /// The first argument (argv[0]) is the test name and it is ignored.
  /// If the arguments are invalid then the error is logged and settings are not applied.
  qMRMLTestingSetup(int argc, char* argv[]);

  /// Returns false if an option has a missing or invalid value. See errorString() for details.
  bool isValid() const;
  QString errorString() const;

  /// True if interactive mode is requested (`-I`).
  bool interactive() const;

  /// Arguments that are not testing options, in their original order.
  QStringList positionalArguments() const;
  /// Returns the positional argument at the given index, or defaultValue if there is no such argument.
  QString positionalArgument(int index, const QString& defaultValue = QString()) const;

  /// Delay between played back QtTesting events, in milliseconds.
  /// Returns -1 if the delay is not specified.
  int eventPlaybackDelay() const;

protected:
  void parse(const QStringList& arguments);
  /// Apply testing settings that are specified in the options (currently the QtTesting event playback delay).
  /// Settings that are not specified are not changed.
  void applySettings() const;

  bool Interactive{ false };
  int EventPlaybackDelay{ -1 };
  QStringList PositionalArguments;
  QString ErrorString;
};

#endif
