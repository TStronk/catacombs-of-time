#ifndef SPRITE_SHEET_LOADER_H
#define SPRITE_SHEET_LOADER_H

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/sprite_frames.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/atlas_texture.hpp>
#include <godot_cpp/variant/vector2i.hpp>
#include <godot_cpp/variant/rect2i.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/dictionary.hpp>

#include <vector>

namespace godot {

// Structure to define an animation
struct AnimationDef {
    int start_frame;      // First frame index in the sprite sheet
    int end_frame;        // Last frame index (inclusive)
    float fps;            // Playback speed
    bool loop;            // Whether the animation loops
    
    AnimationDef(int start = 0, int end = 0, float speed = 10.0f, bool should_loop = true)
        : start_frame(start), end_frame(end), fps(speed), loop(should_loop) {}
};

// Helper class to load sprite sheets and create SpriteFrames resources
class SpriteSheetLoader : public RefCounted {
    GDCLASS(SpriteSheetLoader, RefCounted)

private:
    Vector2i frame_size;      // Size of each frame in pixels
    Vector2i grid_size;       // Number of columns and rows in the sprite sheet
    int frame_count;          // Total number of frames
    
protected:
    static void _bind_methods();

public:
    SpriteSheetLoader();
    ~SpriteSheetLoader();
    
    // Configuration
    void set_frame_size(const Vector2i& size);
    Vector2i get_frame_size() const { return frame_size; }
    
    void set_grid_size(const Vector2i& size);
    Vector2i get_grid_size() const { return grid_size; }
    
    // Main loading function - takes a texture and animation definitions
    Ref<SpriteFrames> load_from_texture(
        const Ref<Texture2D>& atlas_texture,
        const Dictionary& animations  // Changed from std::unordered_map
    );
    
    // Simplified loading - just provide frame counts per animation
    Ref<SpriteFrames> load_simple(
        const Ref<Texture2D>& atlas_texture,
        const std::vector<std::pair<String, int>>& anim_frame_counts
    );
    
    // Load with automatic grid detection
    Ref<SpriteFrames> load_auto_grid(
        const Ref<Texture2D>& atlas_texture,
        const Dictionary& animations  // Changed from std::unordered_map
    );
    
    // Utility function to create a single animation
    void add_animation_to_frames(
        Ref<SpriteFrames>& sprite_frames,
        const Ref<Texture2D>& atlas,
        const String& anim_name,
        const AnimationDef& def
    );
    
private:
    // Create an AtlasTexture for a specific frame
    Ref<AtlasTexture> create_atlas_frame(const Ref<Texture2D>& atlas, int frame_index);
    
    // Calculate grid size from texture dimensions
    void calculate_grid_size(const Ref<Texture2D>& texture);
    
    // Validate frame indices
    bool validate_frame_index(int index) const;
};

} // namespace godot

#endif // SPRITE_SHEET_LOADER_H