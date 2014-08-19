if [istarget "epiphany-*-*"] {
    # This test assumes the absence of struct padding.
    # to make this true for test4 struct A on epiphany would require
    # __attribute__((packed)) .
    return 1
}

if [istarget "msp430-*-*"] {
    # Linker relaxation means that the size of the
    # memmove function changes when using LTO. Other
    # optimizations makes the test too big.
    return 1
}

return 0
