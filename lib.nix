
{ pkgs }: {
	deps = [
		pkgs.clang
		pkgs.ccls
		pkgs.gdb
		pkgs.gnumake
		pkgs.opencv
		pkgs.pkg-config
		pkgs.unzip
	];
}
