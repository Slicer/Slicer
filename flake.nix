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
      in
      let
        slicerVersion = "5.10.0";
        slicerDir = "./downloads/Slicer-${slicerVersion}-linux-amd64";

        # Libraries required by the pre-built Slicer binary
        slicerLibs = with pkgs; [
          # X11 / xcb
          libice
          libsm
          libx11
          libxcb
          libxcb-errors
          libxcb-image
          libxcb-keysyms
          libxcb-render-util
          libxcb-util
          libxcb-wm
          libxcomposite
          libxcursor
          libxdamage
          libxext
          libxfixes
          libxi
          libxrandr
          libxrender
          libxscrnsaver
          libxtst

          # Graphics
          libGL
          libGLU
          vulkan-loader

          # Wayland
          wayland

          # System libs
          at-spi2-atk
          cairo
          cups
          dbus
          fontconfig
          freetype
          glib
          glib-networking
          gtk3
          libxkbcommon
          nspr
          nss
          pango
          pcre2
          zlib

          # Audio / multimedia
          alsa-lib
          libpulseaudio

          # Misc
          expat
          libffi
          libuuid
          xdg-utils

          # C++ runtime
          stdenv.cc.cc.lib
        ];

      in
      {
        devShell = pkgs.mkShell {
          packages = with pkgs; [
            pkg-config
            prek
            python312
            uv
          ];

          # Extend NIX_LD_LIBRARY_PATH so nix-ld finds all Slicer deps.
          # Fix sysconfigdata module name mismatch for Slicer's bundled Python.
          shellHook = ''
            export NIX_LD_LIBRARY_PATH="${pkgs.lib.makeLibraryPath slicerLibs}''${NIX_LD_LIBRARY_PATH:+:$NIX_LD_LIBRARY_PATH}"
            if [ -d /run/opengl-driver/lib ]; then
              export NIX_LD_LIBRARY_PATH="/run/opengl-driver/lib:$NIX_LD_LIBRARY_PATH"
            fi
            # Force uv/prek to use the Nix-provided Python instead of
            # downloading standalone builds that break on NixOS.
            export UV_PYTHON_PREFERENCE=only-system
          '';
        };
      }
    );
}
