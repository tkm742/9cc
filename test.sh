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

try 0 "main(){return 0;}"
try 42 "main(){return 42;}"
try 21 "main(){return 5+20-4;}"
try 41 "main(){return 12 + 34 - 5;}"
try 47 "main(){return 5+6*7;}"
try 15 "main(){return 5*(9-6);}"
try 4 "main(){return (3+5)/2;}"
try 10 "main(){-10+20;}"
try 15 "main(){(-3)*(-5);}"
try 1 "main(){return 1 == 1;}"
try 0 "main(){return 0 == 1;}"
try 1 "main(){return 1 != 2;}"
try 0 "main(){return 1 != 1;}"
try 1 "main(){return 1 < 2;}"
try 0 "main(){return 2 < 1;}"
try 1 "main(){return 1 <= 1;}"
try 0 "main(){return 2 <= 1;}"
try 1 "main(){return 2 > 1;}"
try 0 "main(){return 1 > 2;}"
try 1 "main(){return 1 >= 1;}"
try 0 "main(){return 1 >= 2;}"
try 2 "main(){LVar_1 = 2; return LVar_1;}"
try 3 "main(){a = 3; return a;}"
try 1 "main(){if(1 == 1) return 1; else return 0;}"
try 0 "main(){if(1 == 0) return 1; else return 0;}"
try 2 "main(){if(1) if(0) return 1; else return 2; else return 3;}"
try 2 "main(){if(0) return 1; else if(1) return 2; else return 3;}"
try 1 "main(){while(1 == 0) 2; return 1;}"
try 1 "main(){for(; 1 == 2; ) 2; return 1;}"
try 3 "main(){a = 2; b = a + 1; return b;}"
try 4 "test(x, y){return x + y;} main(){return test(1, 3);}"
try 8 "fibo(n){if(n == 0){return 0;} else if(n == 1){return 1;} else{return fibo(n-1) + fibo(n-2);}} main(){return fibo(6);}"

echo OK
