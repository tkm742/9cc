#!/bin/bash

try(){
	expected="$1"
	input="$2"

	./9cc "$input" > tmp.s
	gcc -o tmp tmp.s
	./tmp
	actual="$?"

	if [ "$actual" = "$expected" ]; then
		echo "$input => $actual"
	else
		echo "$input => $expected expected, but got $actual"
		exit 1
	fi
}

try 0 "0;"
try 42 "42;"
try 21 "5+20-4;"
try 41 " 12 + 34 - 5;"
try 47 "5+6*7;"
try 15 "5*(9-6);"
try 4 "(3+5)/2;"
try 10 "-10+20;"
try 15 "(-3)*(-5);"
try 1 "1 == 1;"
try 0 "0 == 1;"
try 1 "1 != 2;"
try 0 "1 != 1;"
try 1 "1 < 2;"
try 0 "2 < 1;"
try 1 "1 <= 1;"
try 0 "2 <= 1;"
try 1 "2 > 1;"
try 0 "1 > 2;"
try 1 "1 >= 1;"
try 0 "1 >= 2;"
try 2 "LVar_1 = 2; LVar_1;"
try 3 "a = 3; return a;"
try 1 "if(1 == 1) return 1; else return 0;"
try 0 "if(1 == 0) return 1; else return 0;"
try 2 "if(1) if(0) 1; else 2; else 3;"
try 2 "if(0) 1; else if(1) 2; else 3;"
try 1 "while(1 == 0) 2; 1;"
try 1 "for(; 1 == 2; ) 2; 1;"

echo OK
