## Carga del Kernel

### Construcción del Kernel

Ejecutar el siguiente comando en la terminal:
```bash
./build_kernel.sh
```

### Configuración del U-Boot

En el `defconfig` de U-Boot, agregar las siguientes líneas:
```
CONFIG_OF_LIBFDT=y
CONFIG_OF_CONTROL=y
CONFIG_DEFAULT_DEVICE_TREE="sun8i-t113s-saxo"
```

Una vez realizado este cambio, se debe reconstruir (rebuild) el U-Boot.

---

### Modificación de los Device Tree Source (.dts / .dtsi)

Dirigirse a la carpeta `linux-patch-6.16.9` y modificar los siguientes archivos:

#### `sun8i-t113s-saxo-gateway.dts`

Cambiar el nodo `chosen` para que apunte al puerto serial correcto (por defecto está en `serial3`):
```dts
chosen {
    stdout-path = "serial0:115200n8";
};
```

Además, agregar el siguiente bloque al final del archivo:
```dts
&uart0 {
    pinctrl-names = "default";
    pinctrl-0 = <&uart0_pe2_pins>;
    status = "okay";
};
```

#### `sunxi-d1s-t113s-saxo.dtsi`

Agregar el siguiente bloque:
```dts
&uart0 {
    pinctrl-names = "default";
    pinctrl-0 = <&uart0_pe2_pins>;
    status = "okay";
};
```

#### `sunxi-d1s-t113.dtsi`

Este archivo se encuentra en:
```
T113_SAXO_OS/linux/arch/riscv/boot/dts/allwinner/
```

> **Nota:** Hay que preguntarle al profe por esto, que es raro que por cambiar el riscv funcione, cuando estamos es usando arm
> 
Agregar el siguiente bloque para UART0:
```dts
/omit-if-no-ref/
uart0_pe2_pins: uart0-pe2-pins {
    pins = "PE2", "PE3";
    function = "uart0";
};
```

---

### Creación y transferencia de la imagen

El comando `bootz` de U-Boot permite arrancar una imagen comprimida de Linux desde memoria:
```
bootz [addr [initrd[:size]] [fdt]]
```

Este comando requiere tres argumentos: la dirección de la imagen del kernel, la dirección del `initrd` (opcional) y la dirección del Device Tree Blob (DTB).

#### Instalación de `mkbootimg`
```bash
sudo apt install -y mkbootimg
```

#### Generación de la imagen de arranque

Desde el directorio `T113_SAXO_OS/linux/arch/riscv/boot`, ejecutar:
```bash
mkbootimg --kernel zImage --output boot.img
```

> La imagen `zImage` es funcional por sí sola, pero debe empaquetarse en el formato `boot.img` (estándar de arranque Android), que es el que Buildroot espera.

#### Copia de archivos a la microSD
```bash
cp zImage /media/<usuario>/<particion>/
cp ./allwinner/sun8i-t113s-saxo-gateway.dtb /media/<usuario>/<particion>/
```

---

### Arranque desde Minicom

#### Verificar archivos en la microSD
```
fatls mmc 0:1
```

#### Cargar la imagen en memoria
```
fatload mmc 0:1 45000000 boot.img
```

Este comando carga el kernel desde la microSD a la dirección `0x45000000` de la RAM del dispositivo, desde donde U-Boot puede arrancarlo.

---

### Configuración de variables de entorno en U-Boot

Ejecutar los siguientes comandos en Minicom:
```
setenv kernel zImage
setenv dts sun8i-t113s-saxo-gateway.dtb
setenv scan_dev_for_boot
setenv scan_dev_for_boot_part
setenv scan_dev_for_efi
setenv scan_dev_for_extlinux
setenv boot_a_script
setenv boot_efi_binary
setenv boot_efi_bootmgr
setenv boot_extlinux
setenv mmc_dev 0
setenv mmc_boot_part 1
setenv bootargs1 'earlyprintk=sunxi-uart,0x02500000 clk_ignore_unused'
setenv bootargs2 'console=ttyS0,115200 loglevel=8 root=/dev/mmcblk0p5'
setenv bootargs3 'init=/sbin/init partitions=ext4 cma=8M gpt=1'
setenv bootargs ${bootargs1} ${bootargs2} ${bootargs3}
setenv boot_mmc 'fatload mmc ${mmc_dev}:${mmc_boot_part} 45000000 ${kernel}; fatload mmc ${mmc_dev}:${mmc_boot_part} 40000000 ${dts}; bootz 45000000 - 40000000'
saveenv
```

>  **Importante:** Usar el comando `print` con frecuencia para verificar que las variables de entorno sigan definidas, ya que en algunos casos se borran inesperadamente.

---

### Arranque del sistema

Una vez configuradas las variables de entorno, ejecutar:
```
run boot_mmc
```

Si todo está correctamente configurado, el sistema debería iniciar el kernel de Linux.
