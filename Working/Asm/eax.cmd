@rem

bin\nasm -f win32 eax_0.asm && bin\ld -e _start eax_0.obj -o eax_0.exe && bin\strip eax_0.exe && eax_0
@echo eax_0 = %ERRORLEVEL%
bin\nasm -f win32 eax_1.asm && bin\ld -e _start eax_1.obj -o eax_1.exe && bin\strip eax_1.exe && eax_1
@echo eax_1 = %ERRORLEVEL%
@echo ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
eax_0.exe
@echo eax_0 =  %ERRORLEVEL%
@rem echo Exit Code is $LastExitCode
eax_1.exe
@echo eax_1 =  %ERRORLEVEL%
@rem echo Exit Code is $LastExitCode

@rem bin\nasm -fwin32 eax_1.asm && bin\ld eax_1.obj -o eax_1.exe && bin\strip eax_1.exe && eax_1
