# device/samsung/a02q — LineageOS 19.1 (SM-A025M, msm8953)

Galaxy A02s (SM-A025M), base de firmware A025MUBS9CXI1.

- Kernel: `kernel/samsung/a02q` (4.9.227-26302074), áudio e pronto wlan
  compilados do fonte.
- Blobs: `vendor/samsung/a02q`, gerados por `setup-makefiles.sh` a partir de
  `proprietary-files.txt`. Onde existe módulo AOSP/Lineage/CAF equivalente,
  o open-source tem prioridade e o blob sai da lista.
- Userspace 32-bit (kernel arm64): o vendor stock não tem `lib64`.
- SELinux permissive durante a bring-up.

Estado: `m nothing`, kernel, dtbo e `selinux_policy` compilam. ROM completa
(`brunch a02q`) e boot no aparelho ainda não testados.

```bash
source build/envsetup.sh
lunch lineage_a02q-userdebug
mka bacon
```
