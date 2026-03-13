SETLOCAL

@REM set PATH=C:\Windows\system32;C:\Windows;C:\Windows\System32\Wbem;C:\Windows\System32\WindowsPowerShell\v1.0\;C:\Windows\System32\OpenSSH\;C:\Users\justi\dev\revlens_root\thirdbase\20_04\Windows-AMD64-10\Git\2.26.2\Git\cmd;C:\Wildbrain\thirdparty\20.07\win64\python\2.7.18;C:\Wildbrain\thirdparty\20.07\win64\python\2.7.18\DLLs;C:\Users\justi\AppData\Local\Microsoft\WindowsApps;C:\Users\justi\AppData\Local\Programs\Microsoft VS Code\bin;C:\Wildbrain\thirdparty\20.07\win64\wenvlib\global\bin;
set PATH=C:\Users\justi\dev\revlens_root\thirdbase\20_04\Windows-AMD64-10\Python\3.7.7\libs;%PATH%
set PATH=C:\Users\justi\dev\revlens_root\thirdbase\20_04\Windows-AMD64-10\Python\3.7.7;%PATH%
set PATH=C:\Users\justi\dev\revlens_root\thirdbase\20_04\Windows-AMD64-10\PythonQt\Qt_5.14.2_Py_3.7\lib;%PATH%
set PATH=C:\Users\justi\dev\revlens_root\thirdbase\20_04\Windows-AMD64-10\Qt\5.14.2\msvc2017_64\bin;%PATH%
set PATH=C:\Users\justi\dev\revlens_root\sp_revlens\inst\Windows-AMD64-10\core\_projects\core\global\_build\lib;%PATH%
set PATH=C:\Users\justi\dev\revlens_root\sp_revlens\inst\Windows-AMD64-10\edb\_projects\edb\global\_build\lib;%PATH%
set PATH=C:\Users\justi\dev\revlens_root\sp_revlens\inst\Windows-AMD64-10\gui\_projects\gui\global\_build\lib;%PATH%
set PATH=C:\Users\justi\dev\revlens_root\sp_revlens\inst\Windows-AMD64-10\revlens\_projects\revlens\global\_build\lib;%PATH%
set PATH=C:\Users\justi\dev\revlens_root\sp_revlens\inst\Windows-AMD64-10\prod\_projects\prod\global\_build\lib;%PATH%
set PATH=C:\Users\justi\dev\revlens_root\sp_revlens\inst\Windows-AMD64-10\extrevlens\global\lib;%PATH%

set PYTHONPATH=C:\Users\justi\dev\revlens_root\thirdbase\20_04\Windows-AMD64-10\scaffold\2.4.1\lib\python3.7\site-packages;
set PYTHONPATH=C:\Users\justi\dev\revlens_root\sp_revlens\inst\Windows-AMD64-10\core\global\lib\python3.7\site-packages;%PYTHONPATH%
set PYTHONPATH=C:\Users\justi\dev\revlens_root\sp_revlens\inst\Windows-AMD64-10\revlens\global\lib\python3.7\site-packages;%PYTHONPATH%
@REM set PYTHONPATH=C:\Users\justi\dev\revlens_root\sp_revlens\inst\Windows-AMD64-10\extrevlens\global\lib\python3.7\site-packages;%PYTHONPATH%
set PYTHONPATH=C:\Users\justi\dev\revlens_root\sp_revlens\inst\Windows-AMD64-10\extrevlens\_projects\rlplug_qtbuiltin\global\_build\lib\python3.7\site-packages;%PYTHONPATH%

@REM for nodegraph
set PYTHONPATH=C:\Users\justi\dev\revlens_root\sp_revlens\inst\Windows-AMD64-10\extrevlens\global\lib\python3.7\site-packages;%PYTHONPATH%

set REVLENS_PATH=C:\Users\justi\dev\revlens_root\sp_revlens\inst\Windows-AMD64-10\extrevlens\global

@REM C:\Users\justi\dev\revlens_root\sp_revlens\inst\Windows-AMD64-10\edb\global\bin\edb_view.exe production.db pdb_test production_db

@REM C:\Users\justi\dev\revlens_root\sp_revlens\inst\Windows-AMD64-10\edb\global\bin\edb_dump_schema.exe production.db

C:\Users\justi\dev\revlens_root\sp_revlens\inst\Windows-AMD64-10\edb\global\bin\edb_dump_data.exe production.db

ENDLOCAL