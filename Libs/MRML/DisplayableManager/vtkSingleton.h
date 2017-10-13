/*=========================================================================

  Library:   CTK

  Copyright (c) 2010  Kitware Inc.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.commontk.org/LICENSE

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

=========================================================================*/

#ifndef __vtkSingleton_h
#define __vtkSingleton_h

/// @file
///
/// @defgroup vtkSingleton Singleton definition and declaration helpers
///
/// See http://www.parashift.com/c++-faq-lite/ctors.html#faq-10.12
/// and http://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Nifty_Counter
///
/// Inspired from VTK/Utilities/kwsys/SystemTools class
/// @{

//-----------------------------------------------------------------------------
/// Should be included as a class protected member
///
#define VTK_SINGLETON_DECLARE(NAME)       \
typedef NAME Self;                        \
static NAME* Instance;                    \
static void classInitialize();            \
static void classFinalize();              \
friend class NAME##Initialize;

//-----------------------------------------------------------------------------
/// Help macro allowing to declare the utility class to make sure
/// NAME is initialized before it is used.
///
/// Should be added at the bottom of the header file, after the class declaration
///
/// The instance (<code>NAME\#\#Initializer</code>) will show up in any translation unit
/// that uses NAME.  It will make sure NAME is initialized before it is used.
///
#define VTK_SINGLETON_DECLARE_INITIALIZER(EXPORT_DIRECTIVE,NAME)   \
class EXPORT_DIRECTIVE NAME##Initialize                            \
{                                                                  \
public:                                                            \
  typedef NAME##Initialize Self;                                   \
                                                                   \
  NAME##Initialize();                                              \
  ~NAME##Initialize();                                             \
private:                                                           \
  static unsigned int Count;                                       \
};                                                                 \
                                                                   \
static NAME##Initialize NAME##Initializer;


//-----------------------------------------------------------------------------
///
/// Implementation of <code>NAME\#\#Initialize</code> class.
///
/// Macro used by VTK_SINGLETON_DEFINE. See below.
///
/// \note <code>NAME\#\#Initialize::Count</code> and <code>NAME::Instance</code> Must NOT be initialized.
/// Default initialization to zero is necessary.
///
#define VTK_SINGLETON_INITIALIZER_CXX(NAME)      \
NAME##Initialize::NAME##Initialize()                \
{                                                   \
  if(++Self::Count == 1)                            \
    { NAME::classInitialize(); }                    \
}                                                   \
                                                    \
NAME##Initialize::~NAME##Initialize()               \
{                                                   \
  if(--Self::Count == 0)                            \
    { NAME::classFinalize(); }                      \
}                                                   \
                                                    \
unsigned int NAME##Initialize::Count;               \
NAME* NAME::Instance;

// TODO Would be great to have this macro working. that would avoid
// to duplicate the following lines over and over
//#define VTK_SINGLETON_GETINSTANCE_CXX(NAME)
/* Up the reference count so it behaves like New */
//NAME* NAME::New()
//{
//  NAME* instance = Self::GetInstance();
//  instance->Register(0);
//  return instance;
//}
//NAME* NAME::GetInstance()
//{
//  if(!Self::Instance)
//    {
    /* Try the factory first */
//    Self::Instance = (NAME*)vtkObjectFactory::CreateInstance(#NAME);
//
    /* if the factory did not provide one, then create it here*/
//    if(!Self::Instance)
//      {
      /* if the factory failed to create the object, */
      /* then destroy it now, as vtkDebugLeaks::ConstructClass */
      /* was called with "NAME", and not the real name of the class */
//#ifdef VTK_DEBUG_LEAKS
//      vtkDebugLeaks::DestructClass(#NAME);
//#endif
//      Self::Instance = new NAME;
//      }
//    }
//  return Self::Instance;
//}

//----------------------------------------------------------------------------
///
/// This should be added at the end of the CXX file
///
#define VTK_SINGLETON_CXX(NAME)                     \
void NAME::classInitialize()                        \
{                                                   \
  Self::Instance = NAME::GetInstance();             \
}                                                   \
                                                    \
void NAME::classFinalize()                          \
{                                                   \
  Self::Instance->Delete();                         \
  Self::Instance = 0;                               \
}                                                   \
                                                    \
VTK_SINGLETON_INITIALIZER_CXX(NAME)

/// @}

#endif //__vtkSingleton_h
