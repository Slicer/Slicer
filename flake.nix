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
          libffi # Use system libffi to avoid GCC 15 asm compat issue
          libxt
          openssl

          # Build helpers
          pkg-config
          ninja
        ];

        # ── Runtime dependencies ─────────────────────────────────────
        # Libraries needed at runtime by the built Slicer application.
        # Separated for clarity; will also be needed in a future
        # derivation's buildInputs / propagatedBuildInputs.
        runtimeDeps = with pkgs; [
          # X11 / xcb
          libx11
          libxcb
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

          # System libs
          dbus
          fontconfig
          freetype
          glib
          libxkbcommon
          nss
          nspr
          zlib

          # Audio / multimedia
          alsa-lib
          libpulseaudio

          # C++ runtime
          stdenv.cc.cc.lib
        ];

        # ── Development extras ───────────────────────────────────────
        # Additional tools useful during Slicer development but not
        # strictly required for the build itself.
        devDeps = with pkgs; [
          ccache
          python312
          uv
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

          shellHook = ''
            # Bridge Nix's cmake prefix path so that cmake invoked
            # manually in the shell can find Qt6 and other dependencies.
            export CMAKE_PREFIX_PATH="$NIXPKGS_CMAKE_PREFIX_PATH''${CMAKE_PREFIX_PATH:+:$CMAKE_PREFIX_PATH}"

            # Use system libffi to avoid GCC 15 assembly syntax
            # incompatibility in the superbuild's bundled libffi.
            export cmakeFlags="-DSlicer_USE_SYSTEM_LibFFI:BOOL=ON"

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
