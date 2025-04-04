# theExpressionEngine

A expression engine prototype leveraging LLVM as an Just-In-Time-Compiler.
Pass an expression on the commandline. Understood are
- addition
- subtraction
- real numbers
- sqrt
- exp
- sin
- cos
- tan
- asin
- acos
- atan
- sinh
- cosh
- tanh
- asinh
- acosh
- atanh
- abs
- log
- log10
- erf
- erfc
- tgamma
- lgamma
- cbrt
- x
- pow

It reads lines from stdin and interprets them as x values.
It reads one commandline argument,  which is enterpreted as a function and should use x.
You might want to use double quotes around this commandline argument.
e.g.
	`echo -e "0\n1"|./theExpressionEngine.exe "sin(x)"`
which prints the values of sin(0) and sin(1).

## News
All the LLVM code is now hidden behind the expression engine and multiple instances might exist and executed in parallel.
The parser/lexer isn't anymore implemented by hand but using flex&bison.
The versionos being used are:
- flex 2.6.4
- bison (GNU Bison) 3.8.2
## ToBeDone
Avoid recalculation of the same object!
Implement missing binary functions.
Allow for multiple parameters but not only x.
