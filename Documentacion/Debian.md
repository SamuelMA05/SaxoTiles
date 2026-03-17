## Instalación de Debian en el Procesador
Lo primero que se debe de realizar es la instalación de esta herramienta:

```bash
sudo apt install debootstrap qemu-user-static
```

Prosiguiendo, se crea el sistema de archivos con el siguiento comando:

```bash
sudo debootstrap --arch=armhf --foreign bullseye ~/rootfs
```

Instalamos:

```bash
sudo cp /usr/bin/qemu-arm-static ~/rootfs/usr/bin/
sudo chroot ~/rootfs /debootstrap/debootstrap --second-stage
```

Después se crea una partición EXT4 y se formatea (desde aquí documenten ustedes, es que sería más ilustrativo con la SD en físico), y se copia el sistema de archivos
a la partición:

```bash
sudo mkdir -p /mnt/rootfs
sudo mount /dev/sdc2 /mnt/rootfs
sudo cp -a ~/rootfs/* /mnt/rootfs/
sudo umount /mnt/rootfs
```
Se configura para que el usuario root tenga una contraseña: 

```bash
sudo mount /dev/sdc2 /mnt/rootfs
sudo chroot /mnt/rootfs passwd root
sudo umount /mnt/rootfs
```

De ahí, se ejecuta en el minicom

```bash
run boot_mmc
```
Se ingresa el nombre y contraseña del usuario root y ya se estaría trabajando con el GNU/Linux de la placa.
