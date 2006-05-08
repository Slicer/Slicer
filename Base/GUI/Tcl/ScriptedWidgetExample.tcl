
proc ScriptedWidgetExampleInitialize {this frame} {

    set ::ScriptedWidgetExample(this) $this

    button $frame.button -text "scripted widget" -command ScriptedWidgetExampleButtonCallback
    pack $frame.button
}

proc ScriptedWidgetExampleButtonCallback {} {

    tk_messageBox -message [$::ScriptedWidgetExample(this) Print]
}
