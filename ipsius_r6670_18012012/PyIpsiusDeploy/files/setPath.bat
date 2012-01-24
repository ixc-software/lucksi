set PATH=%~dp0\bin

if exist .\bin\Microsoft.VC80.DebugCRT goto _try_copy_debug
if exist .\bin\Microsoft.VC80.CRT      goto _try_copy_release

goto _exit

rem --------------------------------------------

:_try_copy_debug

    if not exist .\bin\imageformats\Microsoft.VC80.DebugCRT goto _copy_debug

    goto _exit
    
:_copy_debug

    md .\bin\imageformats\Microsoft.VC80.DebugCRT
    copy bin\Microsoft.VC80.DebugCRT\*.* bin\imageformats\Microsoft.VC80.DebugCRT\*.*

    goto _exit

rem --------------------------------------------
    
:_try_copy_release

    if not exist .\bin\imageformats\Microsoft.VC80.CRT goto _copy_release

    goto _exit
    
:_copy_release

    md .\bin\imageformats\Microsoft.VC80.CRT
    copy bin\Microsoft.VC80.CRT\*.* bin\imageformats\Microsoft.VC80.CRT\*.*

    goto _exit
    

rem --------------------------------------------
    
:_exit

    echo
    
    