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
