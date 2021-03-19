RWStructuredBuffer<uint64_t> H : register(u0);

[numthreads(1, 1, 1)]
void CSMain() {
  for (int i = 0; i < 100; ++i) {
    uint64_t val = H[i] ^ i;
    val = val << 4 | val >> 60;
    H[i] = val;
  }
}
