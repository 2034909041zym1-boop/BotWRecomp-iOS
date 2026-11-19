// BotWRecomp: built-in resolution scaling. The texture redefine rules of Cemu's BotW Graphics pack
// (botw_texture_rules.inc) resize the game's render targets by g_render_scale; Cemu's texture cache
// and render-target code then scale viewports and scissors to match.
#include "Cafe/HW/Latte/ISA/LatteReg.h"
#include "Cafe/GraphicPack/GraphicPack2.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>

#include "../src/log.h"

extern float g_render_scale;    // runtime: --scale
extern float g_aspect_extra;    // runtime: hor+ horizontal widening (1.0 = off)
extern int g_shadow_half;       // runtime: settings.ini shadow_half

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
        if (g_render_scale == 1.0f && g_aspect_extra <= 1.0f && !g_shadow_half) return v;
        auto gp = std::make_shared<GraphicPack2>();
        gp->m_name = "BotWRecomp resolution";
        if (g_shadow_half) {
            for (int shadowDepth : {2, 3, 4}) {
                TextureRule sr;
                sr.filter_settings.width = 1024;
                sr.filter_settings.height = 1024;
                sr.filter_settings.depth = shadowDepth;
                sr.filter_settings.format_whitelist = {0x005, 0x011, 0x811};
                sr.overwrite_settings.width = 512;
                sr.overwrite_settings.height = 512;
                gp->m_rules.push_back(sr);
            }
        }
        if (g_render_scale == 1.0f && g_aspect_extra <= 1.0f) {
            v.push_back(gp);
            return v;
        }
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
            // A single line of blyt text is rendered into its own RGBA8 (0x01a) + tiled
            // (0x004) strip, from the 293x185 title banner down to 464x28 body text.
            // Stretching or shrinking glyph strips (0x01a + 0x004) corrupts glyph UVs,
            // causing missing/clipped characters in TextBoxes.
            // (0x816 is World/Weapon Bloom & heatwave; 0x001+0x004 is 640x360 DOF mips —
            // those MUST scale with g_render_scale and g_aspect_extra, not stay locked at 1x.)
            const bool glyphStrip =
                (r.formats.size() == 1 && r.formats[0] == 0x01a &&
                 r.tilemodes.size() == 1 && r.tilemodes[0] == 0x004);
            if (glyphStrip && (g_aspect_extra > 1.0f || g_render_scale < 1.0f)) {
                o.width = r.ow;
                o.height = r.oh;
            } else {
                const double widthScale = g_render_scale * g_aspect_extra;
                // Keep exact proportional widths (matching Cemu's Graphics pack rules.txt);
                // rounding small targets up to 32px in hor+ distorts their width ratio by 20-60%
                // and breaks rescale-ratio matching across the Bloom/Exposure mip pyramid.
                int ow = std::max(1, (int)(widthScale * r.ow + r.ow_add));
                o.width = ow;
                o.height = std::max(1, (int)(g_render_scale * r.oh + r.oh_add));
            }
            // Cemu's 10-bit -> 16-bit UNORM color-buffer upgrade (0x019 -> 0x01f) requires matching
            // custom shaders; without them it changes buffer bit depth and washes the picture out.
            // Since FindCustomShaderSource is disabled here, always keep native buffer formats.
            o.format = -1;
            gp->m_rules.push_back(rule);
        }
        v.push_back(gp);
        return v;
    }();
    return packs;
}

namespace {
struct CustomMslRow {
    uint64_t baseHash;
    uint64_t auxHash;
    GraphicPack2::GP_SHADER_TYPE type;
    const char* tmpl;
};

struct CustomMslEntry {
    uint64_t baseHash;
    uint64_t auxHash;
    GraphicPack2::GP_SHADER_TYPE type;
    std::string source;
};

void replace_all(std::string& s, const char* from, const std::string& to) {
    const size_t from_len = std::strlen(from);
    size_t pos = 0;
    while ((pos = s.find(from, pos)) != std::string::npos) {
        s.replace(pos, from_len, to);
        pos += to.size();
    }
}

std::vector<CustomMslEntry> build_custom_msl_shaders() {
    static const CustomMslRow kMslRows[] = {
#include "botw_msl_shaders.inc"
    };
    char wbuf[32], hbuf[32];
    std::snprintf(wbuf, sizeof(wbuf), "%.4f", 1280.0 * (double)g_render_scale * (double)g_aspect_extra);
    std::snprintf(hbuf, sizeof(hbuf), "%.4f", 720.0 * (double)g_render_scale);
    const std::string w_str(wbuf), h_str(hbuf);
    const std::string gw_str("1280.0"), gh_str("720.0");
    const std::string shadow_str(g_shadow_half ? "0.5" : "1.0");
    const std::string uw_str(g_aspect_extra > 1.0f ? "1" : "0");

    std::vector<CustomMslEntry> out;
    out.reserve(sizeof(kMslRows) / sizeof(kMslRows[0]));
    for (const auto& r : kMslRows) {
        std::string src(r.tmpl);
        replace_all(src, "$gameWidth", gw_str);
        replace_all(src, "$gameHeight", gh_str);
        replace_all(src, "$width", w_str);
        replace_all(src, "$height", h_str);
        replace_all(src, "$shadowRes", shadow_str);
        replace_all(src, "$ultrawideHUDMode", uw_str);
        replace_all(src, "$fxaa", "1");
        replace_all(src, "$disableDOF", "0");
        out.push_back({r.baseHash, r.auxHash, r.type, std::move(src)});
    }
    return out;
}
}  // namespace

const std::string* GraphicPack2::FindCustomShaderSource(uint64_t baseHash, uint64_t auxHash, GP_SHADER_TYPE type, bool isVulkan, bool isMetal) {
    (void)isVulkan;
    if (!isMetal) return nullptr;
    if (g_render_scale == 1.0f && g_aspect_extra <= 1.0f && !g_shadow_half) return nullptr;

    static const std::vector<CustomMslEntry> kShaders = build_custom_msl_shaders();
    for (const auto& e : kShaders) {
        if (e.baseHash == baseHash && e.auxHash == auxHash && e.type == type) {
            LOG("[gfx-pack] custom MSL shader %016llx_%016llx type=%d",
                (unsigned long long)baseHash, (unsigned long long)auxHash, (int)type);
            return &e.source;
        }
    }
    return nullptr;
}
