# SPI Driver Pack

This pack converts the SPI device list into PocketOS-compliant drivers with tiered gating, register maps (Tier 2), and identification hooks.

## Implemented Drivers and Tiers
- **mcp2515** — CAN controller — Tier follows `POCKETOS_DRIVER_TIER_MCP2515` (Tier 2 exposes full register map)
- **nrf24l01** — 2.4GHz RF transceiver — Tier follows `POCKETOS_DRIVER_TIER_NRF24L01`
- **w5500** — Ethernet controller — Tier follows `POCKETOS_DRIVER_TIER_W5500`
- **st7789** — TFT display controller — Tier follows `POCKETOS_DRIVER_TIER_ST7789`
- **ili9341** — TFT display controller — Tier follows `POCKETOS_DRIVER_TIER_ILI9341`
- **mcp3008** — SPI ADC (channels exposed as registers 0-7) — Tier follows `POCKETOS_DRIVER_TIER_MCP3008`
- Skeleton-only (return `ERR_UNSUPPORTED` until enabled): st7735, st7796, ili9486/ili9488, hx8357, gc9a01a, ra8875, spi SSD13xx family, W25Qxx/GD25Qxx/AT25DF/AT45/FRAM, ADS12xx/ADS125x/ADS1262, DAC855x/MCP492x, IC sensors in SPI mode (ICM20948, LIS3DH), epaper controllers (SSD1675/SSD1680/UC8151/IL0373), TLC5947.

## Endpoint Requirements / Examples
- CAN: `spi0:cs=5,irq=4`
- RF: `spi0:cs=5,ce=16,irq=4`
- Ethernet: `spi0:cs=5,rst=17,irq=4`
- Display (ILI9341/ST7789): `spi0:cs=5,dc=16,rst=17`
- ADC (MCP3008): `spi0:cs=5`

Required pin: **cs**. Optional pins: **dc/rst/irq/ce/busy** as per device.

## Identification Registers Used
- MCP2515: `CANSTAT`, `CANCTRL`
- NRF24L01+: `STATUS`, `CONFIG`
- W5500: `VERSIONR`
- ST7789 / ILI9341: display ID read commands (RDDID)
- MCP3008: channel conversions (no dedicated ID, bind manually)

## CLI Transcript (sample)
```
bus.list
bus.info spi0
ep.list
dev.bind mcp2515 spi0:cs=5,irq=4
schema.get 1
reg.list 1
reg.read 1 0x0E
reg.write 1 0x0F 0x80

dev.bind mcp3008 spi0:cs=5
schema.get 2
reg.read 2 0x00 2   # read CH0

persist.save
reboot
dev.list   # bindings auto-restored
```

All `reg.*` intents route through the IntentAPI/DeviceRegistry and respect tier gating (Tier 2 required for register maps).
