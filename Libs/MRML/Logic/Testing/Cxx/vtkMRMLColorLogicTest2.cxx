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
// MRML includes
#include "vtkMRMLCoreTestingMacros.h"

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// MRMLLogic includes
#include "vtkMRMLColorLogic.h"


using namespace vtkAddonTestingUtilities;
using namespace vtkMRMLCoreTestingUtilities;

namespace
{

//----------------------------------------------------------------------------
class vtkMRMLTestColorLogic : public vtkMRMLColorLogic
{
public:
  static vtkMRMLTestColorLogic *New();
  typedef vtkMRMLTestColorLogic Self;

  vtkTypeMacro(vtkMRMLTestColorLogic, vtkMRMLColorLogic);

  std::string test_RemoveLeadAndTrailSpaces(std::string inputText)
    {
    return this->RemoveLeadAndTrailSpaces(inputText);
    }

  bool test_ParseTerm(std::string inputText, StandardTerm& term, bool expectedReturnValue,
                      const char * expectedCode, const char * expectedScheme, const char * expectedMeaning)
    {
    std::string inputTextCopy = inputText;
    // errors are logged when invalid terms are parsed, ignore these and just check for returned values
    if (!expectedReturnValue)
      {
      TESTING_OUTPUT_IGNORE_WARNINGS_ERRORS_BEGIN();
      }
    bool retVal = this->ParseTerm(inputText, term);
    if (!expectedReturnValue)
      {
      TESTING_OUTPUT_IGNORE_WARNINGS_ERRORS_END();
      }
    CHECK_BOOL(retVal, expectedReturnValue);

    if (retVal)
      {
      // the string parsed as expected, now check that the term is as expected

      // the parsing shouldn't alter the input string
      CHECK_STD_STRING(inputText, inputTextCopy);

      CHECK_STD_STRING(term.CodeValue, expectedCode);
      CHECK_STD_STRING(term.CodingSchemeDesignator, expectedScheme);
      CHECK_STD_STRING(term.CodeMeaning, expectedMeaning);
      }

    return EXIT_SUCCESS;
    }

protected:
vtkMRMLTestColorLogic()
{
}
};

vtkStandardNewMacro(vtkMRMLTestColorLogic);
}

namespace
{

//----------------------------------------------------------------------------
int TestRemoveLeadAndTrailSpaces();
int TestParseTerm();
}

//----------------------------------------------------------------------------
int vtkMRMLColorLogicTest2(int vtkNotUsed(argc), char * vtkNotUsed(argv) [])
{
  CHECK_EXIT_SUCCESS(TestRemoveLeadAndTrailSpaces());
  CHECK_EXIT_SUCCESS(TestParseTerm());
  return EXIT_SUCCESS;
}

namespace
{
//----------------------------------------------------------------------------
// RemoveLeadAndTrailSpaces
//----------------------------------------------------------------------------
int TestRemoveLeadAndTrailSpaces()
{
  vtkNew<vtkMRMLTestColorLogic> colorLogic;

  CHECK_STD_STRING(colorLogic->test_RemoveLeadAndTrailSpaces(""), "");
  CHECK_STD_STRING(colorLogic->test_RemoveLeadAndTrailSpaces(" "), "");
  CHECK_STD_STRING(colorLogic->test_RemoveLeadAndTrailSpaces("  "), "");
  CHECK_STD_STRING(colorLogic->test_RemoveLeadAndTrailSpaces("   "), "");
  CHECK_STD_STRING(colorLogic->test_RemoveLeadAndTrailSpaces("1"), "1");
  CHECK_STD_STRING(colorLogic->test_RemoveLeadAndTrailSpaces("a"), "a");
  CHECK_STD_STRING(colorLogic->test_RemoveLeadAndTrailSpaces(" a"), "a");
  CHECK_STD_STRING(colorLogic->test_RemoveLeadAndTrailSpaces("a "), "a");
  CHECK_STD_STRING(colorLogic->test_RemoveLeadAndTrailSpaces("  testing string 1  "), "testing string 1");

  CHECK_STD_STRING(colorLogic->test_RemoveLeadAndTrailSpaces("(T-D0050;SRT;Tissue)"), "(T-D0050;SRT;Tissue)");
  CHECK_STD_STRING(colorLogic->test_RemoveLeadAndTrailSpaces(" (T-D0050;SRT;Tissue) "), "(T-D0050;SRT;Tissue)");
  CHECK_STD_STRING(colorLogic->test_RemoveLeadAndTrailSpaces("(T-D0050;SRT;Tissue) "), "(T-D0050;SRT;Tissue)");
  CHECK_STD_STRING(colorLogic->test_RemoveLeadAndTrailSpaces(" (T-D0050;SRT;Tissue)"), "(T-D0050;SRT;Tissue)");

  return EXIT_SUCCESS;
}


//----------------------------------------------------------------------------
// ParseTerm
//----------------------------------------------------------------------------
int TestParseTerm()
{
  vtkNew<vtkMRMLTestColorLogic> colorLogic;
  vtkMRMLColorLogic::StandardTerm term;

  // valid strings
  CHECK_EXIT_SUCCESS(colorLogic->test_ParseTerm("(T-D0050;SRT;Tissue)", term, true, "T-D0050", "SRT", "Tissue"));
  CHECK_EXIT_SUCCESS(colorLogic->test_ParseTerm("(M-01000;SRT;Morphologically Altered Structure)", term, true,
    "M-01000", "SRT", "Morphologically Altered Structure"));

  // invalid strings with the expected flag false will return true
  // empty string
  CHECK_EXIT_SUCCESS(colorLogic->test_ParseTerm("", term, false, "", "", ""));

  // too short a term string
  CHECK_EXIT_SUCCESS(colorLogic->test_ParseTerm("(M;S;B)", term, false, "", "", ""));

  // test missing brackets
  CHECK_EXIT_SUCCESS(colorLogic->test_ParseTerm("M-01000;SRT;Morphologically Altered Structure", term, false, "", "", ""));

  // test with missing semi colons
  CHECK_EXIT_SUCCESS(colorLogic->test_ParseTerm("(M-01000;SRT Morphologically Altered Structure)", term, false, "", "", ""));
  CHECK_EXIT_SUCCESS(colorLogic->test_ParseTerm("(M-01000 SRT;Morphologically Altered Structure)", term, false, "", "", ""));
  CHECK_EXIT_SUCCESS(colorLogic->test_ParseTerm("(M-01000 SRT Morphologically Altered Structure)", term, false, "", "", ""));

  return EXIT_SUCCESS;
}

}
