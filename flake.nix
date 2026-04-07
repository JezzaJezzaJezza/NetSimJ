{
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";

  outputs = { nixpkgs, ... }:
    let
      system = "x86_64-linux";
      pkgs = nixpkgs.legacyPackages.${system};
    in {
      devShells.${system}.default =
        (pkgs.mkShell.override { stdenv = pkgs.clangStdenv; }) {
          nativeBuildInputs = with pkgs; [
            cmake
            ninja
            clang-tools
            python315
          ];
        };

      devShells.${system}.cuda =
        pkgs.mkShell {
          nativeBuildInputs = with pkgs; [
            cmake
            ninja
            python315
            cudaPackages.cuda_nvcc
            cudaPackages.cuda_cudart
          ];
        };
    };
}
