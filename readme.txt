Only main and WndProc function is in one page.
To help understanding directX9 functions, the project doesn't have any error check statement.
You can download DirectX9 from https://www.microsoft.com/en-us/download/details.aspx?id=8109

Setting
Properties > Configuration Properties > VC++ Directories > Include Directories > ~\Microsoft DirectX SDK (June 2010)\Include;
Properties > Configuration Properties > VC++ Directories > Library Directories > ~\Microsoft DirectX SDK (June 2010)\Lib\x86;
Properties > Configuration Properties > Linker > Input > d3d9.lib;d3dx9.lib;winmm.lib;
Properties > Configuration Properties > Advanced > Character Set > Not Set
Properties > Configuration Properties > Linker > System > Subsystem > Not Set
Properties > Configuration Properties > C/C++ > General > SDL checks > No(/sdl-)

