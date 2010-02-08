/*=========================================================================

  Library:   qCTK

  Copyright (c) Kitware Inc. 
  All rights reserved.
  Distributed under a BSD License. See LICENSE.txt file.

  This software is distributed "AS IS" WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the above copyright notice for more information.

=========================================================================*/

#include "qVTKWidgetsCxxTestsConfigure.h"

// qVTK includes
#include "qVTKCommandOptions.h"

// QT includes
#include <QCoreApplication>
#include <QSettings>
#include <QStringList>
#include <QList>
#include <QFile>
#include <QDebug>

// STL includes
#include <stdlib.h>
#include <iostream>

//-----------------------------------------------------------------------------
namespace
{
class qTestVTKCommandOptions : public qVTKCommandOptions
{
public:
  typedef qVTKCommandOptions Superclass;
  qTestVTKCommandOptions(QSettings* _settings):Superclass(_settings)
    {
    this->AwesomeTestEnabled = false;
    this->CurrentSettingsDisabled = false;
    this->TestIfCommandLineOverwriteSettings = false;
    this->TestIfDisableSettingsFlagWorks = false;
    this->GenerateTestData = false;
    }
  virtual ~qTestVTKCommandOptions(){}

  // Added as public member for testing purposes ...
  bool          AwesomeTestEnabled;
  bool          CurrentSettingsDisabled;
  bool          TestIfCommandLineOverwriteSettings;
  bool          TestIfDisableSettingsFlagWorks;
  bool          GenerateTestData;
  QString       WrongArgument;
  QStringList   MyListOfStrings;
  QString       MyString; 
  

  // Convenient method allowing to generate a INI file
  void generateIniFile()
    {
    std::cout << "Saving test data:" << QTESTVTKCOMMANDOPTIONS_SETTINGS << std::endl;
    
    QSettings newSettings(QTESTVTKCOMMANDOPTIONS_SETTINGS, QSettings::IniFormat);

    newSettings.clear();
    
    newSettings.setValue("enable-awesome-test", QVariant(true));

    newSettings.setValue("my-string", QVariant("Testing rocks !"));
    
    QStringList mystringlist;
    mystringlist << "Testing"; mystringlist << " is "; mystringlist << "awesome !";
    newSettings.setValue("my-list-of-strings", QVariant(mystringlist));
    }

  virtual void printAdditionalInfo()
    {
    Superclass::printAdditionalInfo();
    qDebug() << "qTestVTKCommandOptions:" << this << endl
             << " CurrentSettingsDisabled:" << this->CurrentSettingsDisabled << endl
             << " WrongArgument:" << this->WrongArgument << endl
             << " TestIfCommandLineOverwriteSettings:" << this->TestIfCommandLineOverwriteSettings << endl
             << " TestIfDisableSettingsFlagWorks:" << this->TestIfDisableSettingsFlagWorks << endl
             << " GenerateTestData:" << this->GenerateTestData << endl
             << " SettingsFile:" << QTESTVTKCOMMANDOPTIONS_SETTINGS << endl
             << " MyListOfStrings:" << this->MyListOfStrings << endl; 
    
    }
  
protected:

  /// Initialize arguments
  virtual void initialize()
    {
    this->Superclass::initialize();

    this->addBooleanArgument("--enable-awesome-test", "-eat", &this->AwesomeTestEnabled,
                             "Enables awesome test.");

    this->addBooleanArgument("--test-if-commandline-overwrite-liststring-settings", 0,
                             &this->TestIfCommandLineOverwriteSettings,
                             "Should we test if QListString setting is overwriten by the "
                             "corresponding command line argument.");

    this->addBooleanArgument("--test-if-disablesettings-flag-works", 0,
                             &this->TestIfDisableSettingsFlagWorks,
                             "Indicates if '--disable-settings' flags should be tested.");

    this->addBooleanArgument("--generate-test-data", 0, &this->GenerateTestData,
                             "Generates test data and save it to the file specified using '--settings-file'");

    this->addArgument("--my-list-of-strings", "-mlos", &this->MyListOfStrings,
                      "My list of strings.");

    this->addArgument("--my-string", "-ms", &this->MyString,
                      "My string.");
                      
    }

  // Disable current settings
  virtual void disableCurrentSettings()
    {
    this->CurrentSettingsDisabled = true;
    }

  /// This method is called when wrong argument is found. If it returns False, then
  /// the parsing will fail.
  virtual bool wrongArgument(const char* argument)
    {
    this->WrongArgument = QLatin1String(argument);
    return false;
    }
    
};
}

