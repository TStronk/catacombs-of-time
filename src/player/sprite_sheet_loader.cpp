#include "sprite_sheet_loader.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/classes/atlas_texture.hpp>
#include <godot_cpp/classes/sprite_frames.hpp>
#include <godot_cpp/variant/vector2i.hpp>
#include <godot_cpp/variant/rect2i.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/array.hpp>

#include <vector>
#include <string>

using namespace godot;

void SpriteSheetLoader::_bind_methods() {
    // Bind methods if you want to expose this to GDScript
    ClassDB::bind_method(D_METHOD("set_frame_size", "size"), &SpriteSheetLoader::set_frame_size);
    ClassDB::bind_method(D_METHOD("get_frame_size"), &SpriteSheetLoader::get_frame_size);
    ClassDB::bind_method(D_METHOD("set_grid_size", "size"), &SpriteSheetLoader::set_grid_size);
    ClassDB::bind_method(D_METHOD("get_grid_size"), &SpriteSheetLoader::get_grid_size);
}

SpriteSheetLoader::SpriteSheetLoader() 
    : frame_size(64, 64), grid_size(0, 0), frame_count(0) {
}

SpriteSheetLoader::~SpriteSheetLoader() {
}

void SpriteSheetLoader::set_frame_size(const Vector2i& size) {
    frame_size = size;
}

void SpriteSheetLoader::set_grid_size(const Vector2i& size) {
    grid_size = size;
    frame_count = size.x * size.y;
}

Ref<SpriteFrames> SpriteSheetLoader::load_from_texture(
    const Ref<Texture2D>& atlas_texture,
    const Dictionary& animations
) {
    if (atlas_texture.is_null()) {
        ERR_PRINT("SpriteSheetLoader: Atlas texture is null!");
        return Ref<SpriteFrames>();
    }
    
    // Create new SpriteFrames resource
    Ref<SpriteFrames> sprite_frames;
    sprite_frames.instantiate();
    
    // Calculate grid size if not set
    if (grid_size.x <= 0 || grid_size.y <= 0) {
        calculate_grid_size(atlas_texture);
    }
    
    // Remove default animation if it exists
    if (sprite_frames->has_animation("default")) {
        sprite_frames->remove_animation("default");
    }
    
    // Add each animation from Dictionary
    Array keys = animations.keys();
    for (int i = 0; i < keys.size(); i++) {
        String name = keys[i];
        Dictionary anim_data = animations[name];
        
        AnimationDef def;
        def.start_frame = anim_data.get("start", 0);
        def.end_frame = anim_data.get("end", 0);
        def.fps = anim_data.get("fps", 10.0f);
        def.loop = anim_data.get("loop", true);
        
        add_animation_to_frames(sprite_frames, atlas_texture, name, def);
    }
    
    // If no animations were added, add a default one
    if (sprite_frames->get_animation_names().size() == 0) {
        UtilityFunctions::print("Warning: No animations added, creating default animation");
        sprite_frames->add_animation("default");
        sprite_frames->add_frame("default", atlas_texture);
    }
    
    return sprite_frames;
}

Ref<SpriteFrames> SpriteSheetLoader::load_simple(
    const Ref<Texture2D>& atlas_texture,
    const std::vector<std::pair<String, int>>& anim_frame_counts
) {
    Dictionary animations;
    int current_frame = 0;
    
    // Convert simple frame counts to Dictionary
    for (const auto& [name, count] : anim_frame_counts) {
        Dictionary anim_data;
        anim_data["start"] = current_frame;
        anim_data["end"] = current_frame + count - 1;
        anim_data["fps"] = 10.0f;
        anim_data["loop"] = true;
        animations[name] = anim_data;
        current_frame += count;
    }
    
    return load_from_texture(atlas_texture, animations);
}

Ref<SpriteFrames> SpriteSheetLoader::load_auto_grid(
    const Ref<Texture2D>& atlas_texture,
    const Dictionary& animations
) {
    if (atlas_texture.is_null()) {
        ERR_PRINT("SpriteSheetLoader: Atlas texture is null!");
        return Ref<SpriteFrames>();
    }
    
    // Auto-calculate grid size
    calculate_grid_size(atlas_texture);
    
    return load_from_texture(atlas_texture, animations);
}

void SpriteSheetLoader::add_animation_to_frames(
    Ref<SpriteFrames>& sprite_frames,
    const Ref<Texture2D>& atlas,
    const String& anim_name,
    const AnimationDef& def
) {
    if (sprite_frames.is_null() || atlas.is_null()) {
        ERR_PRINT("SpriteSheetLoader: Invalid parameters for add_animation_to_frames");
        return;
    }
    
    // Validate frame indices
    if (!validate_frame_index(def.start_frame) || !validate_frame_index(def.end_frame)) {
        ERR_PRINT(vformat("Invalid frame range for animation '%s': %d-%d (max: %d)", 
                         anim_name, def.start_frame, def.end_frame, frame_count - 1));
        return;
    }
    
    if (def.start_frame > def.end_frame) {
        ERR_PRINT(vformat("Invalid frame range for animation '%s': start > end", anim_name));
        return;
    }
    
    // Add the animation
    sprite_frames->add_animation(anim_name);
    sprite_frames->set_animation_loop(anim_name, def.loop);
    sprite_frames->set_animation_speed(anim_name, def.fps);
    
    // Add frames
    int frames_added = 0;
    for (int i = def.start_frame; i <= def.end_frame; i++) {
        Ref<AtlasTexture> frame = create_atlas_frame(atlas, i);
        if (frame.is_valid()) {
            sprite_frames->add_frame(anim_name, frame);
            frames_added++;
        }
    }
    
    UtilityFunctions::print(vformat("Added animation '%s' with %d frames", anim_name, frames_added));
}

Ref<AtlasTexture> SpriteSheetLoader::create_atlas_frame(const Ref<Texture2D>& atlas, int frame_index) {
    if (!validate_frame_index(frame_index)) {
        return Ref<AtlasTexture>();
    }
    
    // Calculate position in grid
    int col = frame_index % grid_size.x;
    int row = frame_index / grid_size.x;
    
    // Create AtlasTexture for this frame
    Ref<AtlasTexture> frame_texture;
    frame_texture.instantiate();
    frame_texture->set_atlas(atlas);
    
    // Set the region for this frame
    Rect2i region(
        col * frame_size.x,
        row * frame_size.y,
        frame_size.x,
        frame_size.y
    );
    frame_texture->set_region(region);
    
    // Optional: Set filter for pixel art
    // frame_texture->set_filter_clip(true);
    
    return frame_texture;
}

void SpriteSheetLoader::calculate_grid_size(const Ref<Texture2D>& texture) {
    if (texture.is_null() || frame_size.x <= 0 || frame_size.y <= 0) {
        ERR_PRINT("Cannot calculate grid size: invalid texture or frame size");
        return;
    }
    
    Vector2i texture_size = texture->get_size();
    grid_size.x = texture_size.x / frame_size.x;
    grid_size.y = texture_size.y / frame_size.y;
    frame_count = grid_size.x * grid_size.y;
    
    UtilityFunctions::print(vformat("Auto-detected grid size: %dx%d (%d frames)", 
                                   grid_size.x, grid_size.y, frame_count));
}

bool SpriteSheetLoader::validate_frame_index(int index) const {
    return index >= 0 && index < frame_count;
}