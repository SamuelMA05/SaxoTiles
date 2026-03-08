## Carga del Kernel
Ahora vamos a utilizar en la terminal el comando:

```
./build_kernel.sh
```
Ahora vamos crear una imagen y la vamos a pasar a nuestra memoria. Nos estamos basando en el comando de minicom 

=> help bootz
bootz - boot Linux zImage image from memory
```
Usage:
bootz [addr [initrd[:size]] [fdt]]
    - boot Linux zImage stored in memory
        The argument 'initrd' is optional and specifies the address
        of the initrd in memory. The optional argument ':size' allows
        specifying the size of RAW initrd.
        When booting a Linux kernel which requires a flat device-tree
        a third argument is required which is the address of the
        device-tree blob. To boot that kernel without an initrd image,
        use a '-' for the second argument. If you do not pass a third
        a bd_info struct will be passed instead

=> 
```
Según la documentación de ayuda este comando "boot Linux zImage image from memory". Note que este nos pide una imagen, una dirección y el dts. Vamos a hacer eso ahora


Para crear la imagen, necesitamos primero antes tener mkboot, entonces corremos en la terminal:

```
sudo apt install -y mkbootimg
```
De ahí escribimos en la dirección de /linux/arch/arm/boot:
```
mkbootimg --kernel zImage --output boot.img
```
No es que nuestra zImage que teníamos originalmente de por sí no sirva, lo que pasa es que tenemos que empaquetarla en un archivo boot.img que es el formato estándar del arranque de la imagen (Android), ya que que es la permite buildroot.

Como se ve en la imagen, note que la microsd tiene esa dirección. Entonces vamos a pasarle la imagen de la siguiente forma (estando en la misma dirección anterior):

```
cp zImage /media/Su_Usuario/Nombre_de_la_Particion/
```
Nos falta es entonces el dts, este corresponde con el que está nuevamente en esa carpeta de boot. La dirección es boot/dts El comando es:

```
cp ./allwinner/sun8i-t113s-saxo-gateway.dtb /media/Su_Usuario/Nombre_de_la_Particion/
```
Desmonte la SD y abramos minicom

Ahora, en el minicom ejecutemos el comando 

```
fatls mmc 0:1 
```
Y nos aparecerá lo siguiente:


Ahí mismo el minicom vamos a ejecutar:
```
fatload mmc 0:1 45000000 boot.img
```
Este comando carga la imagen (kernel de Linux) desde la SD a la RAM del dispositivo (AllWinner), se lo carga a la dirección de memoria 45000000, para que desde ahí U-Boot pueda arrancarlo.





Vamos a modidifcar nuestras variables de entorno de la siguiente forma. En el minicom escribimos:

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
setenv mmc dev 0
setenv mmc_boot_part 1
setenv bootargs1 'earlyprintk=sunxi-uart,0x02500000 clk_ignore_unused'
setenv bootargs2 'console=ttyS0,115200 loglevel=8 root=/dev/mmcblk0p5'
setenv bootargs3 'init=/sbin/init partitions=ext4 cma=8M gpt=1'
setenv bootargs ${bootargs1} ${bootargs2} ${bootargs3}
setenv boot_mmc 'fatload mmc ${mmc_dev}:${mmc_boot_part} 45000000 ${kernel}; fatload mmc ${mmc_dev}:${mmc_boot_part} 40000000 ${dts}; bootz 45000000 - 40000000'
saveenv
```
Ojo, esté usando constantemente el comando print, porque no sé por qué, pero a veces se borran unas variables de entorno, entonces toca estar pendiente.

Vamos al defconfig del U-Boot y agregamos :
```
CONFIG_OF_LIBFDT=y
CONFIG_OF_CONTROL=y
CONFIG_DEFAULT_DEVICE_TREE="sun8i-t113s-saxo"
```
Y ahora corremos:
```
run boot_mmc
```

Debemos ir a la carpeta linux-patch-6.16.9 y modificar los archivos sun8i-t113s-saxo-gateway.dts y sunxi-d1s-t113s-saxo.dtsi

En el primer archivo debemos de modificar:
```
chosen {
		stdout-path = "serial0:115200n8";
	};
```
Que por defecto está en 3.

Adicionalmente agregamos

```
&uart0 {
	pinctrl-names = "default";
	pinctrl-0 = <&uart0_pe2_pins>;
	status = "okay";
};
```

En el segundo archivo agregamos

```
&uart0 {
	pinctrl-names = "default";
	pinctrl-0 = <&uart0_pe2_pins>;
	status = "okay";
};
```

Adicionalmente debemos de modificar el archivo sunxi-d1s-t113.dtsi que se encuentra en la dirección T113_SAXO_OS/linux/arch/riscv/boot/dts/allwinner(esto preguntarle al profe, porque se me hace muy raro que cambiando el riscv funcione)

Agregamos 
```
			/omit-if-no-ref/
			uart0_pe2_pins: uart0-pe2-pins {
    			pins = "PE2", "PE3";
   			 function = "uart0";
			};
```

buildeamos el kernel, y copiamos los archivos y vamos al minicom y escribimos run boot_mmc y deberíamos de obtener algo así:

