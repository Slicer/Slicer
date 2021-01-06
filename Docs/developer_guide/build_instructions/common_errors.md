# Common errors

## Firewall is blocking git protocol

Some firewalls will block the git protocol. A possible workaround is to configure Slicer by disabling the option `Slicer_USE_GIT_PROTOCOL`. Then the http protocol will be used instead. Consider also reading https://github.com/commontk/CTK/issues/33.

## CMake complains during configuration

CMake may not directly show what's wrong; try to look for log files of the form BUILD/CMakeFiles/*.log (where BUILD is your build directory) to glean further information.

## 'QSslSocket' : is not a class or namespace name

This error message occurs if Slicer is configured to use SSL but Qt is built without SSL support.

Either set Slicer_USE_PYTHONQT_WITH_OPENSSL to OFF when configuring Slicer build in CMake, or build Qt with SSL support.
