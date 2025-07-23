# krb5-gssapi-stub

## Overview

This stub library provides a minimal implementation of the GSSAPI symbol `GSS_C_NT_HOSTBASED_SERVICE`, which is expected by Qt's `libQt5Network.so`. It allows Slicer to run on Linux without installing or linking against the full Kerberos stack.

The stub is built only when:

* Slicer is **not using a system Qt** (`Slicer_USE_SYSTEM_QT` is OFF), and
* The `Qt5::Network` target reports the `gssapi` feature as enabled.

The resulting `libgssapi_krb5.so.2` is installed to the runtime library directory to satisfy the dynamic linker at application startup.

This situation commonly arises when Qt is installed via the official Qt installer or from a custom prefix, rather than built within the Slicer superbuild or installed system-wide via a package manager.

## Debugging

To verify that the stub library is being loaded at runtime, set the environment variable:

```bash
export SLICER_DEBUG_KRB5_GSSAPI_STUB=1
```

This will print the message:

```
[krb5-gssapi-stub] Loaded stub libgssapi_krb5.so.2
[krb5-gssapi-stub] Exported symbol: GSS_C_NT_HOSTBASED_SERVICE (stubbed)
```

during application startup.

## Background

When Qt is installed via the official installer (`qt-online-installer-linux-x64-online.run`), its `libQt5Network.so` library has a direct dependency on `libgssapi_krb5.so.2`. That library, in turn, depends on `libkrb5.so.3` and `libk5crypto.so.3`, which often link against a system OpenSSL version that may differ from the one bundled with Slicer.

To avoid OpenSSL version mismatches and unnecessary Kerberos dependencies, this stub builds a shared library named `libgssapi_krb5.so.2` exporting only the symbol `GSS_C_NT_HOSTBASED_SERVICE`.

This is a safe workaround because Slicer **does not use Kerberos/GSSAPI functionality**. HTTPS communication is handled via OpenSSL.

## Example: Qt's `libQt5Network.so` Dependencies

You can confirm the GSSAPI dependency using `readelf`:

```bash
$ readelf -d /path/to/Qt/5.15.2/gcc_64/lib/libQt5Network.so | gawk '/NEEDED/ {++n; print} END {printf "---- %d lines\n", n}'
 0x0000000000000001 (NEEDED)             Shared library: [libQt5Core.so.5]
 0x0000000000000001 (NEEDED)             Shared library: [libpthread.so.0]
 0x0000000000000001 (NEEDED)             Shared library: [libz.so.1]
 0x0000000000000001 (NEEDED)             Shared library: [libdl.so.2]
 0x0000000000000001 (NEEDED)             Shared library: [libgssapi_krb5.so.2]
 0x0000000000000001 (NEEDED)             Shared library: [libstdc++.so.6]
 0x0000000000000001 (NEEDED)             Shared library: [libm.so.6]
 0x0000000000000001 (NEEDED)             Shared library: [libgcc_s.so.1]
 0x0000000000000001 (NEEDED)             Shared library: [libc.so.6]
---- 9 lines
```
