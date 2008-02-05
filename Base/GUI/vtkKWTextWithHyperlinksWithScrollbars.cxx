#include "vtkKWTextWithHyperlinksWithScrollbars.h"
#include "vtkObjectFactory.h"
#include "vtkKWText.h"
#include <sstream>

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWTextWithHyperlinksWithScrollbars);
vtkCxxRevisionMacro(vtkKWTextWithHyperlinksWithScrollbars, "$Revision: 0.1 $");
vtkKWTextWithHyperlinksWithScrollbars::vtkKWTextWithHyperlinksWithScrollbars(void)
{
}

vtkKWTextWithHyperlinksWithScrollbars::~vtkKWTextWithHyperlinksWithScrollbars(void)
{
}
void vtkKWTextWithHyperlinksWithScrollbars::SetText(const char *s)
{
    std::string text;
    text=s;
    this->GetWidget()->SetText("");
    while(text.size()!=0)
    {
        int begin=text.find("<a>");
        int end=text.find("</a>");
        if(begin!=-1&&end!=-1)
        {
            std::string hyperlink;
            std::string append;
            append=text.substr(0,begin);
            hyperlink=text.substr(begin+3,end-(begin+3));
            text=text.substr(end+4,text.length());
            this->GetWidget()->AppendText(append.c_str());
            if(hyperlink.size()!=0)
            {
                this->Script("%s insert end \"%s\" hyperlink",this->GetWidget()->GetWidgetName(),hyperlink.c_str());
            }
        }
        else
        {
            this->GetWidget()->AppendText(text.c_str());
            break;
        }
    }


}
void vtkKWTextWithHyperlinksWithScrollbars::OpenLink(const char *url)
{
    this->Script("\
\
    if { \"%s\" != \"\" } {\n\
        if { $::tcl_platform(os) == \"Darwin\" } {\n\
            #--- on mac you can easily open in any browser,\n\
            #--- but bookmarks will only be saved in Firefox Bookmark file format.\n\
            #--- so to use the bookmarks, you'll have to use firefox later....\n\
            exec open $s\n\
        } else {\n\
            set browser [ $::slicer3::Application GetWebBrowser ]\n\
\
            if { ![file executable $browser] } {\n\
                set dialog [vtkKWMessageDialog New]\n\
                $dialog SetParent [$::slicer3::ApplicationGUI GetMainSlicerWindow]\n\
                $dialog SetStyleToMessage\n\
                $dialog SetText \"Please use the following dialog to set the path to the Firefox Browser and then re-run your query.\"\n\
                $dialog Create\n\
                $dialog Invoke\n\
                $dialog Delete\n\
\
                set window [$::slicer3::ApplicationGUI GetMainSlicerWindow]\n \
                set interface [$window GetApplicationSettingsInterface] \n\
                $interface Show\n\
                set manager [$interface GetUserInterfaceManager]\n\
                $manager RaiseSection 0 \"Slicer Settings\"\n\
                return\n\
            } else {\n\
                #--- For Windows or Linux builds:\n\
                exec $browser -new-tab %s &\n\
            }\n\
            \
        }\n\
    }\n\
        ",url,url,url);
}

void vtkKWTextWithHyperlinksWithScrollbars::CreateWidget(void)
{
    Superclass::CreateWidget();
    this->Script("%s tag bind hyperlink <Enter> {%s configure -cursor hand2}",this->GetWidget()->GetWidgetName(),this->GetWidget()->GetWidgetName());
    this->Script("%s tag bind hyperlink <Leave> {%s configure -cursor xterm}",this->GetWidget()->GetWidgetName(),this->GetWidget()->GetWidgetName());
    this->Script("%s tag bind hyperlink <Button-1> {%s ClickLink %%x %%y}",this->GetWidget()->GetWidgetName(),this->GetTclName());
    this->Script("%s tag config hyperlink -underline 1",this->GetWidget()->GetWidgetName());
    this->Script("%s tag config hyperlink -foreground #0000FF",this->GetWidget()->GetWidgetName());

}

void vtkKWTextWithHyperlinksWithScrollbars::ClickLink(int x, int y)
{
    this->Script(" set i [%s index @%i,%i]",this->GetWidget()->GetWidgetName(),x,y);
    this->Script(" set range [ %s tag prevrange hyperlink $i]",this->GetWidget()->GetWidgetName());
    this->Script(" set url [eval %s get $range]",this->GetWidget()->GetWidgetName());
    this->Script("%s OpenLink $url",this->GetTclName());



}
