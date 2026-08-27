# vs-opening
The program opens the visual studio 2019 splash screen ("ho_lu_shit.bmp") and creates 3d points from it.

The splash screen png was extracted from the Visual Studio 2019 using Resource Hacker.exe from the file 
`C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\IDE\1045\MsEnvMui_Brand_708_0.dll`, 
there was also a different image in the file `BlendMui_Brand_708_10000.dll` which i am appending as "blend.bmp".

Use WASD, shift and space to fly around. Scroll to change speed of flight.

F toggles fps.

E and Q change what exponent the distances from the camera's original position are raised to.

To leave press alt+f4.

# compiling
If you have Visual Studio installed you should have a shortcut called "x64 Native Tools Command Prompt for VS 2022" on your pc. 
Open that shortcut and navigate to the project's directory. Run `cr.bat`.