# Slanex
The Slanex programming language

Slanex is a lisp. That means alot of the parenthesis.
Unlike the last lisp i had created, it runs on it's own Virtual Machiene.
The reasoning for a custom VM instead of luajit or guile was to make it unique.
It's approximatly 30 times slower than CPython. 
It is not ready for anything but testing.

there are two builtin libraries:
time and version
more to come

goals met:
complete lexer
complete most of the ast
build a working vm
add table datatype
use bignums and rationals
...

goals right now:
get withing 5x of CPython speed
add alot of functions

things to do later:
allow for functions to be table keys
networking and graphics libraries

to build you must have boost installed, a C++17 compiler, and -lgmp for linking