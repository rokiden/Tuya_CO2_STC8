#ifndef FIFO_H
#define FIFO_H
/* Fifo buffer
 *
 * USAGE:
 *  #define <name>_size
 *  Fifo_init(<name>)
 */

#define Fifo_init(name)                                                                            \
  static volatile uint8_t __XDATA name[name##_size];                                               \
  static volatile uint8_t __DATA name##_Iin = 0, name##_Iout = 0;                                  \
  static volatile __BIT name##_full = 0

#define Fifo_full(name) name##_full
#define Fifo_has_data(name) (Fifo_full(name) || name##_Iin != name##_Iout)

#define Fifo_push(name, byte)                                                                      \
  if (!Fifo_full(name)) {                                                                          \
    name##_full = (((name##_Iout - 1) & (name##_size - 1)) == name##_Iin);                         \
    name[name##_Iin++] = byte;                                                                     \
    name##_Iin &= (name##_size - 1);                                                               \
  }

#define Fifo_pop(name, var)                                                                        \
  var = name[name##_Iout++];                                                                       \
  name##_full = 0;                                                                                 \
  name##_Iout &= (name##_size - 1)

#endif