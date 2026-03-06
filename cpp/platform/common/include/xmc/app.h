#ifndef XMC_APP_H
#define XMC_APP_H

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * User defined setup function. This will be called once at the beginning of the
 * program. You can use this function to initialize your application, set up
 * peripherals, etc.
 */
void xmc_app_setup();

/**
 * User defined loop function. This will be called repeatedly after
 * xmc_app_setup. You can use this function to implement the main logic of
 * your application.
 */
void xmc_app_loop();

#if defined(__cplusplus)
}
#endif

#endif
