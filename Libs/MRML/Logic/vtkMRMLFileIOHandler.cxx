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

==============================================================================*/

#include "vtkMRMLFileIOHandler.h"
#include "vtkMRMLFileIOManager.h"
#include "vtkMRMLIOOptionsDescription.h"
#include "vtkMRMLIOProperties.h"

#include <vtkMRMLMessageCollection.h>
#include <vtkMRMLScene.h>

#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtksys/SystemTools.hxx>

#include <algorithm>
#include <cctype>
#include <regex>
#include <sstream>

vtkStandardNewMacro(vtkMRMLFileIOHandler);

//----------------------------------------------------------------------------
vtkMRMLFileIOHandler::vtkMRMLFileIOHandler() = default;

//----------------------------------------------------------------------------
vtkMRMLFileIOHandler::~vtkMRMLFileIOHandler() = default;

//----------------------------------------------------------------------------
void vtkMRMLFileIOHandler::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "FileType: " << this->FileType << "\n";
  os << indent << "Description: " << this->Description << "\n";
  os << indent << "NameFilters:";
  for (const std::string& filter : this->NameFilters)
  {
    os << " " << filter;
  }
  os << "\n";
}

//----------------------------------------------------------------------------
void vtkMRMLFileIOHandler::SetNameFilters(const std::vector<std::string>& filters)
{
  this->NameFilters = filters;
  this->Extensions.clear();
  for (const std::string& filter : filters)
  {
    for (const std::string& extension : vtkMRMLFileIOHandler::ExtensionsFromNameFilter(filter))
    {
      if (std::find(this->Extensions.begin(), this->Extensions.end(), extension) == this->Extensions.end())
      {
        this->Extensions.push_back(extension);
      }
    }
  }
  this->Modified();
}

//----------------------------------------------------------------------------
bool vtkMRMLFileIOHandler::MatchesExtension(const std::string& filePath) const
{
  return !this->GetMatchedExtension(filePath).empty();
}

//----------------------------------------------------------------------------
std::string vtkMRMLFileIOHandler::GetMatchedExtension(const std::string& filePath) const
{
  return vtkMRMLFileIOHandler::LongestMatchingExtension(filePath, this->Extensions);
}

