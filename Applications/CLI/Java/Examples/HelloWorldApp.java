package Examples;
/**
 * The HelloWorldApp class implements an application that
 * simply prints "Hello World!" to standard output.
 */
class HelloWorldApp {
  public static void main(String[] args) {
    String Name = "World";
            
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
    }
    System.out.println("Hello " + Name + "!"); // Display the string.
   }
}
