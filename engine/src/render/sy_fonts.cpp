#include "sy_fonts.hpp"

#include "sy_resources.hpp"

#include "freetype_include.hpp"

SyFont sy_render_create_font(SyRenderInfo *render_info, const char *font_path, uint32_t texture_width, uint32_t texture_height, uint32_t character_width, const char *characters)
{
    SyFont result;
    
    FT_Library ft;
    SY_ERROR_COND(FT_Init_FreeType(&ft) != 0, "Failed to init freetype.");
    
    FT_Face face;
    SY_ERROR_COND(FT_New_Face(ft, font_path, 0, &face), "Failed to load font file.");
    
    
    FT_Set_Pixel_Sizes(face, character_width, 0);
    
    
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

    size_t current_x = 0;

    for (size_t i = 0; i < characters_size; ++i)
    {
	char current_char = characters[i];
	SY_ERROR_COND(FT_Load_Char(face, current_char, FT_LOAD_RENDER), "Failed to load font glyph.");	
	
	// Load current box
	Box *current_box = &boxes[i];

	current_box->left = current_x;
	current_box->right = current_x + face->glyph->bitmap.width;
	current_box->top = 0;
	current_box->bottom = current_box->top + face->glyph->bitmap.rows;

	if (current_x + face->glyph->bitmap.width > texture_width)
	{
	    current_x = 0;
	}

	current_box->left = current_x;
	current_box->top = 0;

	// Resolve collisions with other boxes
	for (size_t j = 0; j < i; ++j)
	{
	    // check for collisions
	    Box *col_box = &boxes[j];

	    // See if x values align
	    bool x_values_align = false;
	    current_box->bottom = current_box->top + face->glyph->bitmap.rows;
	    current_box->right = current_box->left + face->glyph->bitmap.width;
	    if (current_box->left < col_box->right && current_box->left > col_box->left)
	    {
		x_values_align = true;
	    }

	    if (current_box->right < col_box->right && current_box->right > col_box->left)
	    {
		x_values_align = true;
	    }

	    if (current_box->left < col_box->left && current_box->right > col_box->right)
	    {
		x_values_align = true;
	    }

	    if (x_values_align == false)
		continue;

	    // Check / Offset based on y values
	    if (current_box->top < col_box->bottom)
	    {
		current_box->top = col_box->bottom;
	    }

	}
	current_box->bottom = current_box->top + face->glyph->bitmap.rows;
	current_box->right = current_box->left + face->glyph->bitmap.width;

	SY_ERROR_COND(current_box->bottom > texture_height, "Not enough space in texture for font atlas.");

	// Write to texture at box position
	{
	    SyFontCharacter font_character =
		{
		    .tex_bottom_left = glm::uvec2(current_box->left, current_box->top),
		    .tex_top_right = glm::uvec2(current_box->right, current_box->bottom)
		};
	    result.character_map.insert({current_char, font_character});


	    for (size_t y = 0; y < face->glyph->bitmap.rows; ++y)
	    {
		for (size_t x = 0; x < face->glyph->bitmap.width; ++x)
		{
		    pixels[(y + current_box->top)*texture_width + (x + current_box->left)] = face->glyph->bitmap.buffer[y * face->glyph->bitmap.width + x];
//		    printf("%c", face->glyph->bitmap.buffer[y * face->glyph->bitmap.width + x]? 'X' : ' ');

		}
//		printf("\n");
	    }
	}
//	printf("(%d, %d)\n===================================================\n\n", face->glyph->bitmap.width, face->glyph->bitmap.rows);
	
	current_x += face->glyph->bitmap.width;
    }

    SyRenderImage atlas = sy_render_create_texture_image(render_info, (void*)pixels, VkExtent2D{.width=texture_width, .height=texture_height}, VK_FORMAT_R8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);

    free(boxes);
    free(pixels);

    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    result.atlas = atlas;

    return result;
}
