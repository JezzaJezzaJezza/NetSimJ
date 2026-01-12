{pkgs ? import <nixpkgs> {}}:
(pkgs.mkShell.override {stdenv = pkgs.clangStdenv; }) {
  nativeBuildInputs = with pkgs; [
    cmake
    ninja
    clang-tools
  ];
}
