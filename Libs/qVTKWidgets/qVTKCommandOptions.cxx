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

// qCTK includes
#include <qCTKUtils.h>

// VTKSYS includes
#include <vtksys/CommandLineArguments.hxx>

// QT includes
#include <QDebug>
#include <QString>
#include <QLatin1String>
#include <QSettings>
#include <QHash>
#include <QList>
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
                                                       
  /// Since vtksys::CommandLineArguments will only update char*, returns
  /// a valid char* pointer that it could use.
  char** mapQStringPtrToCharPtr(QString* qStringPtr);

  /// Since vtksys::CommandLineArguments will only update std::vector<std::string>*, returns
  /// a valid std::vector<std::string>* pointer that it could use.
  std::vector<std::string>* mapQStringListPtrToStringVectorPtr(QStringList* qStringListPtr);

  /// If required, sync the updated char* with the corresponding QString*
  void syncQStringPtrWithCharPtr();

  /// If required, sync the updated std::vector<std::string> with the corresponding QStringList*
  void syncQStringListPtrWithStringVectorPtr();

  vtksys::CommandLineArguments   CMD;
  QString                        UnknownArgument;
  QString                        ErrorMessage;
  bool                           HelpSelected;
  bool                           DisableSettings;
  int                            Argc;
  char**                         Argv;
  QSettings*                     Settings;
  int                            ProcessType; // GUI, Batch, Daemon, ...
  
  QHash<QString*, char**>                         QStringPointerToCharPointerMap;
  QHash<QStringList*, std::vector<std::string>*>  QStringListPointerToStringVectorPointerMap;
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
  foreach (QString* qStringPtr, this->QStringPointerToCharPointerMap.keys())
    {
    delete this->QStringPointerToCharPointerMap[qStringPtr];
    }
  foreach (QStringList* qStringListPtr, this->QStringListPointerToStringVectorPointerMap.keys())
    {
    delete this->QStringListPointerToStringVectorPointerMap[qStringListPtr];
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
char** qVTKCommandOptionsPrivate::mapQStringPtrToCharPtr(QString* qStringPtr)
{
  Q_ASSERT(!this->QStringPointerToCharPointerMap.contains(qStringPtr));
  char** charPtr = new char*; // Create a new pointer
  *charPtr = 0; // Initialize to 0
  this->QStringPointerToCharPointerMap[qStringPtr] = charPtr;
  return charPtr;
}

// --------------------------------------------------------------------------
std::vector<std::string>*
qVTKCommandOptionsPrivate::mapQStringListPtrToStringVectorPtr(QStringList* qStringListPtr)
{
  Q_ASSERT(!this->QStringListPointerToStringVectorPointerMap.contains(qStringListPtr));
  std::vector<std::string>* vectorPtr = new std::vector<std::string>(); // Create a new vector
  this->QStringListPointerToStringVectorPointerMap[qStringListPtr] = vectorPtr;
  return vectorPtr;
}

// --------------------------------------------------------------------------
void qVTKCommandOptionsPrivate::syncQStringPtrWithCharPtr()
{
  foreach(QString* qStringPtr, this->QStringPointerToCharPointerMap.keys())
    {
    char** charPtr = this->QStringPointerToCharPointerMap[qStringPtr];
    Q_ASSERT(charPtr);
    // Update QString only if the content pointed by charPtr is valid
    if (*charPtr)
      {
      qStringPtr->clear();
      qStringPtr->append(QLatin1String(*charPtr));
      }
    }
}

// --------------------------------------------------------------------------
void qVTKCommandOptionsPrivate::syncQStringListPtrWithStringVectorPtr()
{
  foreach(QStringList* qStringListPtr, this->QStringListPointerToStringVectorPointerMap.keys())
    {
    std::vector<std::string>* vectorPtr =
      this->QStringListPointerToStringVectorPointerMap[qStringListPtr];
    Q_ASSERT(vectorPtr);
    // Update QString only if vectorPtr is not empty
    if (vectorPtr->size() > 0)
      {
      qStringListPtr->clear();
      QStringList convertedVector;
      qCTKUtils::stlVectorToQList(*vectorPtr, convertedVector);
      qStringListPtr->append(convertedVector);
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
void qVTKCommandOptions::printAdditionalInfo()
{
  QCTK_D(qVTKCommandOptions);
  qDebug() << "qVTKCommandOptions:" << this << endl
           << " HelpSelected:" << this->helpSelected() << endl
           << " DisableSettings:" << d->DisableSettings;
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

  d->syncQStringPtrWithCharPtr();
  d->syncQStringListPtrWithStringVectorPtr();
  
  return res1 && res2;
}

//----------------------------------------------------------------------------
QStringList qVTKCommandOptions::remainingArguments()
{
  QCTK_D(qVTKCommandOptions);
  QStringList tmp; 
  for(int i=0; i < d->Argc; ++i)
    {
    tmp << d->Argv[i]; 
    }
  return tmp;
}

//----------------------------------------------------------------------------
void qVTKCommandOptions::remainingArguments(int* argc, char*** argv)
{
  QCTK_D(qVTKCommandOptions);
  *argc = d->Argc;
  *argv = d->Argv;
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
    *var = d->Settings->value(QLatin1String(longarg+2), defaultValue).toBool();
    
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
void qVTKCommandOptions::addArgument(const char* longarg, const char* shortarg, QString* var,
                                    const char* arghelp, const QString& defaultValue, int type)
{
  QCTK_D(qVTKCommandOptions);
  *var = d->Settings->value(QLatin1String(longarg+2), defaultValue).toString();
  
  if(type & qVTKCommandOptions::QSETTINGS_ONLY)
    {
    return;
    }
  
  if(type & d->ProcessType || type == qVTKCommandOptions::ALL)
    {
    char ** charstar = d->mapQStringPtrToCharPtr(var);
    typedef vtksys::CommandLineArguments argT;
    d->CMD.AddArgument(longarg, argT::EQUAL_ARGUMENT, charstar, arghelp);
    if ( shortarg )
      {
      d->CMD.AddArgument(shortarg, argT::EQUAL_ARGUMENT, charstar, longarg);
      }
    }
}

//----------------------------------------------------------------------------
void qVTKCommandOptions::addArgument(const char* longarg, const char* shortarg,
                                     QStringList* var, const char* arghelp,
                                     const QStringList& defaultValue, int type)
{
  QCTK_D(qVTKCommandOptions);
  *var = d->Settings->value(QLatin1String(longarg+2), defaultValue).toStringList();
  
  if(type & qVTKCommandOptions::QSETTINGS_ONLY)
    {
    return;
    }
    
  if(type & d->ProcessType || type == qVTKCommandOptions::ALL)
    {
    std::vector<std::string>* vectorPtr = d->mapQStringListPtrToStringVectorPtr(var);
    typedef vtksys::CommandLineArguments argT;
    d->CMD.AddArgument(longarg, argT::MULTI_ARGUMENT, vectorPtr, arghelp);
    if (shortarg)
      {
      d->CMD.AddArgument(shortarg, argT::MULTI_ARGUMENT, vectorPtr, longarg);
      }
    }
}

//----------------------------------------------------------------------------
void qVTKCommandOptions::addArgument(const char* longarg, const char* shortarg, int* var,
                                     const char* arghelp, int defaultValue, int type)
{
  QCTK_D(qVTKCommandOptions);
  *var = d->Settings->value(QLatin1String(longarg+2), defaultValue).toInt();
  
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
int qVTKCommandOptions::indexOfLastParsedArgument()
{
  QCTK_D(qVTKCommandOptions);
  return d->CMD.GetLastArgument();
}
