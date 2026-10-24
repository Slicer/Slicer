# Common errors

## Firewall is blocking git protocol

Some firewalls will block the git protocol. A possible workaround is to configure Slicer by disabling the option `Slicer_USE_GIT_PROTOCOL`. Then the http protocol will be used instead. Consider also reading https://github.com/commontk/CTK/issues/33.

## CMake complains during configuration

CMake may not directly show what's wrong; try to look for log files of the form BUILD/CMakeFiles/*.log (where BUILD is your build directory) to glean further information.

## 'QSslSocket' : is not a class or namespace name

This error message occurs if Slicer is configured to use SSL but Qt is built without SSL support.

Either set Slicer_USE_PYTHONQT_WITH_OPENSSL to OFF when configuring Slicer build in CMake, or build Qt with SSL support.

## Apple Silicon development launcher passes the wrong architecture

In affected Apple Silicon development builds, the Intel launcher running under Rosetta can pass the wrong architecture to child processes.
Symptoms reported in [issue #9121](https://github.com/Slicer/Slicer/issues/9121) include `os.system("arch")` printing `i386` and a Python `ctypes` failure reporting `objc_msgSendSuper_stret: symbol not found`.

As a workaround, generate an execution wrapper and use it to launch the native application:

```sh
"<Slicer_BUILD>/Slicer" --launcher-generate-exec-wrapper-script
bash "<generated-wrapper-path>" "<Slicer_BUILD>/bin/Slicer.app/Contents/MacOS/Slicer"
```

Replace `<Slicer_BUILD>` with your inner Slicer build directory (for example, `/opt/s2/Slicer-build`) and `<generated-wrapper-path>` with the shell script path reported by the first command.
The issue reports `arm64` from `arch` when using this wrapper.
This workaround applies to affected development builds; it does not resolve the native launcher implementation issue tracked in #9121.

# Other errors

If you encounter any other error then you can ask help on the [Slicer forum](https://discourse.slicer.org). Post your question in `Developer` category, use the `build` tag.

Usually seeing the full build log is needed for investigating the issue. The full build log can be found in the build tree, its exact location depend on the operating system and how Slicer was built, so the easiest way to find it is to look for large `*.log` files in the build tree. For example, you may find the full build log in `<Slicer_BUILD>\Testing\Temporary\LastBuild_<date>-<time>.log`. Large amount of text cannot be included in forum posts, therefore it is recommended to upload the build log to somewhere (Dropbox, OneDrive, Google drive, etc.) and add the download link to the forum post.
