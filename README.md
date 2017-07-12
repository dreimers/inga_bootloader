inga_bootloader
===============

A Bootloader for INGA

#Usage

Get size of elf file

    make size

## with JTAGIce Mk2
Only program bootloader

    make program

Only set fuses

    make fuses

## with bitbanging via ftdi

Only program bootloader with bitbang

    make program.bang

Only set fuses with bitbang

    make fuses.bang

Flash fuses and program bootloader

    make bang

Use serial of the ftdi to select device to program
SERIAL is the variable which is piped to avrdude as extended paramenter

    SERIAL="A502C3YI" make bang
