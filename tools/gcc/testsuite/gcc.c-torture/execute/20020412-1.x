if { [istarget "xstormy16-*-*"] } {
    # This test will fail for any target which uses an upwards growing
    # stack with downward growing args.  ie STACK_GROWS_DOWNWARD is
    # not defined but ARGS_GROW_DOWNWARD is defined.  There is only
    # one such target - the xstormy16.  The bug is in the generic code
    # somewhere, probably calls.c.  What happens is that the two
    # variable sized arrays - x and y - are assembled onto the stack
    # in the wrong order.  Since this bug is for a GCC extension to C
    # and since it only affects varargs handling and since it only
    # affects the xstormy16, the test is XFAILed for now.
    
    set torture_execute_xfail [istarget]
}

return 0
