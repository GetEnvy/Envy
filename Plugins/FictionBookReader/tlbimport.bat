cd ..
cd ..
cd ..
echo %CD%
"%ProgramFiles%\Microsoft SDKs\Windows\v6.0A\bin\tlbimp.exe" "..\..\Envy\Win32 %1\Envy.tlb" /out:Envy.Interop.dll /namespace:Envy /asmversion:1.0.0.0 /keyfile:Envy.snk /nologo /primary /sysarray /machine:x86 /transform:DispRet