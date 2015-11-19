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
#include "vtkMRMLCoreTestingUtilities.h"

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// MRMLLogic includes
#include "vtkMRMLColorLogic.h"

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

  bool test_ParseTerm(int lineNumber, std::string inputText, StandardTerm& term, bool expectedReturnValue,
                      const char * expectedCode, const char * expectedScheme, const char * expectedMeaning)
    {
    std::string inputTextCopy = inputText;
    bool retVal = this->ParseTerm(inputText, term);

    if (!retVal && !expectedReturnValue)
      {
      // Expected to fail parsing this string, test passes
      return true;
      }
    if (retVal && !expectedReturnValue)
      {
      // expected to fail but succeeded, test fails
      std::cerr << lineNumber
                << ": test_ParseTerm: failed to not parse invalid string "
                << inputText
                << std::endl;
      return false;
      }
    if (!retVal && expectedReturnValue)
      {
      // expected to parse it, but failed, test fails
      std::cerr << lineNumber
                << ": test_ParseTerm: failed to parse string "
                << inputText
                << std::endl;
      return false;
      }
    // the string parsed as expected, now check that the term is as expected

    // the parsing shouldn't alter the input string
    if (!CheckString(lineNumber, "test_ParseTerm input string unchanged",
                     inputText.c_str(), inputTextCopy.c_str()))
      {
      return false;
      }
    if (!CheckString(lineNumber, "CodeValue",
                     term.CodeValue.c_str(), expectedCode)
        || !CheckString(lineNumber, "CodingSchemeDesignator",
                        term.CodingSchemeDesignator.c_str(), expectedScheme)
        || !CheckString(lineNumber, "CodeMeaning",
                        term.CodeMeaning.c_str(), expectedMeaning))
      {
      return false;
      }
    return true;
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
bool TestRemoveLeadAndTrailSpaces();
bool TestParseTerm();
}

//----------------------------------------------------------------------------
int vtkMRMLColorLogicTest2(int vtkNotUsed(argc), char * vtkNotUsed(argv) [])
{
  bool res = true;
  res = TestRemoveLeadAndTrailSpaces() && res;
  res = TestParseTerm() && res;
  return res ? EXIT_SUCCESS : EXIT_FAILURE;
}

namespace
{
//----------------------------------------------------------------------------
// RemoveLeadAndTrailSpaces
//----------------------------------------------------------------------------
bool TestRemoveLeadAndTrailSpaces()
{
  vtkNew<vtkMRMLTestColorLogic> colorLogic;

  if (!CheckString(__LINE__, "test_RemoveLeadAndTrailSpaces",
                   colorLogic->test_RemoveLeadAndTrailSpaces("").c_str(), "")
      || !CheckString(__LINE__, "test_RemoveLeadAndTrailSpaces",
                      colorLogic->test_RemoveLeadAndTrailSpaces(" ").c_str(), "")
      || !CheckString(__LINE__, "test_RemoveLeadAndTrailSpaces",
                      colorLogic->test_RemoveLeadAndTrailSpaces("  ").c_str(), "")
      || !CheckString(__LINE__, "test_RemoveLeadAndTrailSpaces",
                      colorLogic->test_RemoveLeadAndTrailSpaces("   ").c_str(), "")
      || !CheckString(__LINE__, "test_RemoveLeadAndTrailSpaces",
                      colorLogic->test_RemoveLeadAndTrailSpaces("1").c_str(), "1")
      || !CheckString(__LINE__, "test_RemoveLeadAndTrailSpaces",
                      colorLogic->test_RemoveLeadAndTrailSpaces("a").c_str(), "a")
      || !CheckString(__LINE__, "test_RemoveLeadAndTrailSpaces",
                      colorLogic->test_RemoveLeadAndTrailSpaces(" a").c_str(), "a")
      || !CheckString(__LINE__, "test_RemoveLeadAndTrailSpaces",
                      colorLogic->test_RemoveLeadAndTrailSpaces("a ").c_str(), "a")
      || !CheckString(__LINE__, "test_RemoveLeadAndTrailSpaces",
                      colorLogic->test_RemoveLeadAndTrailSpaces("  testing string 1  ").c_str(),
                      "testing string 1"))
    {
    return false;
    }

  std::string sampleStringIn1 = "(T-D0050;SRT;Tissue)";
  std::string sampleStringIn2 = " (T-D0050;SRT;Tissue) ";
  std::string sampleStringIn3 = "(T-D0050;SRT;Tissue) ";
  std::string sampleStringIn4 = " (T-D0050;SRT;Tissue)";
  std::string sampleStringOut = "(T-D0050;SRT;Tissue)";
  if (!CheckString(__LINE__, "test_RemoveLeadAndTrailSpaces",
                   colorLogic->test_RemoveLeadAndTrailSpaces(sampleStringIn1).c_str(),
                   sampleStringOut.c_str())
      || !CheckString(__LINE__, "test_RemoveLeadAndTrailSpaces",
                      colorLogic->test_RemoveLeadAndTrailSpaces(sampleStringIn2).c_str(),
                      sampleStringOut.c_str())
      || !CheckString(__LINE__, "test_RemoveLeadAndTrailSpaces",
                      colorLogic->test_RemoveLeadAndTrailSpaces(sampleStringIn3).c_str(),
                      sampleStringOut.c_str())
      || !CheckString(__LINE__, "test_RemoveLeadAndTrailSpaces",
                      colorLogic->test_RemoveLeadAndTrailSpaces(sampleStringIn4).c_str(),
                      sampleStringOut.c_str()))
    {
    return false;
    }

  return true;
}


//----------------------------------------------------------------------------
// ParseTerm
//----------------------------------------------------------------------------
bool TestParseTerm()
{
  vtkNew<vtkMRMLTestColorLogic> colorLogic;

  std::string str = "(T-D0050;SRT;Tissue)";
  vtkMRMLColorLogic::StandardTerm term;
  // the parsing shouldn't change the input string
  if (!colorLogic->test_ParseTerm(__LINE__, str, term, true, "T-D0050", "SRT", "Tissue"))
    {
    return false;
    }

  str = "(M-01000;SRT;Morphologically Altered Structure)";
  if (!colorLogic->test_ParseTerm(__LINE__, str, term, true,
                                  "M-01000", "SRT", "Morphologically Altered Structure"))
    {
    return false;
    }

  // invalid strings with the expected flag false will return true
  // empty string
  str = "";
  if (!colorLogic->test_ParseTerm(__LINE__, str, term, false, "", "", ""))
    {
    return false;
    }
  // too short a term string
  str = "(M;S;B)";
  if (!colorLogic->test_ParseTerm(__LINE__, str, term, false, "", "", ""))
    {
    return false;
    }

  // test missing brackets
  str = "M-01000;SRT;Morphologically Altered Structure";
  if (!colorLogic->test_ParseTerm(__LINE__, str, term, false, "", "", ""))
    {
    return false;
    }

  // test with missing semi colons
  str = "(M-01000;SRT Morphologically Altered Structure)";
  if (!colorLogic->test_ParseTerm(__LINE__, str, term, false, "", "", ""))
    {
    return false;
    }
  str = "(M-01000 SRT;Morphologically Altered Structure)";
  if (!colorLogic->test_ParseTerm(__LINE__, str, term, false, "", "", ""))
    {
    return false;
    }
  str = "(M-01000 SRT Morphologically Altered Structure)";
  if (!colorLogic->test_ParseTerm(__LINE__, str, term, false, "", "", ""))
    {
    return false;
    }
  return true;
}

}