//-----------------------------------------------------------------------------
int qVTKCommandOptionsTest1(int argc, char * argv [] )
{
  QCoreApplication app(argc, argv);

  app.setOrganizationName("qVTKWidgetsCxxTests");
  app.setApplicationName("qVTKCommandOptionsTest1");

  // Check if settings file exists
  if (!QFile::exists(QTESTVTKCOMMANDOPTIONS_SETTINGS))
    {
    std::cerr << "Problem with configuration of the test - "
              << "Settings file doesn't exist: [" << QTESTVTKCOMMANDOPTIONS_SETTINGS << "]" << std::endl;
    return EXIT_FAILURE;
    }

  //std::cout << "QTESTVTKCOMMANDOPTIONS_SETTINGS:" << QTESTVTKCOMMANDOPTIONS_SETTINGS << std::endl;

  QSettings setting(QTESTVTKCOMMANDOPTIONS_SETTINGS, QSettings::IniFormat);

  qTestVTKCommandOptions commandOptions(&setting);

  if (!commandOptions.parse(argc, argv))
    {
    commandOptions.printAdditionalInfo();
    std::cerr << "Problem parsing command line arguments.  Try with --help." << std::endl;
    return EXIT_FAILURE;
    }

  if (commandOptions.helpSelected())
    {
    std::cout << commandOptions.help().toStdString() << std::endl;
    return EXIT_SUCCESS;
    }
    
  // Convenient flags allowing to generate test data
  if (commandOptions.GenerateTestData)
    {
    commandOptions.generateIniFile();
    return EXIT_SUCCESS;
    }

  // Test if '--disable-settings' flag works
  if (commandOptions.TestIfDisableSettingsFlagWorks)
    {
    if (!commandOptions.CurrentSettingsDisabled)
      {
      commandOptions.printAdditionalInfo();
      std::cerr << "Error with parse() function - Settings should be disabled" << std::endl;
      return EXIT_FAILURE;
      }
    return EXIT_SUCCESS;
    }

  // Test case when a wrong argument is a parsing error
//   if (commandOptions.TestIfDisableSettingsFlagWorks)
//     {
//     if (!commandOptions.CurrentSettingsDisabled)
//       {
//       std::cerr << "Error with parse() function - Settings should be disabled" << std::endl;
//       return EXIT_FAILURE;
//       }
//     return EXIT_SUCCESS;
//     }

  // Since MyListOfStringss should have been updated using the settings file,
  // let's check if MyListOfStrings has the expected size
  if (commandOptions.MyListOfStrings.size() != 3)
    {
    commandOptions.printAdditionalInfo();
    std::cerr << "Error with parse() function - "
              << "MyListOfStrings expected size is 3 - Current size: "
              << commandOptions.MyListOfStrings.size() << std::endl;
    for(int i=0; i < commandOptions.MyListOfStrings.size(); ++i)
      {
      std::cerr << "MyListOfStrings[" << i << "] => ["
                << qPrintable(commandOptions.MyListOfStrings[i]) << "]" << std::endl;
      }
    return EXIT_FAILURE;
    }
      
  // Test if command line argument overwrite settings
  if (commandOptions.TestIfCommandLineOverwriteSettings)
    {
    if (commandOptions.MyListOfStrings[0] != "Testing with" ||
        commandOptions.MyListOfStrings[1] != " cmake is " ||
        commandOptions.MyListOfStrings[2] != "super awesome !")
      {
      commandOptions.printAdditionalInfo();
      std::cerr << "Error with parse() function - "
                << "Content of MyListOfStrings is incorrect" << std::endl
                << "expected[0] => [Testing with]" << std::endl
                << "expected[1] => [ cmake is ]" << std::endl
                << "expected[2] => [super awesome !]" << std::endl;
      for(int i=0; i < commandOptions.MyListOfStrings.size(); ++i)
        {
        std::cerr << "MyListOfStrings[" << i << "] => ["
                  << qPrintable(commandOptions.MyListOfStrings[i]) << "]" << std::endl;
        }
      return EXIT_FAILURE;
      }
    // Check MyString value 
    if (commandOptions.MyString != "CMake rocks !")
      {
      commandOptions.printAdditionalInfo();
      std::cerr << "Error with parse() function - Content of MyString is incorrect" << std::endl
                << "expected value is [Testing rocks !]" << std::endl
                << "current value is [" << qPrintable(commandOptions.MyString) << "]" << std::endl;
      }
    return EXIT_SUCCESS;
    }
  else
    {
    if (commandOptions.MyListOfStrings[0] != "Testing" ||
        commandOptions.MyListOfStrings[1] != " is " ||
        commandOptions.MyListOfStrings[2] != "awesome !")
      {
      commandOptions.printAdditionalInfo();
      std::cerr << "Error with parse() function - "
                << "Content of MyListOfStrings is incorrect" << std::endl
                << "expected[0] => [Testing]" << std::endl
                << "expected[1] => [ is ]" << std::endl
                << "expected[2] => [awesome !]" << std::endl;
      for(int i=0; i < commandOptions.MyListOfStrings.size(); ++i)
        {
        std::cerr << "MyListOfStrings[" << i << "] => ["
                  << qPrintable(commandOptions.MyListOfStrings[i]) << "]" << std::endl;
        }
      return EXIT_FAILURE;
      }
    // Check MyString value 
    if (commandOptions.MyString != "Testing rocks !")
      {
      commandOptions.printAdditionalInfo();
      std::cerr << "Error with parse() function - Content of MyString is incorrect" << std::endl
                << "expected value is [CMake rocks !]" << std::endl
                << "current value is [" << qPrintable(commandOptions.MyString) << "]" << std::endl;
      }
    }

  if (commandOptions.CurrentSettingsDisabled)
    {
    commandOptions.printAdditionalInfo();
    std::cerr << "Error with parse() function - Settings should *NOT* be disabled" << std::endl;
    return EXIT_FAILURE;
    }

  if (!commandOptions.WrongArgument.isEmpty())
    {
    commandOptions.printAdditionalInfo();
    std::cerr << "Error with parse() function - "
              << "WrongArgument should *NOT* be set to ["
              << qPrintable(commandOptions.WrongArgument) << "]" << std::endl;
    return EXIT_FAILURE;
    }

  if (!commandOptions.AwesomeTestEnabled)
    {
    commandOptions.printAdditionalInfo();
    std::cerr << "Error with parse() function - AwesomeTestEnabled should be True" << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
