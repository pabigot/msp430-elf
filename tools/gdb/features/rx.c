/* THIS FILE IS GENERATED.  Original: rx.xml */

#include "defs.h"
#include "osabi.h"
#include "target-descriptions.h"

struct target_desc *tdesc_rx;
static void
initialize_tdesc_rx (void)
{
  struct target_desc *result = allocate_target_description ();
  struct tdesc_feature *feature;
  struct tdesc_type *field_type, *type;

  set_tdesc_architecture (result, bfd_scan_arch ("rx"));

  feature = tdesc_create_feature (result, "org.gnu.gdb.rx.core");
  field_type = tdesc_create_flags (feature, "rx_psw_flags", 4);
  tdesc_add_flag (field_type, 0, "C");
  tdesc_add_flag (field_type, 1, "Z");
  tdesc_add_flag (field_type, 2, "S");
  tdesc_add_flag (field_type, 3, "O");
  tdesc_add_flag (field_type, 4, "");
  tdesc_add_flag (field_type, 16, "I");
  tdesc_add_flag (field_type, 17, "U");
  tdesc_add_flag (field_type, 18, "");
  tdesc_add_flag (field_type, 20, "PM");
  tdesc_add_flag (field_type, 21, "");
  tdesc_add_flag (field_type, 24, "IPL");
  tdesc_add_flag (field_type, 27, "");

  field_type = tdesc_create_flags (feature, "rx_fpsw_flags", 4);
  tdesc_add_flag (field_type, 0, "RM");
  tdesc_add_flag (field_type, 2, "CV");
  tdesc_add_flag (field_type, 3, "CO");
  tdesc_add_flag (field_type, 4, "CZ");
  tdesc_add_flag (field_type, 5, "CU");
  tdesc_add_flag (field_type, 6, "CX");
  tdesc_add_flag (field_type, 7, "CE");
  tdesc_add_flag (field_type, 8, "DN");
  tdesc_add_flag (field_type, 9, "");
  tdesc_add_flag (field_type, 10, "EV");
  tdesc_add_flag (field_type, 11, "EO");
  tdesc_add_flag (field_type, 12, "EZ");
  tdesc_add_flag (field_type, 13, "EU");
  tdesc_add_flag (field_type, 14, "EX");
  tdesc_add_flag (field_type, 15, "");
  tdesc_add_flag (field_type, 26, "FV");
  tdesc_add_flag (field_type, 27, "FO");
  tdesc_add_flag (field_type, 28, "FZ");
  tdesc_add_flag (field_type, 29, "FU");
  tdesc_add_flag (field_type, 30, "FX");
  tdesc_add_flag (field_type, 31, "FS");

  tdesc_create_reg (feature, "r0", 0, 1, NULL, 32, "uint32");
  tdesc_create_reg (feature, "r1", 1, 1, NULL, 32, "uint32");
  tdesc_create_reg (feature, "r2", 2, 1, NULL, 32, "uint32");
  tdesc_create_reg (feature, "r3", 3, 1, NULL, 32, "uint32");
  tdesc_create_reg (feature, "r4", 4, 1, NULL, 32, "uint32");
  tdesc_create_reg (feature, "r5", 5, 1, NULL, 32, "uint32");
  tdesc_create_reg (feature, "r6", 6, 1, NULL, 32, "uint32");
  tdesc_create_reg (feature, "r7", 7, 1, NULL, 32, "uint32");
  tdesc_create_reg (feature, "r8", 8, 1, NULL, 32, "uint32");
  tdesc_create_reg (feature, "r9", 9, 1, NULL, 32, "uint32");
  tdesc_create_reg (feature, "r10", 10, 1, NULL, 32, "uint32");
  tdesc_create_reg (feature, "r11", 11, 1, NULL, 32, "uint32");
  tdesc_create_reg (feature, "r12", 12, 1, NULL, 32, "uint32");
  tdesc_create_reg (feature, "r13", 13, 1, NULL, 32, "uint32");
  tdesc_create_reg (feature, "r14", 14, 1, NULL, 32, "uint32");
  tdesc_create_reg (feature, "r15", 15, 1, NULL, 32, "uint32");
  tdesc_create_reg (feature, "usp", 16, 1, NULL, 32, "uint32");
  tdesc_create_reg (feature, "isp", 17, 1, NULL, 32, "uint32");
  tdesc_create_reg (feature, "psw", 18, 1, NULL, 32, "rx_psw_flags");
  tdesc_create_reg (feature, "pc", 19, 1, NULL, 32, "code_ptr");
  tdesc_create_reg (feature, "intb", 20, 1, NULL, 32, "uint32");
  tdesc_create_reg (feature, "bpsw", 21, 1, NULL, 32, "rx_psw_flags");
  tdesc_create_reg (feature, "bpc", 22, 1, NULL, 32, "code_ptr");
  tdesc_create_reg (feature, "fintv", 23, 1, NULL, 32, "uint32");
  tdesc_create_reg (feature, "fpsw", 24, 1, NULL, 32, "rx_fpsw_flags");
  tdesc_create_reg (feature, "acc", 25, 1, NULL, 64, "uint64");

  tdesc_rx = result;
}
