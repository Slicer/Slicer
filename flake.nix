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
          DCMTK = true;
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

        # ── Development extras ───────────────────────────────────────
        # Additional tools useful during Slicer development but not
        # strictly required for the build itself.
        devDeps = with pkgs; [
          ccache
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

            # Ensure OpenGL drivers are available at runtime on NixOS
            if [ -d /run/opengl-driver/lib ]; then
              export LD_LIBRARY_PATH="/run/opengl-driver/lib''${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
            fi

            # Force uv to use the Nix-provided Python instead of
            # downloading standalone builds that break on NixOS.
            export UV_PYTHON_PREFERENCE=only-system
          '';
        };
      }
    );
}
