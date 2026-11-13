// BotWRecomp: built-in resolution scaling. The texture redefine rules of Cemu's BotW Graphics pack
// (botw_texture_rules.inc) resize the game's render targets by g_render_scale; Cemu's texture cache
// and render-target code then scale viewports and scissors to match.
#include "Cafe/HW/Latte/ISA/LatteReg.h"
#include "Cafe/GraphicPack/GraphicPack2.h"

#include <cmath>

extern float g_render_scale;    // runtime: --scale
extern float g_aspect_extra;    // runtime: hor+ horizontal widening (1.0 = off)

namespace {
struct Row {
    int width, height, depth;
    std::vector<unsigned int> formats, formats_excluded;
    std::vector<int> tilemodes, tilemodes_excluded;
    int overwrite_format;
    int ow;
    double ow_add;
    int oh;
    double oh_add;
};
const Row kRows[] = {
#include "botw_texture_rules.inc"
};
}  // namespace

const std::vector<std::shared_ptr<GraphicPack2>>& GraphicPack2::GetActiveGraphicPacks() {
    static std::vector<std::shared_ptr<GraphicPack2>> packs = [] {
        std::vector<std::shared_ptr<GraphicPack2>> v;
        // hor+ has to resize the screen targets even at scale 1.0, otherwise the widened camera
        // projects the HUD/menus past the right edge of the 16:9 buffer and they get clipped.
        if (g_render_scale == 1.0f && g_aspect_extra <= 1.0f) return v;
        auto gp = std::make_shared<GraphicPack2>();
        gp->m_name = "BotWRecomp resolution";
        for (const Row& r : kRows) {
            TextureRule rule;
            auto& f = rule.filter_settings;
            f.width = r.width;
            f.height = r.height;
            f.depth = r.depth;
            f.format_whitelist = r.formats;
            f.format_blacklist = r.formats_excluded;
            f.tilemode_whitelist = r.tilemodes;
            f.tilemode_blacklist = r.tilemodes_excluded;
            auto& o = rule.overwrite_settings;
            // hor+: widen only the screen-space render targets, so their aspect follows the display.
            // Square targets (shadow maps, LUTs) keep their size.
            const bool screenTarget = r.oh > 0 && std::fabs((double)r.ow / (double)r.oh - 16.0 / 9.0) < 0.02;
            const double widthScale = g_render_scale * (screenTarget ? g_aspect_extra : 1.0f);
            o.width = (int)(widthScale * r.ow + r.ow_add);  // the pack's expressions truncate
            o.height = (int)(g_render_scale * r.oh + r.oh_add);
            o.format = r.overwrite_format;
            gp->m_rules.push_back(rule);
        }
        v.push_back(gp);
        return v;
    }();
    return packs;
}
