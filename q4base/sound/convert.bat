SET PATH=D:\projects\Quake4\tools\freac-1.0.32-bin

for /R D:\projects\Quake4\q4base\sound %%G IN (*.ogg) do (
	freaccmd -e WAVE %%G -o %%~pG%%~nG.wav
	del %%G )

pause