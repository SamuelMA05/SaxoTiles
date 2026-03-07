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
No es que nuestra zImage de por sí no sirva, lo que pasa es que tenemos que empaquetarla en un archivo boot.img que es el formato estándar del arranque de la imagen (Android), ya que que es la permite buildroot.

Como se ve en la imagen, note que la microsd tiene esa dirección. Entonces vamos a pasarle la imagen de la siguiente forma (estando en la misma dirección anterior):

```
cp boot.img /media/Su_Usuario/Nombre_de_la_Particion/
```
Todo esto 

En el minicom ejecutemos el comando 

```
fatls mmc 0:1 
```
Y nos aparecerá lo siguiente:


Ahí mismo el minicom vamos a ejecutar:
```
fatload mmc 0:1 45000000 zImage
```
Este comando carga la imagen (kernel de Linux) desde la SD a la RAM del dispositivo (AllWinner), se lo carga a la dirección de memoria 45000000, para que desde ahí U-Boot pueda arrancarlo.











