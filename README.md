# C Crispy
Work in Progress!
The official C implementation of the Crispy Programming Language.

Crispy is a dynamically typed minimalistic scripting language.
This implementation includes a bytecode compiler and a simple stack based virtual machine.

A more advanced (but slower) version of crispy can be found here:
https://github.com/crispyteam/crispy-lang

## Installation  

Just clone the repository and use CMake to generate the make files.
  
	git clone https://github.com/funkschy/crispy
	cd crispy
	cmake .
	make
	./crispy
  
This will open an interactive shell where you can test the language very easily.
Alternatively you can just save your code into a file and run:
  
	./crispy [your filename]
  
If you decide to use the shell I would recommend wrapping the session with rlwrap.
  
If you don't have cmake installed you can also compile crispy like this:  
  
	gcc src/*/*.c -o crispy
  
Crispy is tested with gcc/mingw and clang.  
