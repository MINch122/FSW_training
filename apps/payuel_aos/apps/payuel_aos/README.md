# PAYUEL_AOS

cFS application generated from local template `/home/eodnjs7783/dss/0406/cfs_appgen/sample_app`.

## Integration

1. Place this directory under `apps/` in your cFS mission tree.
2. Add `payuel_aos` to your `targets.cmake`.
3. Add to `cfe_es_startup.scr`:
   ```
   CFE_APP, payuel_aos, PAYUEL_AOS_Main, PAYUEL_AOS, 80, 16384, 0x0, 0;
   ```

## Customization

- **Message IDs**: Edit `fsw/platform_inc/payuel_aos_msgids.h`
  (or `config/` defaults for Draco+)
- **Performance ID**: Edit the perfids header
- **Table structure**: Edit the table header and `fsw/tables/payuel_aos_tbl.c`
- **Commands**: Add new command codes and handlers following the existing pattern
