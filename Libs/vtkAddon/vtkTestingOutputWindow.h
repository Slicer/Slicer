/*==============================================================================

  Program: 3D Slicer

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#ifndef __vtkTestingOutputWindow_h
#define __vtkTestingOutputWindow_h

#include "vtkAddon.h"

#include "vtkObject.h"
#include "vtkOutputWindow.h"
#include "vtkLoggingMacros.h" // for vtkInfoWithoutObjectMacro

/// \brief VTK message output window class for automated testing.
///
/// This is a VTK output window class that is optimized to be used
/// in automated tests:
/// - counts error and warning messages between checkpoints
/// - forces VTK to always log to the console (on Windows the default
///   behavior is to show messages in a popup window)
///
/// A set of convenience macros are defined. Example:
///
///    // Initialize output window. All messages are logged to the console.
///    TESTING_OUTPUT_INIT();
///
///    ...perform operations that _must_not_ log error messages
///    TESTING_OUTPUT_ASSERT_WARNINGS_ERRORS(0);
///
///    TESTING_OUTPUT_ASSERT_WARNINGS_BEGIN();
///    ...perform operations that _must_ log error messages
///    TESTING_OUTPUT_ASSERT_WARNINGS_END();
///
class VTK_ADDON_EXPORT vtkTestingOutputWindow : public vtkOutputWindow
{
public:
  vtkTypeMacro(vtkTestingOutputWindow, vtkOutputWindow);
  static vtkTestingOutputWindow* New();
  
  // Gets a pointer to the singleton testing output window instance.
  // If the current VTK output window is not vtkTestingOutputWindow type then
  // it changes the output window to that.
  static vtkTestingOutputWindow* GetInstance();

  virtual void PrintSelf(ostream& os, vtkIndent indent);

  virtual void DisplayText(const char* text);
  virtual void DisplayErrorText(const char* text);
  virtual void DisplayWarningText(const char* text);
  virtual void DisplayGenericWarningText(const char* text); 
  virtual void DisplayDebugText(const char* text);

  // Sets number of warning and error messages to zero
  virtual void ResetNumberOfLoggedMessages();
  
  // Number of any logged messages
  vtkGetMacro(NumberOfLoggedMessages, int);
  vtkSetMacro(NumberOfLoggedMessages, int);
  
  // Number of logged warning or generic warning messages
  vtkGetMacro(NumberOfLoggedWarningMessages, int);
  vtkSetMacro(NumberOfLoggedWarningMessages, int);

  // Number of logged error messages
  vtkGetMacro(NumberOfLoggedErrorMessages, int);
  vtkSetMacro(NumberOfLoggedErrorMessages, int);
  
  // Returns the sum of warning and error messages logged
  int GetNumberOfLoggedWarningErrorMessages();
  
protected:
  vtkTestingOutputWindow(); 
  virtual ~vtkTestingOutputWindow(); 
  
  int NumberOfLoggedWarningMessages;
  int NumberOfLoggedErrorMessages;
  int NumberOfLoggedMessages;

private:
  vtkTestingOutputWindow(const vtkTestingOutputWindow&);  // Not implemented.
  void operator=(const vtkTestingOutputWindow&);  // Not implemented.
};


// Convenience macros:

/// Initializes logging to the tesing window
#define TESTING_OUTPUT_INIT() vtkTestingOutputWindow::GetInstance();

/// Resets all message counters to 0
#define TESTING_OUTPUT_RESET() vtkTestingOutputWindow::GetInstance()->ResetNumberOfLoggedMessages();

/// Exits with failure if the number of logged messages is not equal to the specified number
#define TESTING_OUTPUT_ASSERT_MESSAGES(expectedNumberOfMessages) \
  { \
  int actualNumberOfMessages = vtkTestingOutputWindow::GetInstance()->GetNumberOfLoggedMessages(); \
  if (actualNumberOfMessages != expectedNumberOfMessages) \
    { \
    std::cerr << "Assertion failed in " << __FILE__ << ":" << __LINE__ << " - expected " << expectedNumberOfMessages \
      << " messages, got " << actualNumberOfMessages << std::endl; \
    exit(EXIT_FAILURE); \
    } \
  };

/// Exits with failure if the number of logged warning messages is not equal to the specified number
#define TESTING_OUTPUT_ASSERT_WARNINGS(expectedNumberOfMessages) \
  { \
  int actualNumberOfMessages = vtkTestingOutputWindow::GetInstance()->GetNumberOfLoggedWarningMessages(); \
  if (actualNumberOfMessages != expectedNumberOfMessages) \
    { \
    std::cerr << "Assertion failed in " << __FILE__ << ":" << __LINE__ << " - expected " << expectedNumberOfMessages \
      << " warnings messages, got " << actualNumberOfMessages << std::endl; \
    exit(EXIT_FAILURE); \
    } \
  };

/// Exits with failure if the number of logged error messages is not equal to the specified number
#define TESTING_OUTPUT_ASSERT_ERRORS(expectedNumberOfMessages) \
  { \
  int actualNumberOfMessages = vtkTestingOutputWindow::GetInstance()->GetNumberOfLoggedErrorMessages(); \
  if (actualNumberOfMessages != expectedNumberOfMessages) \
    { \
    std::cerr << "Assertion failed in " << __FILE__ << ":" << __LINE__ << " - expected " << expectedNumberOfMessages \
      << " error messages, got " << actualNumberOfMessages << std::endl; \
    exit(EXIT_FAILURE); \
    } \
  };

/// Exits with failure if the number of logged error or warning messages is not equal to the specified number
#define TESTING_OUTPUT_ASSERT_WARNINGS_ERRORS(expectedNumberOfMessages) \
  { \
  int actualNumberOfMessages = vtkTestingOutputWindow::GetInstance()->GetNumberOfLoggedWarningErrorMessages(); \
  if (actualNumberOfMessages != expectedNumberOfMessages) \
    { \
    std::cerr << "Assertion failed in " << __FILE__ << ":" << __LINE__ << " - expected " << expectedNumberOfMessages \
      << " error or warning messages, got " << actualNumberOfMessages << std::endl; \
    exit(EXIT_FAILURE); \
    } \
  };

/// Exits with failure if the number of logged warning messages is equal or more than the specified number
#define TESTING_OUTPUT_ASSERT_WARNINGS_MINIMUM(expectedNumberOfMessages) \
  { \
  int actualNumberOfMessages = vtkTestingOutputWindow::GetInstance()->GetNumberOfLoggedWarningMessages(); \
  if (actualNumberOfMessages < expectedNumberOfMessages) \
    { \
    std::cerr << "Assertion failed in " << __FILE__ << ":" << __LINE__ << " - expected minimum " << expectedNumberOfMessages \
      << " warning messages, got " << actualNumberOfMessages << std::endl; \
    exit(EXIT_FAILURE); \
    } \
  };

/// Exits with failure if the number of logged error messages is equal or more than the specified number
#define TESTING_OUTPUT_ASSERT_ERRORS_MINIMUM(expectedNumberOfMessages) \
  { \
  int actualNumberOfMessages = vtkTestingOutputWindow::GetInstance()->GetNumberOfLoggedErrorMessages(); \
  if (actualNumberOfMessages < expectedNumberOfMessages) \
    { \
    std::cerr << "Assertion failed in " << __FILE__ << ":" << __LINE__ << " - expected minimum " << expectedNumberOfMessages \
      << " error messages, got " << actualNumberOfMessages << std::endl; \
    exit(EXIT_FAILURE); \
    } \
  };

/// Asserts that no warnings or errors has been logged so far and prepares for receiving warning(s)
#define TESTING_OUTPUT_ASSERT_WARNINGS_BEGIN() \
  { \
  /* Make sure there were no errors or warnings so far */ \
  TESTING_OUTPUT_ASSERT_WARNINGS_ERRORS(0); \
  vtkInfoWithoutObjectMacro("Expecting warning message(s)..."); \
  }

