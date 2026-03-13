{
  description = "Slicer development environment";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs =
    {
      self,
      nixpkgs,
      flake-utils,
    }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = import nixpkgs { inherit system; };

        # ── USE_SYSTEM toggles ──────────────────────────────────────────
        # Set to true to use Nix-provided system packages instead of the
        # superbuild compiling them from source. Reduces build time but
        # may introduce version mismatches.
        #
        # Not toggleable (Slicer cmake FATAL_ERROR): CTK
        useSystem = {
          # Always-on: proven to work on NixOS
          python = true;

          # Low-risk: stable libraries unlikely to cause version issues
          zlib = true;
          curl = true;
          tbb = true;
          JsonCpp = true;
          RapidJSON = true;
          LibArchive = true;

          # Higher-risk: tightly coupled to Slicer's version expectations.
          # Slicer uses a custom VTK fork (SplineDrivenImageSlicer module)
          # and nixpkgs has VTK 9.5.2 while Slicer wants 9.6.0.
          VTK = true;
          ITK = false; # nixpkgs ITK lacks Slicer remote modules (MGHIO, IOScanco)
          DCMTK = false; # Dual DCMTK copies (system + ITK-bundled) cause crash on exit
        };

        # Helper: collect USE_SYSTEM cmake flags from the toggle map
        useSystemFlags = pkgs.lib.concatStringsSep " \\\n              " (
          pkgs.lib.mapAttrsToList (
            name: value: "-DSlicer_USE_SYSTEM_${name}:BOOL=${if value then "ON" else "OFF"}"
          ) useSystem
        );

        # Python 3.12 with packages required by the Slicer superbuild's
        # system-package checks when Slicer_USE_SYSTEM_python=ON.
        # Each python-* sub-project verifies its modules are importable.
        slicerPython = pkgs.python312.withPackages (
          ps: with ps; [
            # python-ensurepip / python-pip / python-setuptools / python-wheel
            pip
            setuptools
            wheel
            # python-numpy
            numpy
            # python-scipy
            scipy
            # python-SimpleITK
            simpleitk
            # python-pythonqt-requirements
            packaging
            pyparsing
            # python-dicom-requirements
            pydicom
            six
            pillow
            dicomweb-client
            # python-requests-requirements
            certifi
            idna
            chardet
            urllib3
            requests
            # python-extension-manager-ssl-requirements
            pyjwt
            wrapt
            deprecated
            pycparser
            cffi
            pynacl
            python-dateutil
            pygithub
            # python-extension-manager-requirements
            gitpython
            gitdb
            smmap
          ]
        );

        # VTK with Qt6 and Python wrapping enabled.
        # The default nixpkgs vtk has neither; vtkWithQt6 adds Qt but not
        # Python. We need both so that CTK can find WrappingPythonCore.
        slicerVtk = pkgs.vtkWithQt6.override {
          pythonSupport = true;
          python3Packages = pkgs.python312Packages;
        };

        # ITK must be built against slicerVtk so that ITKVtkGlue.cmake
        # hardcodes the same VTK store path (with Qt6 + Python wrapping).
        # Otherwise ITK overrides VTK_DIR to a plain VTK without those.
        slicerItk = (pkgs.itk.override { vtk = slicerVtk; }).overrideAttrs (old: {
          buildInputs = (old.buildInputs or [ ]) ++ [
            pkgs.qt6.qtbase # Widgets, Gui, OpenGL, Sql, OpenGLWidgets
            pkgs.qt6.qtdeclarative # Quick, Qml
          ];
          dontWrapQtApps = true; # ITK is a library, not an app
        });

        # ── Build-time dependencies ──────────────────────────────────
        # Tools and libraries needed to configure and compile Slicer
        # from source using the CMake superbuild.
        buildDeps = with pkgs; [
          # Build toolchain
          cmake
          gnumake
          gnupatch
          gcc
          git
          git-lfs

          # Qt6 (Slicer 5.11+ supports Qt6)
          qt6.qtbase
          qt6.qtmultimedia
          qt6.qtsvg
          qt6.qttools # includes UiTools, LinguistTools
          qt6.qtwebengine # WebEngineCore, WebEngineWidgets
          qt6.qtwebchannel
          qt6.qtdeclarative # Quick, QuickWidgets, Qml
          qt6.qtscxml # StateMachine
          qt6.qt5compat # Core5Compat (XMLPatterns moved here in Qt6)
          qt6.wrapQtAppsHook

          # Required system libraries
          libxt
          openssl
          slicerPython # Python 3.12 + all packages (USE_SYSTEM_python)

          # System libraries for USE_SYSTEM toggles
          zlib # also in runtimeDeps
          curl
          tbb # oneTBB (v2022.3.0 in nixpkgs matches superbuild pin)
          jsoncpp # v1.9.6 in nixpkgs matches superbuild pin
          rapidjson # header-only
          libarchive
          hdf5-cpp # HDF5 with C++ bindings (ITK_USE_SYSTEM_HDF5)

          # Build helpers
          pkg-config
          ninja
        ]
        # Conditionally include system packages for higher-risk toggles
        ++ pkgs.lib.optionals useSystem.VTK [ slicerVtk ]
        ++ pkgs.lib.optionals useSystem.ITK [ slicerItk ]
        ++ pkgs.lib.optionals useSystem.DCMTK [ pkgs.dcmtk ];

        # ── Runtime dependencies ─────────────────────────────────────
        # Libraries needed at runtime by the built Slicer application.
        # Separated for clarity; will also be needed in a future
        # derivation's buildInputs / propagatedBuildInputs.
        runtimeDeps = with pkgs; [
          # X11 / xcb
          libx11
          libxcb
          libxcb-wm # libxcb-icccm, libxcb-ewmh
          libxcb-image
          libxcb-keysyms
          libxcb-render-util
          libxcb-cursor
          libxcb-util
          libxext
          libxi
          libxrandr
          libxrender
          libxcomposite
          libxcursor
          libxdamage
          libxfixes
          libxtst
          libxscrnsaver
          libice
          libsm

          # Graphics
          libGL
          libGLU
          libgbm # QtWebEngine's Chromium needs GBM for EGL/native GPU rendering
          vulkan-loader

          # Wayland
          wayland
          qt6.qtwayland

          # System libs
          dbus
          fontconfig
          freetype
          glib
          libxkbcommon
          nss
          nspr
          pcre2
          zlib

          # Audio / multimedia
          alsa-lib
          libpulseaudio
          pipewire

          # C++ runtime
          stdenv.cc.cc.lib
        ];

        # ── NVIDIA workaround ───────────────────────────────────────
        # QtWebEngine's Chromium scans /proc/self/mem (reading 64 bytes
        # from every VMA). When the process has NVIDIA GPU mappings (from
        # VTK's OpenGL rendering), this triggers a NULL pointer dereference
        # in nvidia_vma_access (kernel module bug in driver 595.x with
        # Open Kernel Module on kernel 6.18+). The kernel oopses and
        # SIGKILL's the process.
        #
        # This LD_PRELOAD library intercepts pread/pread64 and returns EIO
        # for any fd pointing to /proc/self/mem. Chromium handles EIO
        # gracefully (some VMAs already return EIO normally).
        blockProcMem = pkgs.stdenv.mkDerivation {
          name = "block-proc-self-mem";
          dontUnpack = true;
          buildPhase = ''
            cat > block_proc_mem.c << 'CSRC'
            #define _GNU_SOURCE
            #include <dlfcn.h>
            #include <errno.h>
            #include <fcntl.h>
            #include <string.h>
            #include <unistd.h>
            #include <sys/types.h>
            #include <stdio.h>
            #include <stdarg.h>

            static int is_proc_self_mem(const char *path) {
                return path && strcmp(path, "/proc/self/mem") == 0;
            }

            static int is_proc_self_mem_fd(int fd) {
                char link[64], target[256];
                snprintf(link, sizeof(link), "/proc/self/fd/%d", fd);
                ssize_t len = readlink(link, target, sizeof(target) - 1);
                if (len > 0) { target[len] = '\0'; return is_proc_self_mem(target); }
                return 0;
            }

            /* Block open/openat for /proc/self/mem (catches libc callers) */
            int open(const char *path, int flags, ...) {
                if (is_proc_self_mem(path)) { errno = EACCES; return -1; }
                static int (*real)(const char *, int, ...) = NULL;
                if (!real) real = dlsym(RTLD_NEXT, "open");
                if (flags & (O_CREAT | O_TMPFILE)) {
                    va_list ap; va_start(ap, flags);
                    int mode = va_arg(ap, int); va_end(ap);
                    return real(path, flags, mode);
                }
                return real(path, flags);
            }

            int open64(const char *path, int flags, ...) {
                if (is_proc_self_mem(path)) { errno = EACCES; return -1; }
                static int (*real)(const char *, int, ...) = NULL;
                if (!real) real = dlsym(RTLD_NEXT, "open64");
                if (flags & (O_CREAT | O_TMPFILE)) {
                    va_list ap; va_start(ap, flags);
                    int mode = va_arg(ap, int); va_end(ap);
                    return real(path, flags, mode);
                }
                return real(path, flags);
            }

            int openat(int dirfd, const char *path, int flags, ...) {
                if (is_proc_self_mem(path)) { errno = EACCES; return -1; }
                static int (*real)(int, const char *, int, ...) = NULL;
                if (!real) real = dlsym(RTLD_NEXT, "openat");
                if (flags & (O_CREAT | O_TMPFILE)) {
                    va_list ap; va_start(ap, flags);
                    int mode = va_arg(ap, int); va_end(ap);
                    return real(dirfd, path, flags, mode);
                }
                return real(dirfd, path, flags);
            }

            int openat64(int dirfd, const char *path, int flags, ...) {
                if (is_proc_self_mem(path)) { errno = EACCES; return -1; }
                static int (*real)(int, const char *, int, ...) = NULL;
                if (!real) real = dlsym(RTLD_NEXT, "openat64");
                if (flags & (O_CREAT | O_TMPFILE)) {
                    va_list ap; va_start(ap, flags);
                    int mode = va_arg(ap, int); va_end(ap);
                    return real(dirfd, path, flags, mode);
                }
                return real(dirfd, path, flags);
            }

            /* Block pread on /proc/self/mem (catches raw-syscall openers) */
            ssize_t pread(int fd, void *buf, size_t count, off_t offset) {
                if (is_proc_self_mem_fd(fd)) { errno = EIO; return -1; }
                static ssize_t (*real)(int, void *, size_t, off_t) = NULL;
                if (!real) real = dlsym(RTLD_NEXT, "pread");
                return real(fd, buf, count, offset);
            }

            ssize_t pread64(int fd, void *buf, size_t count, off_t offset) {
                if (is_proc_self_mem_fd(fd)) { errno = EIO; return -1; }
                static ssize_t (*real)(int, void *, size_t, off_t) = NULL;
                if (!real) real = dlsym(RTLD_NEXT, "pread64");
                return real(fd, buf, count, offset);
            }
            CSRC
            $CC -shared -fPIC -o block_proc_mem.so block_proc_mem.c -ldl
          '';
          installPhase = ''
            mkdir -p $out/lib
            cp block_proc_mem.so $out/lib/
          '';
        };

        # ── Development extras ───────────────────────────────────────
        # Additional tools useful during Slicer development but not
        # strictly required for the build itself.
        devDeps = with pkgs; [
          ccache
          gdb
          uv

          # Wrapper script that invokes cmake with NixOS-specific flags.
          # Uses a bash script rather than a shell alias so that flags are
          # word-split correctly in any shell (fish, zsh, bash, etc.).
          # Flags are generated from the useSystem toggle map above.
          (writeShellScriptBin "slicer-cmake" ''
            exec cmake \
              ${useSystemFlags} \
              -DSlicer_BUILD_MULTIMEDIA_SUPPORT:BOOL=OFF \
              "$@"
          '')
        ];

        # # ── Pre-built binary runtime libs (commented out) ──────────
        # # These were previously used with nix-ld to run pre-built
        # # Slicer binaries. Kept here for reference in case we need
        # # to restore pre-built binary support.
        # prebuiltExtraLibs = with pkgs; [
        #   libxcb-errors
        #   libxcb-image
        #   libxcb-keysyms
        #   libxcb-render-util
        #   libxcb-util
        #   libxcb-wm
        #   at-spi2-atk
        #   cairo
        #   cups
        #   glib-networking
        #   gtk3
        #   pango
        #   pcre2
        #   expat
        #   libffi
        #   libuuid
        #   xdg-utils
        # ];

      in
      {
        devShells.default = pkgs.mkShell {
          packages = buildDeps ++ runtimeDeps ++ devDeps;

          # Runtime library path for superbuild binaries (not nix-wrapped)
          LD_LIBRARY_PATH = pkgs.lib.makeLibraryPath runtimeDeps;

          shellHook = ''
            # Bridge Nix's cmake prefix path so that cmake invoked
            # manually in the shell can find Qt6 and other dependencies.
            export CMAKE_PREFIX_PATH="$NIXPKGS_CMAKE_PREFIX_PATH''${CMAKE_PREFIX_PATH:+:$CMAKE_PREFIX_PATH}"

            # Use 'slicer-cmake' instead of 'cmake' to configure Slicer.
            # It passes NixOS-specific USE_SYSTEM flags generated from the
            # useSystem toggle map in flake.nix. Edit the toggles at the
            # top of the flake to switch between system and superbuild.
            #
            # Example:
            #   slicer-cmake ../../src/Slicer

            # Enable ccache for C/C++ compilation. CMake will use these
            # as compiler launchers, speeding up subsequent rebuilds.
            export CMAKE_C_COMPILER_LAUNCHER=ccache
            export CMAKE_CXX_COMPILER_LAUNCHER=ccache

            # Ensure GPU drivers are available at runtime on NixOS.
            # LD_LIBRARY_PATH: OpenGL driver .so files for VTK rendering.
            # XDG_DATA_DIRS: Vulkan ICD manifests for QtWebEngine's Chromium.
            if [ -d /run/opengl-driver ]; then
              export LD_LIBRARY_PATH="/run/opengl-driver/lib''${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
              export XDG_DATA_DIRS="/run/opengl-driver/share''${XDG_DATA_DIRS:+:$XDG_DATA_DIRS}"
            fi

            # QtWebEngine's process binary, resources, and locales are in a
            # separate nix store path from qtbase. Without these env vars,
            # QtWebEngine can't find its Chromium subprocess or .pak files.
            export QTWEBENGINEPROCESS_PATH="${pkgs.qt6.qtwebengine}/libexec/QtWebEngineProcess"
            export QTWEBENGINE_RESOURCES_PATH="${pkgs.qt6.qtwebengine}/resources"
            export QTWEBENGINE_LOCALES_PATH="${pkgs.qt6.qtwebengine}/qtwebengine_locales"

            # Work around NVIDIA driver bug: nvidia_vma_access NULL deref
            # when Chromium scans /proc/self/mem over NVIDIA GPU mappings.
            # The LD_PRELOAD intercepts pread on /proc/self/mem (returns EIO).
            # --disable-gpu prevents Chromium from using GPU (VTK unaffected).
            # --no-zygote avoids multi-process setup that also reads /proc/*/mem.
            export LD_PRELOAD="${blockProcMem}/lib/block_proc_mem.so''${LD_PRELOAD:+:$LD_PRELOAD}"
            export QTWEBENGINE_CHROMIUM_FLAGS="--disable-gpu --no-zygote"

            # Force uv to use the Nix-provided Python instead of
            # downloading standalone builds that break on NixOS.
            export UV_PYTHON_PREFERENCE=only-system
          '';
        };
      }
    );
}
