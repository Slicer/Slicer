#include <sstream>

#include "vtkObjectFactory.h"
#include <vtkStdString.h>

#include "vtkMRMLAnnotationTextDisplayNode.h"

//------------------------------------------------------------------------------
vtkMRMLAnnotationTextDisplayNode*
vtkMRMLAnnotationTextDisplayNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance(
      "vtkMRMLAnnotationTextDisplayNode");
  if (ret)
    {
      return (vtkMRMLAnnotationTextDisplayNode*) ret;
    }
  // If the factory was unable to create the object, then create it here.
  vtkMRMLAnnotationTextDisplayNode* result = new vtkMRMLAnnotationTextDisplayNode;
#ifdef VTK_HAS_INITIALIZE_OBJECT_BASE
  result->InitializeObjectBase();
#endif
  return result;
}

//-----------------------------------------------------------------------------
vtkMRMLNode*
vtkMRMLAnnotationTextDisplayNode::CreateNodeInstance()
{
  vtkObject* ret = vtkObjectFactory::CreateInstance(
      "vtkMRMLAnnotationTextDisplayNode");
  if (ret)
    {
      return (vtkMRMLAnnotationTextDisplayNode*) ret;
    }
  // If the factory was unable to create the object, then create it here.
  vtkMRMLAnnotationTextDisplayNode* result = new vtkMRMLAnnotationTextDisplayNode;
#ifdef VTK_HAS_INITIALIZE_OBJECT_BASE
  result->InitializeObjectBase();
#endif
  return result;
}

//----------------------------------------------------------------------------
vtkMRMLAnnotationTextDisplayNode::vtkMRMLAnnotationTextDisplayNode()
{
  this->TextScale = 4.5;

  this->UseLineWrap = 0;
  this->MaxCharactersPerLine = 20;

  this->ShowArrowHead = 1;
  this->ShowBorder = 1;
  this->ShowLeader = 1;
  this->UseThreeDimensionalLeader = 1;
  this->LeaderGlyphSize = 0.025;
  this->MaximumLeaderGlyphSize = 20;
  this->Padding = 3;
  this->AttachEdgeOnly = 0;
}

//----------------------------------------------------------------------------
void
vtkMRMLAnnotationTextDisplayNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults

  Superclass::WriteXML(of, nIndent);

  of << " textScale=\"" << this->TextScale << "\"";
  of << " useLineWrap=\"" << (this->UseLineWrap ? "true" : "false") << "\"";
  of << " maxCharactersPerLine=\"" << this->MaxCharactersPerLine<< "\"";
  of << " showBorder=\"" << (this->ShowBorder ? "true" : "false") << "\"";
  of << " showLeader=\"" << (this->ShowLeader ? "true" : "false") << "\"";
  of << " showArrowHead=\"" << (this->ShowArrowHead ? "true" : "false") << "\"";
  of << " useThreeDimensionalLeader=\"" << (this->UseThreeDimensionalLeader ? "true" : "false") << "\"";
  of << " leaderGlyphSize=\"" << this->LeaderGlyphSize << "\"";
  of << " maximumLeaderGlyphSize=\"" << this->MaximumLeaderGlyphSize << "\"";
  of << " padding=\"" << this->Padding << "\"";
  of << " attachEdgeOnly=\"" << (this->AttachEdgeOnly ? "true" : "false") << "\"";
}

