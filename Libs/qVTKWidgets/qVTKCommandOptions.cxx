/*=========================================================================
  
  Program:   ParaView
  Module:    $RCSfile: vtkCommandOptions.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "qVTKCommandOptions.h"

// VTKSYS includes
#include <vtksys/CommandLineArguments.hxx>

// QT includes
#include <QDebug>
#include <QString>
#include <QLatin1String>
#include <QSettings>
#include <QHash>
#include <QStringList>

// --------------------------------------------------------------------------
class qVTKCommandOptionsPrivate: public qCTKPrivate<qVTKCommandOptions>
{
public:
  typedef qVTKCommandOptionsPrivate Self;
  qVTKCommandOptionsPrivate();
  ~qVTKCommandOptionsPrivate();

  void cleanArgcArgv();

  /// Callback executed when an unknown arguments is parsed
  static int unknownArgumentHandler(const char* argument, void* call_data);

  /// Callback executed when a deprecated arguments is parsed
  static int deprecatedArgumentHandler(const char* argument, const char* value, void* call_data);

  /// Read options from the settings.
  void readBooleanOption(const char* longarg, bool& var, bool defaultValue);
  void readOption(const char* longarg, int& var, int defaultValue);
  void readOption(const char* longarg, QString& var, const QString& defaultValue);
                                                       
  /// Since vtksys::CommandLineArguments will only update char*, returns
  /// a valid char* pointer that it could use.
  char** mapQStringStarToCharStar(QString* qstringstar);

  /// Sync the updated char* with the corresponding QString*
  void syncQStringStarWithCharStar();

  vtksys::CommandLineArguments  CMD;
  QString                       UnknownArgument;
  QString                       ErrorMessage;
  bool                          HelpSelected;
  bool                          DisableSettings;
  int                           Argc;
  char**                        Argv;
  QSettings*                    Settings;
  int                           ProcessType; // GUI, Batch, Daemon, ...
  QHash<QString*, char**>       QStringStarToCharStarMap;
};

//-----------------------------------------------------------------------------
// qVTKCommandOptionsPrivate methods

// --------------------------------------------------------------------------
qVTKCommandOptionsPrivate::qVTKCommandOptionsPrivate()
{
  this->CMD.SetUnknownArgumentCallback(qVTKCommandOptionsPrivate::unknownArgumentHandler);
  this->CMD.SetClientData(this);

  this->Argc = 0;
  this->Argv = 0; 
  this->HelpSelected = false;
  this->DisableSettings = false;
}

// --------------------------------------------------------------------------
qVTKCommandOptionsPrivate::~qVTKCommandOptionsPrivate()
{
  this->cleanArgcArgv();
  foreach (QString* qstringchar, this->QStringStarToCharStarMap.keys())
    {
    delete this->QStringStarToCharStarMap[qstringchar];
    }
}

// --------------------------------------------------------------------------
void qVTKCommandOptionsPrivate::cleanArgcArgv()
{
  int cc;
  if (this->Argv)
    {
    for (cc = 0; cc < this->Argc; cc++)
      {
      delete [] this->Argv[cc];
      }
    delete [] this->Argv;
    this->Argv = 0;
    }
}

//----------------------------------------------------------------------------
int qVTKCommandOptionsPrivate::unknownArgumentHandler(const char* argument,
                                                      void* call_data)
{
  qVTKCommandOptionsPrivate* self =
    static_cast<qVTKCommandOptionsPrivate*>(call_data);
  if (self)
    {
    self->UnknownArgument = QString::fromLatin1(argument);
    return self->qctk_p()->wrongArgument(argument);
    }
  return 0;
}

//----------------------------------------------------------------------------
int qVTKCommandOptionsPrivate::deprecatedArgumentHandler(const char* argument,
                                                         const char* , void* call_data)
{
  //qDebug() << "UnknownArgumentHandler: " << argument;
  qVTKCommandOptionsPrivate* self = static_cast<qVTKCommandOptionsPrivate*>(call_data);
  if (self)
    {
    return self->qctk_p()->deprecatedArgument(argument);
    }
  return 0;
}

// --------------------------------------------------------------------------
void qVTKCommandOptionsPrivate::readBooleanOption(const char* longarg, bool& var,
                                                      bool defaultValue)
{
  Q_ASSERT(this->Settings);
  var = this->Settings->value(QLatin1String(longarg+2), defaultValue).toBool();
}

// --------------------------------------------------------------------------
void qVTKCommandOptionsPrivate::readOption(const char* longarg, int& var, int defaultValue)
{
  Q_ASSERT(this->Settings);
  var = this->Settings->value(QLatin1String(longarg+2), defaultValue).toInt();
}

// --------------------------------------------------------------------------
void qVTKCommandOptionsPrivate::readOption(const char* longarg, QString& var,
                                               const QString& defaultValue)
{
  Q_ASSERT(this->Settings);
  var = this->Settings->value(QLatin1String(longarg+2), defaultValue).toString();
}

// --------------------------------------------------------------------------
char** qVTKCommandOptionsPrivate::mapQStringStarToCharStar(QString* qstringstar)
{
  Q_ASSERT(!QStringStarToCharStarMap.contains(qstringstar));
  char** charstar = new char*; // Create a new pointer
  *charstar = 0; // Initialize to 0
  QStringStarToCharStarMap[qstringstar] = charstar;
  return charstar; 
}

// --------------------------------------------------------------------------
void qVTKCommandOptionsPrivate::syncQStringStarWithCharStar()
{
  foreach(QString* qstringstar, this->QStringStarToCharStarMap.keys())
    {
    char** charstar = this->QStringStarToCharStarMap[qstringstar];
    Q_ASSERT(charstar);
    // Update QString only if the content pointed by charstar is valid
    if (*charstar)
      {
      qstringstar->clear();
      qstringstar->append(QLatin1String(*charstar));
      }
    }
}

//-----------------------------------------------------------------------------
// qVTKCommandOptions methods

// --------------------------------------------------------------------------
qVTKCommandOptions::qVTKCommandOptions(QSettings* settings)
{
  QCTK_INIT_PRIVATE(qVTKCommandOptions);
  Q_ASSERT(settings);
  QCTK_D(qVTKCommandOptions);
  d->Settings = settings;
}

//----------------------------------------------------------------------------
qVTKCommandOptions::~qVTKCommandOptions()
{
}

//----------------------------------------------------------------------------
QCTK_GET_CXX(qVTKCommandOptions, QString, errorMessage, ErrorMessage);
QCTK_GET_CXX(qVTKCommandOptions, QString, unknownArgument, UnknownArgument);
QCTK_GET_CXX(qVTKCommandOptions, bool, helpSelected, HelpSelected);
QCTK_GET_CXX(qVTKCommandOptions, bool, disableSettings, DisableSettings);

//----------------------------------------------------------------------------
QCTK_GET_CXX(qVTKCommandOptions, int, processType, ProcessType);
QCTK_SET_CXX(qVTKCommandOptions, int, setProcessType, ProcessType);

//----------------------------------------------------------------------------
void qVTKCommandOptions::initialize()
{
  
}

//----------------------------------------------------------------------------
QString qVTKCommandOptions::help()
{
  QCTK_D(qVTKCommandOptions);
  d->CMD.SetLineLength(300);
  return QLatin1String(d->CMD.GetHelp());
}

//----------------------------------------------------------------------------
bool qVTKCommandOptions::postProcess(int, const char* const*)
{
  return true;
}

//----------------------------------------------------------------------------
bool qVTKCommandOptions::parse(int argc, const char* const argv[])
{
  QCTK_D(qVTKCommandOptions);
  d->CMD.Initialize(argc, argv);
  this->initialize();
  this->addBooleanArgument("--help", "/?", &d->HelpSelected,
                           "Displays available command line arguments.");

  this->addBooleanArgument("--disable-settings", 0, &d->DisableSettings,
                           "Start application ignoring user settings.");

  // Get options from the command line
  bool res1 = d->CMD.Parse();
  bool res2 = this->postProcess(argc, argv);
  //qDebug() << "Res1:" << res1 << ", Res2:" << res2;
  d->cleanArgcArgv();
  d->CMD.GetRemainingArguments(&d->Argc, &d->Argv);

  if (d->DisableSettings)
    {
    this->disableCurrentSettings();
    }

  d->syncQStringStarWithCharStar();
  return res1 && res2;
}

//----------------------------------------------------------------------------
void qVTKCommandOptions::addDeprecatedArgument(const char* longarg, const char* shortarg,
                                               const char* arghelp, int type)
{
  QCTK_D(qVTKCommandOptions);

  // If it is for settings or not for the current process do nothing
  if((type & qVTKCommandOptions::QSETTINGS_ONLY) ||
     !(type & d->ProcessType || type == qVTKCommandOptions::ALL))
    {
    return;
    }
  
  // Add a callback for the deprecated argument handling
  d->CMD.AddCallback(longarg, vtksys::CommandLineArguments::NO_ARGUMENT,
                     qVTKCommandOptionsPrivate::deprecatedArgumentHandler, this, arghelp);
  if(shortarg)
    {
    d->CMD.AddCallback(shortarg, vtksys::CommandLineArguments::NO_ARGUMENT,
                       qVTKCommandOptionsPrivate::deprecatedArgumentHandler, this, arghelp);
    }
}

//----------------------------------------------------------------------------
bool qVTKCommandOptions::deprecatedArgument(const char* argument)
{
  QCTK_D(qVTKCommandOptions);
  d->ErrorMessage = QString("  %1").arg(d->CMD.GetHelp(argument));
  return false;
}

//----------------------------------------------------------------------------
bool qVTKCommandOptions::wrongArgument(const char* argument)
{
  Q_UNUSED(argument);
  return false;
}

//----------------------------------------------------------------------------
void qVTKCommandOptions::addBooleanArgument(const char* longarg, const char* shortarg,
                                            bool* var, const char* arghelp,
                                            bool defaultValue, int type)
{
  QCTK_D(qVTKCommandOptions);

  // Attempt to read from settings only if longarg is different from '--disable-settings'.
  if (QLatin1String(longarg) != "--disable-settings")
    {
    d->readBooleanOption(longarg, *var, defaultValue);
    
    if(type & qVTKCommandOptions::QSETTINGS_ONLY)
      {
      return;
      }
    }

  // If the process type matches then add the argument to the command line
  if(type & d->ProcessType || type == qVTKCommandOptions::ALL)
    {
    d->CMD.AddBooleanArgument(longarg, var, arghelp);
    if (shortarg)
      {
      d->CMD.AddBooleanArgument(shortarg, var, longarg);
      }
    }
}

//----------------------------------------------------------------------------
void qVTKCommandOptions::addArgument(const char* longarg, const char* shortarg, int* var,
                                    const char* arghelp, int defaultValue, int type)
{
  QCTK_D(qVTKCommandOptions);

  d->readOption(longarg, *var, defaultValue);
  if(type & qVTKCommandOptions::QSETTINGS_ONLY)
    {
    return;
    }
    
  if(type & d->ProcessType || type == qVTKCommandOptions::ALL)
    {
    typedef vtksys::CommandLineArguments argT;
    d->CMD.AddArgument(longarg, argT::EQUAL_ARGUMENT, var, arghelp);
    if (shortarg)
      {
      d->CMD.AddArgument(shortarg, argT::EQUAL_ARGUMENT, var, longarg);
      }
    }
}

//----------------------------------------------------------------------------
void qVTKCommandOptions::addArgument(const char* longarg, const char* shortarg, QString* var,
                                    const char* arghelp, const QString& defaultValue, int type)
{
  QCTK_D(qVTKCommandOptions);

  d->readOption(longarg, *var, defaultValue);
  if(type & qVTKCommandOptions::QSETTINGS_ONLY)
    {
    return;
    }
  
  if(type & d->ProcessType || type == qVTKCommandOptions::ALL)
    {
    char ** charstar = d->mapQStringStarToCharStar(var);
    typedef vtksys::CommandLineArguments argT;
    d->CMD.AddArgument(longarg, argT::EQUAL_ARGUMENT, charstar, arghelp);
    if ( shortarg )
      {
      d->CMD.AddArgument(shortarg, argT::EQUAL_ARGUMENT, charstar, longarg);
      }
    }
}

//----------------------------------------------------------------------------
int qVTKCommandOptions::indexOfLastParsedArgument()
{
  QCTK_D(qVTKCommandOptions);
  return d->CMD.GetLastArgument();
}
