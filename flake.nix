{
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";

  outputs = { nixpkgs, ... }:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs {
        inherit system;
        config.allowUnfree = true;
      };

      commonBuildInputs = with pkgs; [
        cmake
        ninja
        python315
      ];
    in {
      devShells.${system} = {
        default =
          (pkgs.mkShell.override { stdenv = pkgs.clangStdenv; }) {
            nativeBuildInputs = commonBuildInputs ++ (with pkgs; [
              clang-tools
            ]);
          };

        cuda =
          pkgs.mkShell {
            nativeBuildInputs = commonBuildInputs ++ (with pkgs; [
              cudaPackages.cuda_nvcc
              cudaPackages.cuda_cudart
            ]);
          };
      };
    };
}
