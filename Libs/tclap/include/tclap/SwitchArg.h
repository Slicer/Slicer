
/****************************************************************************** 
 * 
 *  file:  SwitchArg.h
 * 
 *  Copyright (c) 2003, Michael E. Smoot .
 *  Copyright (c) 2004, Michael E. Smoot, Daniel Aarno.
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


#ifndef TCLAP_SWITCH_ARG_H
#define TCLAP_SWITCH_ARG_H

#include <string>
#include <vector>

#include <tclap/Arg.h>

namespace TCLAP {

/**
 * A simple switch argument.  If the switch is set on the command line, then
 * the getValue method will return the opposite of the default value for the
 * switch.
 */
class SwitchArg : public Arg
{
protected:

  /**
   * The value of the switch.
   */
  bool _value;

public:

  /**
   * SwitchArg constructor.
   * \param flag - The one character flag that identifies this
   * argument on the command line.
   * \param name - A one word name for the argument.  Can be
   * used as a long flag on the command line.
   * \param desc - A description of what the argument is for or
   * does.
   * \param def - The default value for this Switch. 
   * \param v - An optional visitor.  You probably should not
   * use this unless you have a very good reason.
   */
  SwitchArg(const std::string& flag, 
            const std::string& name, 
            const std::string& desc,
            bool def = false,
            Visitor* v = NULL);

          
  /**
   * SwitchArg constructor.
   * \param flag - The one character flag that identifies this
   * argument on the command line.
   * \param name - A one word name for the argument.  Can be
   * used as a long flag on the command line.
   * \param desc - A description of what the argument is for or
   * does.
   * \param parser - A CmdLine parser object to add this Arg to
   * \param def - The default value for this Switch.
   * \param v - An optional visitor.  You probably should not
   * use this unless you have a very good reason.
   */
  SwitchArg(const std::string& flag, 
            const std::string& name, 
            const std::string& desc,
            CmdLineInterface& parser,
            bool def = false,
            Visitor* v = NULL);
          
          
  /**
   * Handles the processing of the argument.
   * This re-implements the Arg version of this method to set the
   * _value of the argument appropriately.
   * \param i - Pointer the the current argument in the list.
   * \param args - Mutable list of strings. Passed
   * in from main().
   */
  virtual bool processArg(int* i, std::vector<std::string>& args); 

  /**
   * Checks a string to see if any of the chars in the string
   * match the flag for this Switch.
   */
  bool combinedSwitchesMatch(std::string& combined);

  /**
   * Returns bool, whether or not the switch has been set.
   */
  bool getValue();

};

//////////////////////////////////////////////////////////////////////
//BEGIN SwitchArg.cpp
//////////////////////////////////////////////////////////////////////
inline SwitchArg::SwitchArg(const std::string& flag, 
                            const std::string& name, 
                            const std::string& desc, 
                            bool _default,
                            Visitor* v )
  : Arg(flag, name, desc, false, false, v),
    _value( _default )
{ }

inline SwitchArg::SwitchArg(const std::string& flag, 
                            const std::string& name, 
                            const std::string& desc, 
                            CmdLineInterface& parser,
                            bool _default,
                            Visitor* v )
  : Arg(flag, name, desc, false, false, v),
    _value( _default )
{ 
  parser.add( this );
}

inline bool SwitchArg::getValue() { return _value; }

inline bool SwitchArg::combinedSwitchesMatch(std::string& combinedSwitches )
{
  // make sure this is actually a combined switch
  if ( combinedSwitches[0] != Arg::flagStartString()[0] )
    return false;

  // make sure it isn't a long name 
  if ( combinedSwitches.substr( 0, Arg::nameStartString().length() ) == 
       Arg::nameStartString() )
    return false;

  // ok, we're not specifying a ValueArg, so we know that we have
  // a combined switch list.  
  for ( unsigned int i = 1; i < combinedSwitches.length(); i++ )
    if ( combinedSwitches[i] == _flag[0] ) 
      {
      // update the combined switches so this one is no longer present
      // this is necessary so that no unlabeled args are matched
      // later in the processing.
      //combinedSwitches.erase(i,1);
      combinedSwitches[i] = Arg::blankChar(); 
      return true;
      }

  // none of the switches passed in the list match. 
  return false;  
}


inline bool SwitchArg::processArg(int *i, std::vector<std::string>& args)
{
  if ( _ignoreable && Arg::ignoreRest() )
    return false;

  if ( argMatches( args[*i] ) || combinedSwitchesMatch( args[*i] ) )
    {
    // If we match on a combined switch, then we want to return false
    // so that other switches in the combination will also have a
    // chance to match.
    bool ret = false;
    if ( argMatches( args[*i] ) )
      ret = true;

    if ( _alreadySet || ( !ret && combinedSwitchesMatch( args[*i] ) ) )
      throw(CmdLineParseException("Argument already set!", toString()));  

    _alreadySet = true;

    if ( _value == true )
      _value = false;
    else
      _value = true;

    _checkWithVisitor();

    return ret;
    }
  else
    return false;
}

//////////////////////////////////////////////////////////////////////
//End SwitchArg.cpp
//////////////////////////////////////////////////////////////////////

} //namespace TCLAP

#endif
