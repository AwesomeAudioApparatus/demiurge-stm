
#ifndef VCO_DEMI_ASSERTS_H
#define VCO_DEMI_ASSERTS_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define configASSERT(a) if (!(a)) {                                     \
        printf("%s:%d (%s)- assert failed!\n", __FILE__, __LINE__,  \
                   __FUNCTION__);                                       \
        };

#endif //VCO_DEMI_ASSERTS_H
