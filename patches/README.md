# Wine DComp counterpart and combined package

`wine-dcomp-explicit-binding-v1.patch` contains the Wine-side source changes:

| Wine source path | Purpose |
| --- | --- |
| `dlls/dcomp/device.c` | Target/device lifetime, scene synchronization and Commit integration |
| `dlls/dcomp/dcomp.spec` | Export composition binding protocol version 1 |
| `dlls/dcomp/wine_dxgi_composition.h` | New private COM binding ABI shared with DXVK |
| `dlls/dcomp/composition_binding.c` | New snapshot/diff and bind/unbind implementation |

The patch requires the existing AutoCAD ScaleWine baseline
`a23f56c9b0e5a8e4c4542fea5faeab1fadf70dc9`. It does not include that baseline's
earlier DComp interfaces and visual-tree implementation, and is not a standalone
implementation for stock Wine. `composition_binding.c` is included by `device.c`,
not compiled separately. In a clean checkout of the compatible Wine baseline:

```sh
git apply --check /absolute/path/to/wine-dcomp-explicit-binding-v1.patch
git apply /absolute/path/to/wine-dcomp-explicit-binding-v1.patch
```

Build DComp for each required architecture using that Wine tree's configuration.
The resulting DLL must export `wine_dcomp_get_composition_binding_version()`
returning 1. The original temporary Wine worktree is no longer available locally;
the patch is the retained source-change deliverable, not a complete Wine source
distribution.

## Combined binary package

```text
dxvk-3.1.1-for-scalewine-<build>/
  x32/{d3d8,d3d9,d3d10core,d3d11,dxgi}.dll
  x64/{d3d8,d3d9,d3d10core,d3d11,dxgi}.dll
  wine-dcomp/x32/dcomp.dll
  wine-dcomp/x64/dcomp.dll
  dxvk.conf
  dxvk-composition.conf
  SHA256SUMS
```

Keep the archive basename and its single root directory identical, apart from
`.tar.gz`, as expected by the Winetricks `helper_dxvk` layout. DComp is in the
same archive but in separate directories. The standard DXVK helper does not
install it; a custom verb must explicitly deploy compatible DComp or verify
that the selected Wine runtime already supplies protocol v1.

Build all DXVK APIs from the same revision rather than mixing upstream DLLs
with this fork: the pending-clear fix is in shared rendering code used by D3D9
as well as D3D11. The default `dxvk.conf` keeps the bridge disabled. Select the
optional `dxvk-composition.conf` via an absolute `DXVK_CONFIG_FILE` only with
compatible DComp. Back up DLLs, symlinks and overrides before installation, and
stop the application. See `../WINE_COMPOSITION.md` for validation limits.
