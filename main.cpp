#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <comdef.h>
#include <d3d12.h>
#include <dxgi1_6.h>

#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>

#include "d3dx12.h"

using Microsoft::WRL::ComPtr;
using namespace std;

#define HrEnsure(e_)                                                                            \
  ([](HRESULT Value) {                                                                          \
    if (SUCCEEDED(Value))                                                                       \
      return;                                                                                   \
    fprintf(stderr, TEXT("line=%d Value=0x%.8x: %s\n"), __LINE__, static_cast<unsigned>(Value), \
        _com_error(Value).ErrorMessage());                                                      \
    abort();                                                                                    \
  }(e_))

#define Ensure(e_)                                                     \
  ([](bool Pred) {                                                     \
    if (Pred)                                                          \
      return;                                                          \
    fprintf(stderr, TEXT("line=%d false: %s\n"), __LINE__, TEXT(#e_)); \
    abort();                                                           \
  }(static_cast<bool>(e_)))

enum ComputeRootParameters : UINT {
  RP_H = 0,
  RP_Num,
};

string LoadAsset(const char* Path) {
  ifstream FileStream(Path, ios_base::binary);
  ostringstream StringStream;
  StringStream << FileStream.rdbuf();
  return StringStream.str();
}

ComPtr<ID3D12Device> GDevice;
ComPtr<ID3D12RootSignature> GComputeRootSignature;
ComPtr<ID3D12PipelineState> GComputeState;

#ifndef USE_NVIDIA
#define USE_NVIDIA 1
#endif

void InitPipeline(const char* CsPath) {
  UINT DxgiFactoryFlags = 0;

#ifndef NDEBUG
  // Enable Debug layer
  {
    ComPtr<ID3D12Debug> Debug;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&Debug)))) {
      Debug->EnableDebugLayer();
      DxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
    }
  }
#endif

  // Create Factory
  ComPtr<IDXGIFactory6> Factory;
  HrEnsure(CreateDXGIFactory2(DxgiFactoryFlags, IID_PPV_ARGS(&Factory)));

  // Create device
  {
    GDevice.Reset();

    ComPtr<IDXGIAdapter1> Adapter;
    for (UINT AdapterIdx = 0;; ++AdapterIdx) {
      HRESULT HRes = Factory->EnumAdapterByGpuPreference(
          AdapterIdx, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&Adapter));
      if (HRes == DXGI_ERROR_NOT_FOUND) {
        break;
      }
      HrEnsure(HRes);

      DXGI_ADAPTER_DESC1 Desc;
      HrEnsure(Adapter->GetDesc1(&Desc));

#if USE_NVIDIA
      if (Desc.VendorId != 0x10de)
        continue;
#else
      if (Desc.VendorId == 0x10de)
        continue;
#endif

      if (Desc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)
        continue;

      if (SUCCEEDED(
              D3D12CreateDevice(Adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&GDevice)))) {
        printf("Adapter: %ls\n", Desc.Description);
        break;
      }
    }
    Ensure(Adapter.Get());
    Ensure(GDevice.Get());
  }

  // Create root RootSignature
  {
    CD3DX12_ROOT_PARAMETER1 Parameters[RP_Num];
    Parameters[RP_H].InitAsUnorderedAccessView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE);

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC Desc;
    Desc.Init_1_1(RP_Num, Parameters);

    ComPtr<ID3DBlob> RootSignature;
    ComPtr<ID3DBlob> Error;
    HrEnsure(D3DX12SerializeVersionedRootSignature(
        &Desc, D3D_ROOT_SIGNATURE_VERSION_1_1, &RootSignature, &Error));
    HrEnsure(GDevice->CreateRootSignature(0, RootSignature->GetBufferPointer(),
        RootSignature->GetBufferSize(), IID_PPV_ARGS(&GComputeRootSignature)));
  }

  // Create the pipeline state
  {
    string ComputeShader = LoadAsset(CsPath);

    D3D12_COMPUTE_PIPELINE_STATE_DESC Desc{};
    Desc.pRootSignature = GComputeRootSignature.Get();
    Desc.CS.pShaderBytecode = ComputeShader.data();
    Desc.CS.BytecodeLength = ComputeShader.size();

    // vvvvvvvv E_OUTOFMEMORY vvvvvvvv
    HrEnsure(GDevice->CreateComputePipelineState(&Desc, IID_PPV_ARGS(&GComputeState)));
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  }
}

int main() {
  printf(TEXT("Initializing pipeline...\n"));

  InitPipeline("CS.cso");

#if USE_NVIDIA
  printf(TEXT("If you reached here then you are too lucky to reproduce this bug :(\n"));
#else
  printf(TEXT("Cool :)\n"));
#endif

  return EXIT_SUCCESS;
}
