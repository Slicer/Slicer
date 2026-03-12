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
          qt6.qttools # includes UiTools
          qt6.qtwebengine
          qt6.qtwebchannel
          qt6.qt5compat # XMLPatterns moved here in Qt6
          qt6.wrapQtAppsHook

          # Required system libraries
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
            # Make Qt6 discoverable by CMake
            export QT_PLUGIN_PATH="${pkgs.qt6.qtbase}/${pkgs.qt6.qtbase.qtPluginPrefix}"

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
