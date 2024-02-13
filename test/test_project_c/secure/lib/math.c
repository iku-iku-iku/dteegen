static int MOD = 10;

int mul(int x, int y) { return x * y % MOD; }

int qpow(int x, int y) {
  int res = 1;
  while (y) {
    if (y & 1)
      res = mul(res, x);
    x = mul(x, x);
    y >>= 1;
  }
  return res;
}

float pi() {
  float res = 0;
  int a = 1;
  for (int i = 0; i < 100000000; i++) {
    res += 1.0 / a;
    a += 2;
    res -= 1.0 / a;
    a += 2;
  }
  return res * 4;
}
