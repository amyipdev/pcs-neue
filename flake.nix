# SPDX-License-Identifier: GPL-2.0-or-later
{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };
  outputs = {self, nixpkgs, flake-utils}:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs {
          inherit system;
        };
      in {
	packages.buddyinfo = pkgs.rustPlatform.buildRustPackage rec {
          name = "buddyinfo";
          src = ./buddyinfo;
          cargoHash = "sha256-D8cGsDRTcP9YECADux1PdTGqRASIg7jmIiug9vuoKmA=";
          doCheck = false;
        };
        packages.pcsbench = pkgs.stdenv.mkDerivation {
          name = "pcsbench";
          src = ./pcsbench;
          buildInputs = with pkgs; [
            glib
            pkg-config
          ];
          installPhase = ''
            mkdir -p $out/bin
	    cp pcsbench $out/bin/pcsbench
          '';
        };
      }
    );
}