//----------------------------------------------------------------------------
void vtkMRMLFileIOHandler::SetScene(vtkMRMLScene* scene)
{
  if (this->Scene == scene)
  {
    return;
  }
  this->Scene = scene;
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLFileIOHandler::GetOptionsDescription(vtkMRMLIOOptionsDescription* vtkNotUsed(description))
{
  // no options by default
}

//----------------------------------------------------------------------------
void vtkMRMLFileIOHandler::FillOptionsDescription(vtkMRMLIOProperties* properties, vtkMRMLIOOptionsDescription* description)
{
  if (!description)
  {
    vtkErrorMacro("FillOptionsDescription failed: invalid description");
    return;
  }
  description->RemoveAllOptions();
  if (properties)
  {
    description->SetProperties(properties);
  }
  else
  {
    vtkNew<vtkMRMLIOProperties> emptyProperties;
    description->SetProperties(emptyProperties);
  }
  description->SetDefaults(this->GetOptionDefaults());
  this->GetOptionsDescription(description);
}

//----------------------------------------------------------------------------
std::string vtkMRMLFileIOHandler::GetOptionsDescriptionJSON(vtkMRMLIOProperties* properties)
{
  vtkNew<vtkMRMLIOOptionsDescription> description;
  this->FillOptionsDescription(properties, description);
  if (description->GetNumberOfOptions() == 0)
  {
    return std::string();
  }
  return description->ToJSON();
}

//----------------------------------------------------------------------------
void vtkMRMLFileIOHandler::GetOptionValues(vtkMRMLIOProperties* properties, vtkMRMLIOProperties* values)
{
  if (!values)
  {
    return;
  }
  vtkNew<vtkMRMLIOOptionsDescription> description;
  this->FillOptionsDescription(properties, description);
  description->GetOptionValues(values);
}

//----------------------------------------------------------------------------
vtkMRMLIOProperties* vtkMRMLFileIOHandler::GetOptionDefaults()
{
  return this->OptionDefaults;
}

//----------------------------------------------------------------------------
vtkMRMLScene* vtkMRMLFileIOHandler::GetScene() const
{
  return this->Scene;
}

//----------------------------------------------------------------------------
vtkMRMLMessageCollection* vtkMRMLFileIOHandler::GetUserMessages() const
{
  return this->UserMessages;
}

//----------------------------------------------------------------------------
std::string vtkMRMLFileIOHandler::LongestMatchingExtension(const std::string& filePath, const std::vector<std::string>& extensions)
{
  std::string name = filePath;
  const std::string::size_type slash = name.find_last_of("/\\");
  if (slash != std::string::npos)
  {
    name = name.substr(slash + 1);
  }
  std::transform(name.begin(), name.end(), name.begin(), [](char c) { return static_cast<char>(std::tolower(static_cast<unsigned char>(c))); });

  // The longest match wins, so that ".seg.nrrd" is preferred over ".nrrd"
  std::string matched;
  for (const std::string& extension : extensions)
  {
    if (extension.empty() || extension.size() > name.size())
    {
      continue;
    }
    if (name.compare(name.size() - extension.size(), extension.size(), extension) == 0 && extension.size() > matched.size())
    {
      matched = extension;
    }
  }
  return matched;
}

//----------------------------------------------------------------------------
std::vector<std::string> vtkMRMLFileIOHandler::ExtensionsFromNameFilter(const std::string& nameFilter)
{
  // "Mimics project (*.mcs *.mcs.gz)" -> ".mcs", ".mcs.gz"; a filter without brackets is taken as
  // the patterns themselves ("*.mcs"). Patterns are in the last pair of brackets, as the description
  // may contain brackets, too ("Markups (JSON) (*.mrk.json)").
  std::string patterns = nameFilter;
  const std::string::size_type close = nameFilter.rfind(')');
  const std::string::size_type open = (close != std::string::npos) ? nameFilter.rfind('(', close) : std::string::npos;
  if (open != std::string::npos && close != std::string::npos && close > open)
  {
    patterns = nameFilter.substr(open + 1, close - open - 1);
  }

  std::vector<std::string> extensions;
  std::istringstream stream(patterns);
  std::string pattern;
  while (stream >> pattern)
  {
    if (pattern == "*" || pattern == "*.*")
    {
      continue; // takes any file: nothing to match on
    }
    if (!pattern.empty() && pattern[0] == '*')
    {
      pattern = pattern.substr(1);
    }
    if (pattern.empty() || pattern[0] != '.')
    {
      continue;
    }
    std::transform(pattern.begin(), pattern.end(), pattern.begin(), [](char c) { return static_cast<char>(std::tolower(static_cast<unsigned char>(c))); });
    extensions.push_back(pattern);
  }
  return extensions;
}

//----------------------------------------------------------------------------
vtkMRMLFileIOManager* vtkMRMLFileIOHandler::GetFileIOManager() const
{
  return this->FileIOManager;
}

//----------------------------------------------------------------------------
void vtkMRMLFileIOHandler::SetFileIOManager(vtkMRMLFileIOManager* manager)
{
  this->FileIOManager = manager;
}

//----------------------------------------------------------------------------
std::vector<std::string> vtkMRMLFileIOHandler::NameFilterToWildcards(const std::string& nameFilter)
{
  // Same as ctk::nameFilterToExtensions
  static const std::regex nameFilterRegExp(R"re(^(.*)\(([a-zA-Z0-9_.*? +;#\-\[\]@{}/!<>$%&=^~:|]*)\)$)re");
  std::smatch match;
  std::vector<std::string> wildcards;
  if (!std::regex_match(nameFilter, match, nameFilterRegExp))
  {
    static const std::regex validWildcard(R"re(^[\w\s.*_~$\[\]]+$)re");
    if (std::regex_match(nameFilter, validWildcard))
    {
      wildcards.push_back(nameFilter);
    }
    return wildcards;
  }
  std::istringstream stream(match[2].str());
  std::string wildcard;
  while (stream >> wildcard)
  {
    wildcards.push_back(wildcard);
  }
  return wildcards;
}

//----------------------------------------------------------------------------
bool vtkMRMLFileIOHandler::WildcardMatch(const std::string& pattern, const std::string& text)
{
  // Iterative matching with backtracking for '*'
  size_t p = 0;
  size_t t = 0;
  size_t starPattern = std::string::npos;
  size_t starText = 0;
  auto lower = [](char c) { return static_cast<char>(std::tolower(static_cast<unsigned char>(c))); };
  // Returns the length of the pattern element at position pos and whether it matches character c
  auto matchElement = [&](size_t pos, char c, bool& matched) -> size_t
  {
    if (pattern[pos] == '?')
    {
      matched = true;
      return 1;
    }
    if (pattern[pos] == '[')
    {
      size_t end = pattern.find(']', pos + 2);
      if (end != std::string::npos)
      {
        size_t start = pos + 1;
        bool negate = (pattern[start] == '!' || pattern[start] == '^');
        if (negate)
        {
          ++start;
        }
        bool inSet = false;
        for (size_t i = start; i < end; ++i)
        {
          if (i + 2 < end && pattern[i + 1] == '-')
          {
            if (lower(c) >= lower(pattern[i]) && lower(c) <= lower(pattern[i + 2]))
            {
              inSet = true;
            }
            i += 2;
          }
          else if (lower(pattern[i]) == lower(c))
          {
            inSet = true;
          }
        }
        matched = (inSet != negate);
        return end - pos + 1;
      }
    }
    matched = (lower(pattern[pos]) == lower(c));
    return 1;
  };
  while (t < text.size())
  {
    if (p < pattern.size() && pattern[p] == '*')
    {
      starPattern = p++;
      starText = t;
      continue;
    }
    if (p < pattern.size())
    {
      bool matched = false;
      size_t elementLength = matchElement(p, text[t], matched);
      if (matched)
      {
        p += elementLength;
        ++t;
        continue;
      }
    }
    if (starPattern != std::string::npos)
    {
      p = starPattern + 1;
      t = ++starText;
      continue;
    }
    return false;
  }
  while (p < pattern.size() && pattern[p] == '*')
  {
    ++p;
  }
  return p == pattern.size();
}

//----------------------------------------------------------------------------
std::vector<std::string> vtkMRMLFileIOHandler::GetMatchingNameFilters(const std::string& filePath,
                                                                      const std::vector<std::string>& nameFilters,
                                                                      bool requireReadableFile,
                                                                      int* longestExtensionMatch /*=nullptr*/)
{
  if (longestExtensionMatch)
  {
    *longestExtensionMatch = 0;
  }
  std::vector<std::string> matchingNameFilters;
  std::string fileName = vtksys::SystemTools::GetFilenameName(filePath);
  if (requireReadableFile)
  {
    std::string::size_type lastDot = fileName.rfind('.');
    std::string suffix = (lastDot == std::string::npos) ? std::string() : fileName.substr(lastDot + 1);
    if (!vtksys::SystemTools::FileExists(filePath, /*isFile=*/true)               //
        || !vtksys::SystemTools::TestFileAccess(filePath, vtksys::TEST_FILE_READ) //
        || suffix.find('~') != std::string::npos)                                 // temporary file
    {
      return matchingNameFilters;
    }
  }
  for (const std::string& nameFilter : nameFilters)
  {
    for (std::string wildcard : vtkMRMLFileIOHandler::NameFilterToWildcards(nameFilter))
    {
      if (!vtkMRMLFileIOHandler::WildcardMatch(wildcard, fileName))
      {
        continue;
      }
      // wildcard does not count, that's not a specific match
      wildcard.erase(std::remove(wildcard.begin(), wildcard.end(), '*'), wildcard.end());
      if (longestExtensionMatch && *longestExtensionMatch < static_cast<int>(wildcard.size()))
      {
        *longestExtensionMatch = static_cast<int>(wildcard.size());
      }
      if (std::find(matchingNameFilters.begin(), matchingNameFilters.end(), nameFilter) == matchingNameFilters.end())
      {
        matchingNameFilters.push_back(nameFilter);
      }
    }
  }
  return matchingNameFilters;
}
