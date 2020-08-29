# Common errors

## Firewall is blocking git protocol

Some firewalls will block the git protocol. A possible workaround is to configure Slicer by disabling the option `Slicer_USE_GIT_PROTOCOL`. Then the http protocol will be used instead. Consider also reading https://github.com/commontk/CTK/issues/33.
