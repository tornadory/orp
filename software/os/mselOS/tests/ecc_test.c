/** @file main.c

    The reset vector for all the Libero-generated code
*/

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <msel.h>
#include <msel/syscalls.h>
#include <msel/tasks.h>
#include <msel/malloc.h>
#include <msel/uuid.h>
#include <msel/stdc.h>
#include <msel/debug.h>

#include <crypto/ecc.h>

void get_task(const uint8_t **endpoint, void (**task_fn)(void *arg, const size_t arg_sz),
        uint16_t *port, const uint8_t* data)
{
    *endpoint = NULL;
    *task_fn = NULL;
}

static void byte_to_string(uint8_t byte, uint8_t* str)
{
    str[0] = (byte >> 4);
    if (str[0] <= 9) str[0] += 0x30;
    else str[0] += 0x57;
    str[1] = byte & 0x0f;
    if (str[1] <= 9) str[1] += 0x30;
    else str[1] += 0x57;
}

static uint8_t base_point[128] = { 0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0, 0x75,0x2c,0xb4,0x5c,0x48,0x64,0x8b,0x18,0x9d,0xf9,0x0c,0xb2,0x29,0x6b,0x28,0x78, 0xa3,0xbf,0xd9,0xf4,0x2f,0xc6,0xc8,0x18,0xec,0x8b,0xf3,0xc9,0xc0,0xc6,0x20,0x39, 0x13,0xf6,0xec,0xc5,0xcc,0xc7,0x24,0x34,0xb1,0xae,0x94,0x9d,0x56,0x8f,0xc9,0x9c, 0x60,0x59,0xd0,0xfb,0x13,0x36,0x48,0x38,0xaa,0x30,0x2a,0x94,0x0a,0x2f,0x19,0xba,0x6c };
static uint8_t group_order[128] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfd, 0x15, 0xb6, 0xc6, 0x47, 0x46, 0xfc, 0x85, 0xf7, 0x36, 0xb8, 0xaf, 0x5e, 0x7e, 0xc5, 0x3f, 0x04, 0xfb, 0xd8, 0xc4, 0x56, 0x9a, 0x8f, 0x1f, 0x45, 0x40, 0xea, 0x24, 0x35, 0xf5, 0x18, 0x0d, 0x6b };

void ecc_task(void *arg, const size_t arg_sz) {

    uint8_t str[2];
    uint8_t scalar1[128];
    uint8_t scalar2[128];
    uint8_t point1[128];
    uint8_t point2[128];

    ecc_ctx_t ecc_ctx;
    ecc_ctx.scalar = scalar1;
    ecc_ctx.point = point1;

    msel_memcpy(point1, base_point, 128);
    msel_memcpy(scalar1, group_order, 128);
    msel_svc(MSEL_SVC_ECC, &ecc_ctx);

    unsigned i;
    for (i = 0; i < 128; ++i)
    {
        byte_to_string(point1[i], str);
        uart_write(str, 2);
    }
    uart_print("\n");
    
    unsigned j = 0;
    while (j++ < 5)
    {
        msel_memset(scalar1, 0, 128);
        msel_memset(scalar2, 0, 128);

        // Make up two random numbers and do some multiplies;
        for (i = 63; i < 128; ++i)
        {
            msel_svc(MSEL_SVC_TRNG, scalar1 + i);
            msel_svc(MSEL_SVC_TRNG, scalar2 + i);
        }

        // Print out the scalars in case something breaks
        uart_print("n1 = ");
        for (i = 63; i < 128; ++i)
            { byte_to_string(scalar1[i], str); uart_write(str, 2); }
        uart_print("\nn2 = ");
        for (i = 63; i < 128; ++i)
            { byte_to_string(scalar2[i], str); uart_write(str, 2); }
        uart_print("\n");

        // Compute point2 * (point1 * base)
        msel_memcpy(point1, base_point, 128);
        ecc_ctx.point = point1;
        msel_svc(MSEL_SVC_ECC, &ecc_ctx);
        ecc_ctx.scalar = scalar2;
        msel_svc(MSEL_SVC_ECC, &ecc_ctx);

        // Compute point1 * (point2 * base)
        msel_memcpy(point2, base_point, 128);
        ecc_ctx.point = point2;
        msel_svc(MSEL_SVC_ECC, &ecc_ctx);
        ecc_ctx.scalar = scalar1;
        msel_svc(MSEL_SVC_ECC, &ecc_ctx);

        // Make sure the two numbers are equivalent
        int good = 1;
        uart_print("out = ");
        for (i = 0; i < 128; ++i)
        {
            byte_to_string(point1[i], str);
            uart_write(str, 2);
            if (point1[i] != point2[i])
                good = 0;
        }
        uart_print("\n");
        if (!good)
            uart_print("ERROR\n");
        uart_print("\n");
    }
    uart_print("Done!\n");
}

/** @brief Runs immediately after reset and gcc init. initializes system and never returns

    @return Never returns
*/
int main() {
    msel_status ret;

    /* let msel initialize itself */
    msel_init();

    if((ret = msel_task_create(ecc_task,NULL,0,NULL)) != MSEL_OK)
        goto err;

    /* give control over to msel */
    msel_start();
    
err:
    while(1);

    /* never reached */
    return 0;
}