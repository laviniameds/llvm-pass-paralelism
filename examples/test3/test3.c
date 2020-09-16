int f(int a, int b, int c){ 
  int x1,x2,x3,x4,x5,x6,x7,x8;;
  x1 = a-b;
  x2 = a+b;
  x3 = x2+c;
  x4 = x3+x2;
  x5 = x4+c;
  x6 = x5+x3;
  x7 = x6+x2;
  x8 = x1*x7;
  return x8; 
}