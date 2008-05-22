  if (argc >= 2 && (strcmp(argv[1],"--xml") == 0))
    {
std::cout << "<?xml version=\"1.0\" encoding=\"utf-8\"?>" << std::endl;
std::cout << "<executable>" << std::endl;
std::cout << "  <category>utility</category>" << std::endl;
std::cout << "  <title>NAMIC Command Line Processing Code Generator</title>" << std::endl;
std::cout << "  <description>Generates C++ code that will parse command lines</description>" << std::endl;
std::cout << "  <version>1.0</version>" << std::endl;
std::cout << "  <documentation-url></documentation-url>" << std::endl;
std::cout << "  <license></license>" << std::endl;
std::cout << "  <contributor>Bill Lorensen</contributor>" << std::endl;
std::cout << "" << std::endl;
std::cout << "  <parameters>" << std::endl;
std::cout << "    <label>Parameters</label>" << std::endl;
std::cout << "    <description>Parameters used for command line processing</description>" << std::endl;
std::cout << "    <boolean>" << std::endl;
std::cout << "      <name>UseTCLAP</name>" << std::endl;
std::cout << "      <longflag>--TCLAP</longflag>" << std::endl;
std::cout << "      <description>Generate TCLAP Code</description>" << std::endl;
std::cout << "      <label>Generate TCLAP Code</label>" << std::endl;
std::cout << "      <default>true</default>" << std::endl;
std::cout << "    </boolean>" << std::endl;
std::cout << "  </parameters>" << std::endl;
std::cout << "  <parameters>" << std::endl;
std::cout << "    <label>IO</label>" << std::endl;
std::cout << "    <description>Input/Output parameters</description>" << std::endl;
std::cout << "    <file>" << std::endl;
std::cout << "      <name>InputXML</name>" << std::endl;
std::cout << "      <label>Input XML</label>" << std::endl;
std::cout << "      <channel>input</channel>" << std::endl;
std::cout << "      <longflag>--InputXML</longflag>" << std::endl;
std::cout << "      <description>XML description of interface</description>" << std::endl;
std::cout << "    </file>" << std::endl;
std::cout << "    <file>" << std::endl;
std::cout << "      <name>OutputCxx</name>" << std::endl;
std::cout << "      <label>Output C++</label>" << std::endl;
std::cout << "      <channel>output</channel>" << std::endl;
std::cout << "      <longflag>--OutputCxx</longflag>" << std::endl;
std::cout << "      <description>C++ Code to process command line arguments</description>" << std::endl;
std::cout << "    </file>" << std::endl;
std::cout << "  </parameters>" << std::endl;
std::cout << "</executable>" << std::endl;
std::cout << "" << std::endl;
    return EXIT_SUCCESS;
    }
    bool UseTCLAP = true;
    std::string InputXML;
    std::string OutputCxx;
    bool echoSwitch = false;
    bool xmlSwitch = false;
try
  {
    TCLAP::CmdLine commandLine (
      "Generates C++ code that will parse command lines",
      ' ',
      "$Revision: $" );

      itksys_ios::ostringstream msg;
    msg.str("");msg << "Generate TCLAP Code (default: " << UseTCLAP << ")";
    TCLAP::SwitchArg UseTCLAPArg("", "TCLAP", msg.str(), commandLine, 0);

    msg.str("");msg << "XML description of interface";    TCLAP::ValueArg<std::string> InputXMLArg("", "InputXML", msg.str(), 0, InputXML, "std::string", commandLine);

    msg.str("");msg << "C++ Code to process command line arguments";    TCLAP::ValueArg<std::string> OutputCxxArg("", "OutputCxx", msg.str(), 0, OutputCxx, "std::string", commandLine);

    msg.str("");msg << "Echo the command line arguments (default: " << echoSwitch << ")";
    TCLAP::SwitchArg echoSwitchArg("", "echo", msg.str(), commandLine, 0);

    msg.str("");msg << "Produce xml description of command line arguments (default: " << xmlSwitch << ")";
    TCLAP::SwitchArg xmlSwitchArg("", "xml", msg.str(), commandLine, 0);

    commandLine.parse ( argc, (char**) argv );
    UseTCLAP = UseTCLAPArg.getValue();
    InputXML = InputXMLArg.getValue();
    OutputCxx = OutputCxxArg.getValue();
    echoSwitch = echoSwitchArg.getValue();
    xmlSwitch = xmlSwitchArg.getValue();
  }
catch ( TCLAP::ArgException e )
  {
  std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
  exit ( EXIT_FAILURE );
    }
if (echoSwitch)
{
std::cout << "Command Line Arguments" << std::endl;
std::cout << "    UseTCLAP: " << UseTCLAP << std::endl;
std::cout << "    InputXML: " << InputXML << std::endl;
std::cout << "    OutputCxx: " << OutputCxx << std::endl;
std::cout << "    echoSwitch: " << echoSwitch << std::endl;
std::cout << "    xmlSwitch: " << xmlSwitch << std::endl;
}