//----------------------------------------------------------------------------
void
vtkMRMLAnnotationTextDisplayNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
    {
    attName = *(atts++);
    attValue = *(atts++);

    if (!strcmp(attName, "textScale"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->TextScale;
      }
    else if (!strcmp(attName, "useLineWrap"))
      {
      if (!strcmp(attValue,"true"))
        {
        this->UseLineWrap = 1;
        }
      else
        {
        this->UseLineWrap = 0;
        }
      }
    else if (!strcmp(attName, "maxCharactersPerLine"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->MaxCharactersPerLine;
      }
    else if (!strcmp(attName, "showBorder"))
      {
      if (!strcmp(attValue,"true"))
        {
        this->ShowBorder = 1;
        }
      else
        {
        this->ShowBorder = 0;
        }
      }
    else if (!strcmp(attName, "showLeader"))
      {
      if (!strcmp(attValue,"true"))
        {
        this->ShowLeader = 1;
        }
      else
        {
        this->ShowLeader = 0;
        }
      }
    else if (!strcmp(attName, "showArrowHead"))
      {
      if (!strcmp(attValue,"true"))
        {
        this->ShowArrowHead = 1;
        }
      else
        {
        this->ShowArrowHead = 0;
        }
      }
    else if (!strcmp(attName, "useThreeDimensionalLeader"))
      {
      if (!strcmp(attValue,"true"))
        {
        this->UseThreeDimensionalLeader = 1;
        }
      else
        {
        this->UseThreeDimensionalLeader = 0;
        }
      }
    else if (!strcmp(attName, "leaderGlyphSize"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->LeaderGlyphSize;
      }
    else if (!strcmp(attName, "maximumLeaderGlyphSize"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->MaximumLeaderGlyphSize;
      }
    else if (!strcmp(attName, "padding"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->Padding;
      }
    else if (!strcmp(attName, "attachEdgeOnly"))
      {
      if (!strcmp(attValue,"true"))
        {
        this->AttachEdgeOnly = 1;
        }
      else
        {
        this->AttachEdgeOnly = 0;
        }
      }
    }

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void
vtkMRMLAnnotationTextDisplayNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);
  vtkMRMLAnnotationTextDisplayNode *node =
      (vtkMRMLAnnotationTextDisplayNode *) anode;

  this->SetTextScale(node->TextScale);
  this->SetUseLineWrap(node->UseLineWrap);
  this->SetMaxCharactersPerLine(node->MaxCharactersPerLine);
  this->SetShowBorder(node->ShowBorder);
  this->SetShowLeader(node->ShowLeader);
  this->SetShowArrowHead(node->ShowArrowHead);
  this->SetUseThreeDimensionalLeader(node->UseThreeDimensionalLeader);
  this->SetLeaderGlyphSize(node->LeaderGlyphSize);
  this->SetMaximumLeaderGlyphSize(node->MaximumLeaderGlyphSize);
  this->SetPadding(node->Padding);
  this->SetAttachEdgeOnly(node->AttachEdgeOnly);

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void
vtkMRMLAnnotationTextDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
  os << indent << "Text scale: (";
  os << this->TextScale << ")\n";

  os << indent << "UseLineWrap: " << (this->UseLineWrap ? "true" : "false") << std::endl;
  os << indent << "MaxCharactersPerLine: " << this->MaxCharactersPerLine << std::endl;

  os << indent << "ShowBorder: " << (this->ShowBorder ? "true" : "false") << std::endl;
  os << indent << "ShowLeader: " << (this->ShowLeader ? "true" : "false") << std::endl;
  os << indent << "ShowArrowHead: " << (this->ShowArrowHead ? "true" : "false") << std::endl;
  os << indent << "UseThreeDimensionalLeader: " << (this->UseThreeDimensionalLeader ? "true" : "false") << std::endl;
  os << indent << "LeaderGlyphSize: " << this->LeaderGlyphSize << std::endl;
  os << indent << "MaximumLeaderGlyphSize: " << this->MaximumLeaderGlyphSize << std::endl;
  os << indent << "Padding: " << this->Padding << std::endl;
  os << indent << "AttachEdgeOnly: " << (this->AttachEdgeOnly ? "true" : "false") << std::endl;

}

//---------------------------------------------------------------------------
void
vtkMRMLAnnotationTextDisplayNode::ProcessMRMLEvents(vtkObject *caller,
    unsigned long event, void *callData)
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
  return;
}

//-----------------------------------------------------------
void
vtkMRMLAnnotationTextDisplayNode::UpdateScene(vtkMRMLScene *scene)
{
  Superclass::UpdateScene(scene);
}

//---------------------------------------------------------------------------
void
vtkMRMLAnnotationTextDisplayNode::SetTextScale(double scale)
{
  if (this->TextScale == scale)
    {
    return;
    }
  //vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting TextScale to " << scale);
  this->TextScale = scale;

  this->Modified();
}



//----------------------------------------------------------------------------
// Create a backup of this node and store it with the node.
void vtkMRMLAnnotationTextDisplayNode::CreateBackup()
{

  vtkMRMLAnnotationTextDisplayNode * backupNode = vtkMRMLAnnotationTextDisplayNode::New();

  int oldMode = backupNode->GetDisableModifiedEvent();
  backupNode->DisableModifiedEventOn();
  backupNode->Copy(this);
  backupNode->SetDisableModifiedEvent(oldMode);

  this->m_Backup = backupNode;

}

//----------------------------------------------------------------------------
// Restores the backup of this node.
void vtkMRMLAnnotationTextDisplayNode::RestoreBackup()
{

  if (this->m_Backup)
    {
    MRMLNodeModifyBlocker blocker(this);
    this->Copy(this->m_Backup);
    }
  else
    {
    vtkErrorMacro("RestoreBackup - could not get the attached backup:" << this->GetID());
    }

}

//----------------------------------------------------------------------------
// returns the line wrapped version of the input string, using the current
// setting of MaxCharactersPerLine
std::string vtkMRMLAnnotationTextDisplayNode::GetLineWrappedText(std::string inputText)
{
  std::string wrappedText;
  size_t maxCharPerLine = (size_t)(this->GetMaxCharactersPerLine());


  if (inputText.find_first_of(' ') == std::string::npos)
    {
    vtkDebugMacro("There are no spaces in the line '" << inputText.c_str() << "'" );
    // there are no spaces in the line, so grab max chars at a time and line
    // break
    size_t currentChar = 0;
    while (currentChar < inputText.size())
      {
      std::string oneLine = inputText.substr(currentChar, maxCharPerLine);
      wrappedText.append(oneLine);
      currentChar += maxCharPerLine;
      if (currentChar < inputText.size())
        {
        wrappedText.push_back('\n');
        }
      vtkDebugMacro("\tAppended a line '" << oneLine << "', currentChar = " << currentChar );
      }
    return wrappedText;
    }
  size_t spaceLeft = maxCharPerLine;
  vtkDebugMacro("spaceLeft = " << spaceLeft );
  char *line = (char *)(inputText.c_str());
  char *ptr = strtok(line, " ");
  while (ptr != nullptr)
    {
    size_t wordWidth = strlen(ptr);
    vtkDebugMacro("ptr = '" << ptr << "', len = " << wordWidth << ", spaceLeft = " << spaceLeft << ", wrappedText is currently = \n'" << wrappedText.c_str() << "'" );
    // check if adding this word plus a space goes over the line limit
    if (wordWidth + 1 <= spaceLeft)
      {
      // add this word and a space
      vtkDebugMacro("space to add this word '" << ptr << "' and a space" );
      wrappedText.append(std::string(ptr));
      wrappedText.push_back(' ');
      spaceLeft = spaceLeft - (wordWidth + 1);
      }
    else if (wordWidth <= spaceLeft)
      {
      // room to add this word but no space
      vtkDebugMacro("wordWidth " << wordWidth << " is less than space left " << spaceLeft << ", adding it" );
      wrappedText.append(std::string(ptr));
      wrappedText.push_back('\n');
      spaceLeft = maxCharPerLine;
      }
    else if (wordWidth > maxCharPerLine)
      {
      // start it on it's own line and break it up
      // remove any extra space from the line before
      size_t lastSpace = wrappedText.find_last_of(' ');
      if (lastSpace == wrappedText.size()-1)
        {
        wrappedText.erase(lastSpace);
        }
      // now add the new line
      wrappedText.push_back('\n');
      std::string bigWord = std::string(ptr);
      size_t currentChar = 0;
      while (currentChar < bigWord.size())
        {
        std::string oneLine = bigWord.substr(currentChar, maxCharPerLine);
        wrappedText.append(oneLine);
        spaceLeft = maxCharPerLine - oneLine.length();
        currentChar += oneLine.length();
        if (currentChar < bigWord.size())
          {
          wrappedText.push_back('\n');
          spaceLeft = maxCharPerLine;
          }
        else
          {
          // finish with a space
          wrappedText.push_back(' ');
          spaceLeft--;
          }

        }
      }
    else
      {
      // insert a line break before this word
      vtkDebugMacro("Adding a line break and this word and a space, width + 1 " << wordWidth + 1 << ", space left " << spaceLeft );
      // remove any extra space from the line before
      size_t lastSpace = wrappedText.find_last_of(' ');
      if (lastSpace == wrappedText.size()-1)
        {
        wrappedText.erase(lastSpace);
        }
      // now add the new line and word
      wrappedText.push_back('\n');
      wrappedText.append(std::string(ptr));
      spaceLeft = maxCharPerLine - wordWidth;
      // don't add a space if we're already over the line limit on a single word
      if (spaceLeft > 0)
        {
        wrappedText.push_back(' ');
        spaceLeft--;
        }
      }
    ptr = strtok(nullptr, " ");
    }
  // if the last thing on the line is a space or newline, remove it
  size_t lastLineFeed = wrappedText.find_last_of('\n');
  vtkDebugMacro("\tlast line feed = " << lastLineFeed << ", text size = " <<  wrappedText.size() );
  if (lastLineFeed == wrappedText.size()-1)
    {
    vtkDebugMacro("\tRemoving trailing newline" );
    wrappedText.erase(lastLineFeed);
    }
  size_t lastSpace = wrappedText.find_last_of(' ');
  vtkDebugMacro("\tlast space = " << lastSpace << ", text size =  " <<  wrappedText.size() );
  if (lastSpace == wrappedText.size()-1)
    {
    vtkDebugMacro("\tRemoving trailing space" );
    wrappedText.erase(lastSpace);
    }

  return wrappedText;
}
