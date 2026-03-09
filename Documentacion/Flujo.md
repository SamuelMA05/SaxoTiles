1) Se activa la Boot ROM al encender el chip. Esta tiene como función encontrar el SPL en la SD/EMMC.
2) La SPL inicializa la RAM. Una vez lista la RAM, se carga el U-Boot. Cuando nos sale en el minicom algo como:
```
U-Boot SPL 2025.07 ...
DRAM: 128 MiB
Trying to boot from MMC1
```
Eso es que el SPL iniciliazó la DRAM de 128Mib y luego cargó el U-Boot completo.

Eso fue gracias al comando
```
sudo dd if=u-boot-sunxi-with-spl.bin of=/dev/sdc bs=1k seek=16400
```
Que tiene SPL y U-Boot empaquetados.
