# Instalación de Debian en Allwinner T113

## Requisitos previos

Tener instalado en la PC:
```bash
sudo apt install debootstrap fakeroot qemu-user-static rsync
```

---

## 1. Generar el rootfs (Primera etapa)

El profesor provee un script llamado `build_debian`. Ejecutarlo así:

```bash
chmod +x build_debian
./build_debian
```

Esto descarga e instala los paquetes base y genera la carpeta:
```
debian_fs/debian_bookworm/
```


---

## 2. Preparar la SD

Conectar la SD a la PC e identificar las particiones:

```bash
lsblk
```

La SD debe tener dos particiones:
- `BOOT` (~256MB) — partición FAT donde va el kernel y DTB
- `ROOTFS` (~10GB, ext4) — partición donde va el sistema operativo

### Desmontar y formatear el ROOTFS

>  Reemplazar `/dev/sddX` con la partición correcta según `lsblk`

```bash
sudo umount /media/nicolas/ROOTFS
sudo mkfs.ext4 -F -L ROOTFS /dev/sddX
sudo mount /dev/sddX /mnt
```

### Copiar el rootfs a la SD

```bash
sudo rsync -a ~/Descargas/debian_fs/debian_bookworm/. /mnt/
```

---

## 3. Segunda etapa del debootstrap (desde la PC)

La primera etapa del script usa `--foreign`, lo que significa que la segunda etapa debe completarse mediante `chroot` desde la PC.

### Montar sistemas necesarios

```bash
sudo mount -o exec,dev /dev/sddX /mnt
sudo mount --bind /proc /mnt/proc
sudo mount --bind /sys /mnt/sys
sudo mount --bind /dev /mnt/dev
sudo mount --bind /dev/pts /mnt/dev/pts
sudo cp /usr/bin/qemu-arm-static /mnt/usr/bin/
```

> El flag `exec,dev` es importante — sin él el debootstrap falla con error de permisos.

### Entrar al entorno ARM

```bash
sudo chroot /mnt /bin/bash
```

El prompt cambiará a algo como `I have no name!@PC:/#`

### Ejecutar la segunda etapa

```bash
/debootstrap/debootstrap --second-stage
```

Esto instala todos los paquetes del sistema. Al finalizar debe mostrar:
```
I: Base system installed successfully.
```

### Configurar el sistema

```bash
passwd root          # Establecer contraseña (ej: embebidos123)
echo "root" > /etc/hostname
```

### Salir del chroot

```bash
exit
```

---

## 4. Desmontar la SD

```bash
sudo umount /mnt/dev/pts
sudo umount /mnt/dev
sudo umount /mnt/sys
sudo umount /mnt/proc
sudo sync
sudo umount /mnt
```

---

## 5. Arrancar el T113

Insertar la SD en el Allwinner T113 y encenderlo. El sistema debe arrancar en Debian Bookworm.

Verificar la versión del sistema:
```bash
cat /etc/debian_version
```

Debe mostrar algo como `12.x`.

---

## Notas

- El script `build_debian` del profesor usa `debootstrap --foreign` para arquitectura `armhf`, por eso requiere la segunda etapa con `qemu-arm-static`.
- Si el sistema arranca en modo solo lectura, ejecutar: `mount -o remount,rw /`
- Para instalar paquetes adicionales se requiere conexión a internet en el T113.
