# Configuración de TWI3 (I2C) en Device Tree

cd linux-5.4-1.0.0/arch/arm/boot/dts

# El archivo que necesito tocar es:
# sun8iw20p1-linux.dtsi

# Dentro del archivo busco:
# pio: pinctrl@2000000
# y después de test_pins_b agrego:

twi3_pins_a: twi3@0 {
    allwinner,pins = "PB6", "PB7";
    allwinner,function = "twi3";
    allwinner,muxsel = <4>;
    allwinner,drive = <1>;
    allwinner,pull = <1>;
};

twi3_pins_b: twi3@1 {
    allwinner,pins = "PB6", "PB7";
    allwinner,function = "io_disabled";
    allwinner,muxsel = <0xF>;
    allwinner,drive = <1>;
    allwinner,pull = <0>;
};

# pins_a → activo
# pins_b → deshabilitado

# Luego busco estos bloques:
# rgb24_pins_a
# rgb24_pins_b
# y elimino "PB6" y "PB7" para evitar conflicto

# Después busco el nodo twi3 y lo dejo así:

&twi3 {
    pinctrl-names = "default", "sleep";
    pinctrl-0 = <&twi3_pins_a>;
    pinctrl-1 = <&twi3_pins_b>;
    status = "okay";
};

# Compilo el dtb

make ARCH=arm sun8i-mangopi-mq-dual-linux.dtb

# Copio a la SD

cp arch/arm/boot/dts/sun8i-mangopi-mq-dual-linux.dtb /media/nicolas/BOOT/sun8i-t113s-saxo-gateway.dtb
sync

# En el embebido verifico el bus I2C

i2cdetect -y 3

# Si aparecen direcciones, pruebo lectura continua

while true; do i2cget -y 3 0x4d 0x00 w; sleep 0.5; done
