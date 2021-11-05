
#-----------------------------------------------------------------------------
set(MODULE_NAME WebServer)

#-----------------------------------------------------------------------------
set(MODULE_PYTHON_SCRIPTS
  WebServer.py
  )

set(MODULE_PYTHON_RESOURCES
  Resources/Icons/WebServer.png
  Resources/docroot/index.html
  Resources/docroot/favicon.ico
  Resources/docroot/images/3DSlicer-DesktopIcon.png
  Resources/docroot/ServerTests/glMatrix-0.9.5.min.js
  Resources/docroot/ServerTests/index.html
  Resources/docroot/ServerTests/threeDCube.html
  Resources/docroot/ServerTests/timecube.html
  Resources/docroot/ServerTests/webgl-utils.js
  Resources/docroot/stylesheets/application.css
  )

#-----------------------------------------------------------------------------
slicerMacroBuildScriptedModule(
  NAME ${MODULE_NAME}
  SCRIPTS ${MODULE_PYTHON_SCRIPTS}
  RESOURCES ${MODULE_PYTHON_RESOURCES}
  WITH_GENERIC_TESTS
  )
