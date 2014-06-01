/* This code used to provoke a seg fault in cc1 due to an infinite
   recursion between emit_move_insn, emit_move_inen1 and store_bit_field.  */
#pragma pack(1) 

void func (void * ptr) { (*(unsigned short *) ptr) = 0; }
