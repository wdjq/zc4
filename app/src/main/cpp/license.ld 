SECTIONS
{
    . = 0x0;
    .text : { *(.text*) *(.rodata*) }
    . = 0x2000;
    .config : { KEEP(*(.config)) }
    .data : { *(.data*) }
    .bss : { *(.bss*) }
}
