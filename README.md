NV-PSO-OOM-DX12
===============

## Description

In one sentence, if one writes circular shift like `(a << b | a >> (64 - b))` where a is an 64-bit
integer in compute shader, then they will get `E_OUTOFMEMORY` when creating compute pipeline state.

This repository contains a minimal example illustrating the problem. (the term “minimal” does not
apply to DX12 boilerplates)

## How to Reproduce
1. Open MSVC x64 Native Tools Command Prompt
2. Clone the repository and `cd` into it
3. Run `make.cmd`

   (optionally with a number 1/0 indicating if you want to create an NVIDIA device or not)
4. Then run `main.exe`, and an OOM error should show up for NVIDIA GPU.

## Notes

This example works fine with the integrated GPU on my laptop, while OOM was returned when using my
discrete GeForce GTX 1050Ti. And I confirmed with the stack trace that the error was generated in
`nvwgf2umx.dll`, an NVIDIA driver module.

I guess there is something wrong in the driver who compiles a shader from DXBC to some GPU native
format, especially for recognizing and optimizing the circular shift pattern.

Hope NVIDIA can get circular shift for `uint64_t` work.

I also posted this to [NVIDIA Forum](https://forums.developer.nvidia.com/t/out-of-memory-when-creating-a-simple-compute-pipeline-state-object-dx12/171178).
