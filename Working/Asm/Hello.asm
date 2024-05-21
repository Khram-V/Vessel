
	global  _main
	extern  _printf

	section .text
_main:
	push	message
	call	_printf
	add	esp, 4
	ret
message:
	db  'Hello, World', 10, 0


; bin\nasm -f win32 Hello.asm && bin\gcc -m32 hello.obj -o hello.exe && bin\strip hello.exe && hello
; echo hello =  %ERRORLEVEL%
