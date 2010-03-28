
/****************************************************************************** 
 * 
 *  file:  XMLOutput.h
 * 
 *  Copyright (c) 2004, Michael E. Smoot
 *  All rights reverved.
 * 
 *  See the file COPYING in the top directory of this distribution for
 *  more information.
 *  
 *  THE SOFTWARE IS PROVIDED _AS IS_, WITHOUT WARRANTY OF ANY KIND, EXPRESS 
 *  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
 *  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 *  DEALINGS IN THE SOFTWARE.  
 *  
 *****************************************************************************/ 

#ifndef TCLAP_XMLOUTPUT_H
#define TCLAP_XMLOUTPUT_H

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <algorithm>

#include <tclap/CmdLineInterface.h>
#include <tclap/CmdLineOutput.h>
#include <tclap/XorHandler.h>
#include <tclap/Arg.h>

namespace TCLAP {

/**
 * A class that generates XML output for usage() method for the 
 * given CmdLine and its Args.
 */
class XMLOutput : public CmdLineOutput
{

  public:

    /**
     * Prints the usage to stdout.  Can be overridden to 
     * produce alternative behavior.
     * \param c - The CmdLine object the output is generated for. 
     */
    virtual void usage(CmdLineInterface& c);

    /**
     * Prints the version to stdout. Can be overridden 
     * to produce alternative behavior.
     * \param c - The CmdLine object the output is generated for. 
     */
    virtual void version(CmdLineInterface& c);

    /**
     * Prints (to stderr) an error message, short usage 
     * Can be overridden to produce alternative behavior.
     * \param c - The CmdLine object the output is generated for. 
     * \param e - The ArgException that caused the failure. 
     */
    virtual void failure(CmdLineInterface& c, 
                 ArgException& e );

  protected:

    /**
     * Substitutes the char r for string x in string s.
     * \param s - The string to operate on. 
     * \param r - The char to replace. 
     * \param x - What to replace r with. 
     */
    void substituteSpecialChars( std::string& s, char r, std::string& x );
    void removeChar( std::string& s, char r);

    void printShortArg(Arg* it);
    void printLongArg(Arg* it);
};


inline void XMLOutput::version(CmdLineInterface& _cmd) 
{ 
  std::cout << _cmd.getVersion() << std::endl;
}

inline void XMLOutput::usage(CmdLineInterface& _cmd ) 
{
  std::list<Arg*> argList = _cmd.getArgList();
  std::string progName = _cmd.getProgramName();
  std::string version = _cmd.getVersion();
  XorHandler xorHandler = _cmd.getXorHandler();
  std::vector< std::vector<Arg*> > xorList = xorHandler.getXorList();


  std::cout << "<?xml version=\"1.0\" encoding=\"utf-8\"?>" << std::endl;
  std::cout << "<executable>" << std::endl;
  std::cout << "<name>" << generateSafeString ( progName ) << "</name>" << std::endl;
  std::cout << "<version>" << generateSafeString ( version ) << "</version>" << std::endl;
  std::cout << "<description><CDATA[" << description << "]]></description>" << std::endl;

  // Start the parameters
  std::cout << "<parameters>" << std::endl;
  
  // Do the xor's first
  for ( int i = 0; (unsigned int)i < xorList.size(); i++ )
    {
    std::cout << "<xor>" << std::endl;
    for ( ArgVectorIterator it = xorList[i].begin(); 
          it != xorList[i].end(); it++ )
      {
      printLongArg((*it));
      }
    std::cout << "</xor>" << std::endl;
    }
  
  // rest of args
  for (ArgListIterator it = argList.begin(); it != argList.end(); it++)
    {
    if ( !xorHandler.contains( (*it) ) )
      {
      printLongArg((*it));
      }
    }
  std::cout << "</parameters>" << std::endl;
  std::cout << "</executable>" << std::endl;
}

inline void XMLOutput::failure( CmdLineInterface& _cmd,
            ArgException& e ) 
{ 
    std::cout << e.what() << std::endl;
}

inline void XMLOutput::substituteSpecialChars( std::string& s,
             char r,
                           std::string& x )
{
  std::string::size_type p;
  while ( (p = s.find_first_of(r)) != std::string::npos )
  {
    s.erase(p,1);
    s.insert(p,x);
  }
}

inline void XMLOutput::removeChar( std::string& s, char r)
{
  std::string::size_type p;
  while ( (p = s.find_first_of(r)) != std::string::npos )
  {
    s.erase(p,1);
  }
}

  inline std::string XMLOutput::generateSafeString ( std::string s )
  {
    std::string lt = "&lt;"; 
    std::string gt = "&gt;";
    std::string amp = "&amp;";
    std::string apos = "&apos;";
    std::string quot = "&quot;";
    
    std::out ( s );
    substituteSpecialChars(out,'<',lt);
    substituteSpecialChars(out,'>',gt);
    substituteSpecialChars(out,'&',amp);
    substituteSpecialChars(out,'\'',apos);
    substituteSpecialChars(out,'"',quot);
    removeChar(out,'[');
    removeChar(out,']');
    return out;
  }
    
  
inline void XMLOutput::printShortArg(Arg* a)
{
  std::string lt = "&lt;"; 
  std::string gt = "&gt;"; 

  std::string id = a->shortID();
  substituteSpecialChars(id,'<',lt);
  substituteSpecialChars(id,'>',gt);
  removeChar(id,'[');
  removeChar(id,']');
  
  std::string choice = "opt";
  if ( a->isRequired() )
    choice = "req";

  std::string repeat = "norepeat";
  if ( a->acceptsMultipleValues() )
    repeat = "repeat";

    
        
  std::cout << "<arg choice='" << choice 
        << "' repeat='" << repeat << "'>" 
        << id << "</arg>" << std::endl; 

}

inline void XMLOutput::printLongArg(Arg* a)
{

  std::string id = a->longID();
  std::string desc = a->getDescription();

  
  if ( a->isValueRequired() )
    {
    std::cout << "<value ";
    }
  else
    {
    std::cout << "<switch ";
    }

  std::cout << "name=\"" << generateSafeString ( a->getName() ) << " ";
  std::cout << "flag=\"" << generateSafeString ( a->getFlag() ) << " ";
  if ( a->isRequired() )
    {
    std::cout << "required=\"true\" ";
    }
  else
    {
    std::cout << "required=\"false\" ";
    }
  if ( a->acceptsMultipleValues() )
    {
    std::cout << "allowrepeats=\"true\" ";
    }
  else
    {
    std::cout << "allowrepeats=\"false\" ";
    }
  std::cout << "type=\"" << a->getTypeDescription() << "\" ";
  std::cout << ">" << std::endl;
  
  
  

  std::cout << "<simplelist>" << std::endl;

  std::cout << "<member>" << std::endl;
  std::cout << id << std::endl;
  std::cout << "</member>" << std::endl;

  std::cout << "<member>" << std::endl;
  std::cout << desc << std::endl;
  std::cout << "</member>" << std::endl;

  std::cout << "</simplelist>" << std::endl;
}

} //namespace TCLAP
#endif 
