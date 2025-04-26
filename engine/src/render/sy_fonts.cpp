#include "sy_fonts.hpp"

#include "sy_resources.hpp"

#include "sy_utils.hpp"

#include "freetype_include.hpp"

void sy_render_destroy_font(SyRenderInfo *render_info, SyEcs *ecs, SyFont *font)
{
    SyRenderImage *image = ecs->component_from_index<SyRenderImage>(font->texture_index);
    sy_render_destroy_render_image(render_info->logical_device, render_info->vma_allocator, image);
    ecs->release_component<SyRenderImage>(font->texture_index);
    font->~SyFont();
}

SyFont sy_render_create_font(SyRenderInfo *render_info, SyEcs *ecs, const char *font_path, uint32_t texture_width, uint32_t texture_height, uint32_t character_width, const char *characters, uint32_t spacing)
{
    SyFont result;
    result.character_max_width = character_width;
    result.texture_dimensions = glm::uvec2(texture_width, texture_height);
    
    FT_Library ft;
    SY_ERROR_COND(FT_Init_FreeType(&ft) != 0, "Failed to init freetype.");
    
    size_t font_data_size;
    FT_Byte *font_data = (FT_Byte*)sy_read_resource_file(font_path, &font_data_size);
    SY_ERROR_COND(font_data == NULL, "Failed to read font resource %s", font_path);

    FT_Face face;
    SY_ERROR_COND(FT_New_Memory_Face(ft, font_data, font_data_size, 0, &face), "Failed to load font file.");

    
    FT_Set_Pixel_Sizes(face, character_width, 0);

    result.line_height = (float)face->size->metrics.height / 64.0f / (float)character_width;
    
    
    uint8_t *pixels = (uint8_t*)calloc(texture_width * texture_height, sizeof(uint8_t));

    struct Box
    {
	uint32_t left;
	uint32_t right;
	uint32_t bottom;
	uint32_t top;
    };

    size_t characters_size = strlen(characters);

    Box *boxes = (Box*)calloc(characters_size, sizeof(Box));

    size_t current_x = spacing;

    for (size_t i = 0; i < characters_size; ++i)
    {
	char current_char = characters[i];
	SY_ERROR_COND(FT_Load_Char(face, current_char, FT_LOAD_RENDER), "Failed to load font glyph.");	
	
	// Load current box

	if (current_x + face->glyph->bitmap.width + spacing > texture_width)
	{
	    current_x = spacing;
	}

	Box *current_box = &boxes[i];
	current_box->left = current_x;
	current_box->right = current_x + face->glyph->bitmap.width;
	current_box->top = spacing;
	current_box->bottom = current_box->top + face->glyph->bitmap.rows;

	// Resolve collisions with other boxes
	for (size_t j = 0; j < i; ++j)
	{
	    // check for collisions
	    Box *col_box = &boxes[j];

	    // See if x values align
	    bool x_values_align = false;

	    if (current_box->left < col_box->right + spacing && current_box->left >= col_box->left - spacing)
	    {
		x_values_align = true;
	    }

	    if (current_box->right <= col_box->right + spacing && current_box->right > col_box->left - spacing)
	    {
		x_values_align = true;
	    }

	    if (current_box->left <= col_box->left - spacing && current_box->right >= col_box->right + spacing)
	    {
		x_values_align = true;
	    }


	    if (x_values_align == false)
		continue;

	    // Check / Offset based on y values
	    if (current_box->top < col_box->bottom + spacing)
	    {
		current_box->top = col_box->bottom + spacing;
	    }

	}
	current_box->bottom = current_box->top + face->glyph->bitmap.rows;
	current_box->right = current_box->left + face->glyph->bitmap.width;

	SY_ERROR_COND(current_box->bottom > texture_height && current_box->right > texture_width, "Not enough space in texture for font atlas.");

	// Write to texture at box position
	{
	    SyFontCharacter font_character =
		{
		    .tex_bottom_left = glm::uvec2(current_box->left, current_box->top),
		    .tex_top_right = glm::uvec2(current_box->right, current_box->bottom),
		    .scale = glm::vec2((float)face->glyph->bitmap.width / (float)character_width, (float)face->glyph->bitmap.rows / (float)character_width),
		    .offset = glm::vec2((float)face->glyph->bitmap_left / (float)character_width, -1 * (float)face->glyph->bitmap_top / (float)character_width),
		    .advance = (float)face->glyph->advance.x / 64.0f / (float)character_width,
		};

	    result.character_map.insert({current_char, font_character});
	    

	    for (size_t y = 0; y < face->glyph->bitmap.rows; ++y)
	    {
		for (size_t x = 0; x < face->glyph->bitmap.width; ++x)
		{
		    pixels[(y + current_box->top)*texture_width + (x + current_box->left)] = face->glyph->bitmap.buffer[y * face->glyph->bitmap.width + x];
		}
	    }
	}
	current_x += face->glyph->bitmap.width + spacing;
    }

    size_t texture_index = ecs->get_unused_component<SyRenderImage>();
    SyRenderImage *image = ecs->component_from_index<SyRenderImage>(texture_index);

    result.texture_index = texture_index;

    *image = sy_render_create_texture_image(render_info, (void*)pixels, VkExtent2D{.width=texture_width, .height=texture_height}, VK_FORMAT_R8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);

    free(boxes);
    free(pixels);

    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    free(font_data);

    return result;
}
