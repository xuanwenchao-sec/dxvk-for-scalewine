# ScaleWine explicit composition bridge (DXVK v3.1.1)

Base: upstream v3.1.1 `b1a1c99ab52b687cf950d62c88bc2fa316b41663`.

## Status: 2026-10-29

The AutoCAD 2024 Home page now displays in the isolated 64-bit test prefix, with
user confirmation. DXVK DXGI/D3D11 and Wine DComp compile for x64 and x86. New
explicit-binding probes pass 33 checks per architecture with no failures.
This does not qualify every AutoCAD feature or implement a complete compositor.

The superseded latest-target implementation failed application acceptance:
WebView2 recreated Home swapchains after hidden targets had been created on the
same GPU thread, causing presentation to the wrong HWND. This version no longer
uses `wine_dcomp_get_current_target_hwnd()`.

## Required Wine counterpart

Apply `patches/wine-dcomp-explicit-binding-v1.patch` to compatible ScaleWine
AutoCAD baseline `a23f56c9b0e5a8e4c4542fea5faeab1fadf70dc9`, or port the behavior.
The patch depends on that baseline's DComp interfaces/visual tree; it is not a
standalone patch for arbitrary stock Wine. Added implementation file
`composition_binding.c` is included by `device.c`, not separately compiled.

The loaded `dcomp.dll` must export:

```c
UINT WINAPI wine_dcomp_get_composition_binding_version(void); /* returns 1 */
```

DXVK holds a module reference during version lookup; it does not load DComp for
the application. Missing module/export or unsupported version fails creation
with `DXGI_ERROR_INVALID_CALL`, without guessing a window or dummy fallback.

Both sides share `wine_dxgi_composition.h`. Private COM interface
`IWineDXGICompositionBinding`, IID `f20b126a-b535-4a7e-9853-f897e7629145`, adds:

```c
HRESULT BindTarget(HWND hwnd, UINT64 binding_id);
HRESULT UnbindTarget(UINT64 binding_id);
```

Wine Commit snapshots each device's target/root/child/content tree, resolves
canonical COM identity outside its scene snapshot lock, rejects conflicting
owners, unbinds removed bindings before adding replacements and retains the
committed references. Scene revision checks reject stale snapshots with
`E_PENDING`; concurrent/reentrant Commit is also rejected with `E_PENDING`.
Callback-stage failure clears bindings rather than restoring an old frame.
Target destruction triggers reconciliation, and device destruction releases
retained binding interfaces. Extreme concurrency/reentrancy remains a review
area, not a proven general compositor contract.

## DXVK behavior

Composition creation allocates real D3D11 backbuffers without selecting an HWND
or allocating a dummy composition window. The private surface-factory extension
requests deferred creation. The private presenter extension releases surface/WSI
resources after context flush and CS synchronization while retaining backbuffers.

Wine Commit calls the swapchain dispatcher's private binding interface. The
wrapper preserves external canonical IUnknown identity. Bind validates the HWND
and owner, updates the surface factory and presents pending content. Unbind
checks the target cookie, so a stale owner cannot detach a newer binding.
Conflicting ownership requires an explicit prior unbind.

Before binding, non-test Present requests are coalesced into a pending flag and
return success. Bind replays the current content with a full-frame backend
Present. This does **not** preserve every pre-bind Present1 dirty/scroll region,
flag, sync interval or per-frame statistic. Pre-bind surface queries can return
`VK_NOT_READY` and currently log that state as a surface creation error even
though subsequent binding succeeds.

The independent pending-clear fix is retained:
`DxvkContext::beginExternalRendering` calls `endCurrentPass(false)` before
`endCurrentCommands()`, executing clears before the external presentation blit.

## Enable and deploy

Use matching `dxgi.dll` and `d3d11.dll` from this build together with the patched
Wine `dcomp.dll`, with architecture matching each process. In a standard WoW64
prefix, system32 contains x64 DLLs and syswow64 contains x86 DLLs.

```ini
dxgi.enableWineCompositionBridge = True
```

Pass the absolute config path via `DXVK_CONFIG_FILE` to the application and its
children. Configure the required native DLL overrides for the deployed files;
verify physical DLL mappings in both the application and WebView2 GPU processes.
The bridge defaults to False. Ordinary HWND and legacy dummy paths are not the
explicit-binding composition path.

Use an isolated prefix, stop its application before replacing DLLs and back up
DLLs, symlinks and overrides. The successful test uses `autocad2024-dxvk311-test`;
the production prefix is not changed. Source changes and this README are local
until explicitly committed and pushed.

## Validation scope

New x64/x86 probes cover interleaved targets, creation without a guessed HWND,
Present before Commit, preserved buffer and dispatcher COM identities, stale
cookies, owner conflicts, repeated Commit, duplicate-content rejection, hidden
targets followed by Home-like chain replacement, root detach, target migration
and destruction, and final independent window ownership.

The legacy immediate-binding probes require updates for deferred semantics;
the old 20-scenario and Wine 2042-test counts are not claims about this revision.
AutoCAD Home pixel evidence and application/GPU module maps are retained in the
local workspace's `analysis/dxvk-composition-20261028/autocad-explicit/` directory.
No full new drawing, long-running, repeated-navigation or cold-restart regression
is claimed by the current Home-page acceptance.

## Not implemented

This bridge is not full DirectComposition: arbitrary visual blending, alpha,
transforms, clipping, effects/animation, multiple distinct DXVK contents sharing
one HWND, cross-process composition, and full queued Present1 semantics are not
provided. Wine WaitForCommitCompletion has not become a GPU/compositor fence.
Clients must keep target windows alive while presenting. Additional synchronization
and failure recovery deserve review before general-purpose deployment.
