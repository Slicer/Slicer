package Examples;
/**
 * The HelloWorldApp class implements an application that
 * prints "Hello Name!" to standard output.
 * Also returns a string with the greeting via a file.
 */
import java.io.*;

class HelloWorldApp {
  public static void main(String[] args) {
    String Name = "World";
    String ReturnFileName = "";
            
    for (int i = 0; i < args.length; i++)
       {
        //System.out.println("\targ " + i + " = " + args[i]);
       if (args[i].endsWith("xml"))
         {
         //System.out.println("Got the xml arg");
// print out something Slicer can understand
         System.out.println("<?xml version=\"1.0\" encoding=\"utf-8\"?>");
         System.out.println("<executable>");
         System.out.println("<category>Developer Tools</category>");
         System.out.println("<title>Java Example</title>");
         System.out.println("<description>");
         System.out.println("Prints out Hello with your name, World if not passed");
         System.out.println("</description>");
         System.out.println("<version>0.1.0.$Revision: 10200 $(alpha)</version>");
         System.out.println("<documentation-url>http://www.slicer.org/slicerWiki/index.php/Modules:JavaExample-Documentation-3.5</documentation-url>");
         System.out.println("<license></license>");
         System.out.println("<contributor>Nicole Aucoin</contributor>");
         System.out.println("<acknowledgements>This work is part of the National Alliance for Medical Image Computing (NAMIC), funded by the National Institutes of Health through the NIH Roadmap for Medical Research, Grant U54 EB005149.</acknowledgements>");
         System.out.println("<parameters>");
         System.out.println("\t<label>IO</label>");
         System.out.println("\t<description>Input/output parameters</description>");
         System.out.println("\t<string>");
         System.out.println("\t\t<name>name</name>");
         System.out.println("\t\t<label>Your Name</label>");
         System.out.println("\t\t<default>World</default>");
         System.out.println("\t\t<longflag>--name</longflag>");
         System.out.println("\t\t<description>Enter your name</description>");
         System.out.println("\t</string>");
         System.out.println("\t<string>");
         System.out.println("\t\t<name>helloReturn</name>");
         System.out.println("\t\t<label>Your greeting:</label>");
         System.out.println("\t\t<channel>output</channel>");
         System.out.println("\t\t<default>Hello World</default>");
         System.out.println("\t\t<description>The return greeting from the program, a string</description>");
         System.out.println("\t</string>");

         System.out.println("</parameters>");
         System.out.println("</executable>");
         System.exit(0);
      }
      else if (args[i].endsWith("--name"))
      {
      Name = args[++i];
      }
      else if (args[i].startsWith("--name"))
      {
       Name = args[i].substring(7);
      }
      else if (args[i].endsWith("--returnparameterfile"))
        {
        ReturnFileName = args[++i];
        }
      else if (args[i].startsWith("--returnparameterfile"))
        {
        ReturnFileName = args[i].substring(21);
        }
    }
    System.out.println("Hello " + Name + "!"); // Display the string.
    // return the string
    // System.out.println("Writing to return file '" + ReturnFileName + "'");
    try
      {
      // Create file 
      FileWriter fstream = new FileWriter(ReturnFileName);
      BufferedWriter out = new BufferedWriter(fstream);
      out.write("helloReturn = Hello  " + Name + "!");
      //Close the output stream
      out.close();
      }
    catch (Exception e)
      {
      //Catch exception if any
      System.err.println("Error: " + e.getMessage());
      }
   }
}
