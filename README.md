# device/samsung/a02q — LineageOS 19.1 (SM-A025M, msm8953)

Skeleton inicial. Valores de board reais (arch, kernel, partições, super, AVB)
semeados a partir do device tree de TWRP. Ainda NÃO builda: falta a bring-up
(ver TODO em device.mk).

Próximos passos:
1. kernel/samsung/a02q  (reusar ~/a02q-lineage-backup/kernel-a02q — `make mrproper` antes)
2. vendor/samsung/a02q  (extrair blobs do stock: extract-files.sh / proprietary-files.txt)
3. rootdir (fstab.qcom, init.*.rc, ueventd, gpt) + manifests Treble
4. HALs + SELinux + overlays
5. `breakfast a02q` -> `export OUT_DIR=/mnt/android/a02s_twrp/out` -> `brunch a02q`
