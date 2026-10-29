#ifndef WINE_DXGI_COMPOSITION_H
#define WINE_DXGI_COMPOSITION_H

#include <unknwn.h>

static const GUID IID_IWineDXGICompositionBinding = {0xf20b126a, 0xb535, 0x4a7e,
    {0x98, 0x53, 0xf8, 0x97, 0xe7, 0x62, 0x91, 0x45}};

#undef INTERFACE
#define INTERFACE IWineDXGICompositionBinding
DECLARE_INTERFACE_(IWineDXGICompositionBinding, IUnknown)
{
    STDMETHOD(QueryInterface)(THIS_ REFIID iid, void **out) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;
    STDMETHOD(BindTarget)(THIS_ HWND hwnd, UINT64 binding_id) PURE;
    STDMETHOD(UnbindTarget)(THIS_ UINT64 binding_id) PURE;
};

#undef INTERFACE

#endif
