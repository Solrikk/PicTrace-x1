{ pkgs }: {
  deps = [
    pkgs.clang
    pkgs.ccls
    pkgs.gdb
    pkgs.gnumake
    pkgs.opencv
    pkgs.pkg-config
    pkgs.unzip
    pkgs.spdlog.dev
    pkgs.qt6.qtbase
    pkgs.qt6.qttools
  ];
}