/// Asserts that warning(s) are logged but not errors, and clears the counters
#define TESTING_OUTPUT_ASSERT_WARNINGS_END() \
  { \
  TESTING_OUTPUT_ASSERT_WARNINGS_MINIMUM(1); \
  vtkInfoWithoutObjectMacro("Expected warning message(s) successfully received"); \
  TESTING_OUTPUT_ASSERT_ERRORS(0); \
  TESTING_OUTPUT_RESET(); \
  }

/// Asserts that no warnings or errors has been logged so far and prepares for receiving error(s)
#define TESTING_OUTPUT_ASSERT_ERRORS_BEGIN() \
  { \
  /* Make sure there were no errors or warnings so far */ \
  TESTING_OUTPUT_ASSERT_WARNINGS_ERRORS(0); \
  vtkInfoWithoutObjectMacro("Expecting warning or error message(s)..."); \
  }

/// Asserts that error(s) are logged (warnings ignored), and clears the counters
#define TESTING_OUTPUT_ASSERT_ERRORS_END() \
  { \
  TESTING_OUTPUT_ASSERT_ERRORS_MINIMUM(1); \
  vtkInfoWithoutObjectMacro("Expected error message(s) successfully received"); \
  TESTING_OUTPUT_RESET(); \
  }

/// Asserts that no warnings or errors has been logged so far and prepares for receiving error(s)
#define TESTING_OUTPUT_IGNORE_WARNINGS_ERRORS_BEGIN() \
  { \
  /* Make sure there were no errors or warnings so far */ \
  TESTING_OUTPUT_ASSERT_WARNINGS_ERRORS(0); \
  vtkInfoWithoutObjectMacro("Ignoring expected warning or error message(s)..."); \
  }

/// Asserts that error(s) are logged (warnings ignored), and clears the counters
#define TESTING_OUTPUT_IGNORE_WARNINGS_ERRORS_END() \
  { \
  vtkInfoWithoutObjectMacro("Finished ignoring warning or error message"); \
  TESTING_OUTPUT_RESET(); \
  }


#endif
