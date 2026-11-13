// Wide-screen (hor+) support.
//
// The perspective projection matrix is built by f_030c1990, which reads the camera descriptor's
// aspect field (+0xAC) into f13 at 0x030c199c and folds it into the horizontal-FOV term a few
// instructions later (`fmuls f8, f12, f13` at 0x030c19ac). The main camera stores 16:9 in that
// field, so we rewrite f13 to the display aspect right after the load. That widens the horizontal
// FOV while keeping the vertical FOV unchanged (hor+), so the widened render target is filled
// without stretching or cropping. Square shadow/reflection cameras (1.0) and 4:3 scopes are left
// untouched by the 16:9 check.
#include <math.h>

#include "hooks.h"

extern float g_aspect_extra;

BOTW_HOOK(0x030c199c) {
    if (g_aspect_extra <= 1.0f) return;
    const double k16x9 = 16.0 / 9.0;
    if (fabs(c->f[13] - k16x9) < 0.01) {
        c->f[13] = (double)g_aspect_extra * k16x9;
    }
}
