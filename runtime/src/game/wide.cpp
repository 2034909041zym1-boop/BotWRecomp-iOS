// Wide-screen (hor+) support: port of Cemu's graphic-pack aspect-ratio patch for BotW v208
// (src/BreathOfTheWild/Graphics/patch_AspectRatio.asm, moduleMatches 0x6267BFD0). The game keeps
// the 16:9 projection aspect in three .rodata slots and reloads it every frame for the 3D scene and
// the inventory preview; rewriting the slots plus those two loads widens the horizontal FOV while
// the vertical FOV stays as authored. The render targets are widened by the same factor in
// graphic_pack_shim.cpp, so the extra horizontal area is real geometry rather than a stretch.
#include <math.h>

#include "hooks.h"

extern float g_aspect_extra;

namespace {
constexpr float k16x9 = 16.0f / 9.0f;
constexpr uint32_t kAspectSlots[] = {0x101BF8E8, 0x1030A57C, 0x1036DD4C};  // v208 rodata aspect copies
}  // namespace

void wide_apply(float aspect) {
    for (uint32_t addr : kAspectSlots) mem::wf32(addr, aspect);
    // v208: the second grass-culling extent is halved for ultrawide (Cemu: 120 * 0.5).
    mem::wf32(0x1047BFB8, 60.0f);
    LOG("[wide] v208 aspect slots = %.4f (grass culling 60)", aspect);
}

// Every-frame 3D projection: v208 loads the aspect here into f12 (Cemu patches the lis/lfs pair).
BOTW_HOOK(0x0386D020) {
    if (g_aspect_extra <= 1.0f) return;
    c->f[12] = (double)(k16x9 * g_aspect_extra);
}

// Inventory 3D preview: v208 loads the aspect here into f4.
BOTW_HOOK(0x02E2C578) {
    if (g_aspect_extra <= 1.0f) return;
    c->f[4] = (double)(k16x9 * g_aspect_extra);
}

// First grass-culling factor: the ratio between the authored and the actual aspect.
BOTW_HOOK(0x035b0a38) {
    if (g_aspect_extra <= 1.0f) return;
    c->f[11] = (double)(1.0f / g_aspect_extra);
}
