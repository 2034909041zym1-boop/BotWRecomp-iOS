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
        // hor+ needs the screen targets resized even at scale 1.0, otherwise the widened camera
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
            // hor+: every screen-space target scales by the same width factor the game's own
            // resolution rule uses (Cemu's aspect-ratio pack does the same), so the non-16:9
            // targets stay consistent with the main framebuffer.
            const double widthScale = g_render_scale * g_aspect_extra;
            // GX2 textures expect a 32-pixel pitch alignment; 1280 * 1.2229 = 1565 is not aligned.
            int ow = (int)(widthScale * r.ow + r.ow_add);  // the pack's expressions truncate
            if (g_aspect_extra > 1.0f) {
                // Only the full-screen 16:9 targets are genuinely widened by hor+. The rest of
                // the resolution rule set is authored at a fixed pixel size and laid out by the
                // game in that exact space: the font atlases (1024x512 / 512x512 / 512x256, R8 =
                // 0x816) and the per-line glyph buffers (512x96 / 480x32 / 464x28). Stretching
                // those horizontally corrupts the glyph UVs, so every TextBox draws nothing while
                // sprite-based HUD numbers still look fine. Keep their authored width.
                const bool onlyR8 = (r.formats.size() == 1 && r.formats[0] == 0x816);
                const bool full169 =
                    r.height > 0 && std::fabs((double)r.width / (double)r.height - 16.0 / 9.0) < 0.02;
                ow = (!full169 || onlyR8) ? r.ow : ((ow + 31) & ~31);
            }
            o.width = ow;
            o.height = (int)(g_render_scale * r.oh + r.oh_add);
            // Cemu's 10-bit -> 16-bit UNORM color-buffer upgrade is a *resolution* feature, not an
            // aspect-ratio feature. In hor+ (wider frustum, scale stays 1.0) rewriting the HDR
            // buffers changes their bit depth without the matching tonemap path and washes the
            // picture out. Skip format overrides whenever we only widen.
            o.format = (g_aspect_extra > 1.0f) ? -1 : r.overwrite_format;
            gp->m_rules.push_back(rule);
        }
        v.push_back(gp);
        return v;
    }();
    return packs;
}
