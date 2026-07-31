/*-----------------------------------------------------------------------------

	SpriteLibBackendSDL.cpp

	SDL2 backend implementation for SpriteLib.
	Implements the unified backend interface using SDL2.

	2025.01.14

-----------------------------------------------------------------------------*/

#include "SpriteLibBackendSDL.h"
#include "../DXLib/CDirectDraw.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifndef SPRITELIB_SDL_VERBOSE_LOG
#define SPRITELIB_SDL_VERBOSE_LOG 0
#endif

#if SPRITELIB_SDL_VERBOSE_LOG
#define SPRITELIB_SDL_LOG(...) do { fprintf(stderr, __VA_ARGS__); } while (0)
#else
#define SPRITELIB_SDL_LOG(...) do {} while (0)
#endif

/* ============================================================================
 * Global State
 * ============================================================================ */

static int g_spritectl_initialized = 0;
static SDL_Renderer* g_spritectl_default_renderer = NULL;

/* ============================================================================
 * Initialization
 * ============================================================================ */

int spritectl_init(void) {
	if (g_spritectl_initialized) {
		return 0;  /* Already initialized */
	}

	/* Initialize SDL video subsystem */
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		fprintf(stderr, "SpriteLib Backend: SDL_Init failed: %s\n", SDL_GetError());
		return -1;
	}

	g_spritectl_initialized = 1;
	return 0;
}

void spritectl_shutdown(void) {
	if (!g_spritectl_initialized) {
		return;
	}

	/* Clean up default renderer */
	if (g_spritectl_default_renderer) {
		SDL_DestroyRenderer(g_spritectl_default_renderer);
		g_spritectl_default_renderer = NULL;
	}

	SDL_Quit();
	g_spritectl_initialized = 0;
}

/* ============================================================================
 * Surface Functions
 * ============================================================================ */

SDL_Surface* spritectl_sdl_create_surface(int width, int height, int format) {
	SDL_Surface* surf = NULL;

	/* Calculate depth and masks */
	switch (format) {
		case SPRITECTL_FORMAT_RGB565:
			/* RGB565: R at bits 11-15, G at bits 5-10, B at bits 0-4 */
			surf = SDL_CreateRGBSurface(0, width, height, 16,
			                            0xF800,  /* R mask */
			                            0x07E0,  /* G mask */
			                            0x001F,  /* B mask */
			                            0x0000); /* A mask */
			if (surf) {
				static int debug_565_count = 0;
				if (debug_565_count < 3) {
					SPRITELIB_SDL_LOG("Created RGB565 surface: requested format=RGB565, actual format=%s\n",
						SDL_GetPixelFormatName(surf->format->format));
					debug_565_count++;
				}
			}
			return surf;
		case SPRITECTL_FORMAT_RGB555:
			/* RGB555: R at bits 10-14, G at bits 5-9, B at bits 0-4 */
			surf = SDL_CreateRGBSurface(0, width, height, 16,
			                            0x7C00,  /* R mask */
			                            0x03E0,  /* G mask */
			                            0x001F,  /* B mask */
			                            0x0000); /* A mask */
			if (surf) {
				static int debug_555_count = 0;
				if (debug_555_count < 3) {
					SPRITELIB_SDL_LOG("Created RGB555 surface: requested format=RGB555, actual format=%s\n",
						SDL_GetPixelFormatName(surf->format->format));
					debug_555_count++;
				}
			}
			return surf;
		case SPRITECTL_FORMAT_RGBA32:
			/* RGBA32: standard byte order */
			return SDL_CreateRGBSurface(0, width, height, 32,
			                            0x000000FF,  /* R mask */
			                            0x0000FF00,  /* G mask */
			                            0x00FF0000,  /* B mask */
			                            0xFF000000); /* A mask */
		default:
			/* Default to RGB565 */
			surf = SDL_CreateRGBSurface(0, width, height, 16,
			                            0xF800,  /* R mask */
			                            0x07E0,  /* G mask */
			                            0x001F,  /* B mask */
			                            0x0000); /* A mask */
			if (surf) {
				SPRITELIB_SDL_LOG("Created DEFAULT surface: actual format=%s\n",
					SDL_GetPixelFormatName(surf->format->format));
			}
			return surf;
	}
}

uint32_t spritectl_sdl_get_pixelformat(int format) {
	switch (format) {
		case SPRITECTL_FORMAT_RGB565:
			return SDL_PIXELFORMAT_RGB565;
		case SPRITECTL_FORMAT_RGB555:
			return SDL_PIXELFORMAT_RGB555;
		case SPRITECTL_FORMAT_RGBA32:
			return SDL_PIXELFORMAT_RGBA32;
		default:
			return SDL_PIXELFORMAT_UNKNOWN;
	}
}

spritectl_surface_t spritectl_create_surface(int width, int height, int format) {
	spritectl_surface_t surface;

	if (!g_spritectl_initialized) {
		fprintf(stderr, "SpriteLib Backend: Not initialized\n");
		return SPRITECTL_INVALID_SURFACE;
	}

	/* Allocate surface structure */
	surface = (spritectl_surface_t)malloc(sizeof(struct spritectl_surface_s));
	if (!surface) {
		fprintf(stderr, "SpriteLib Backend: Failed to allocate surface\n");
		return SPRITECTL_INVALID_SURFACE;
	}

	/* Create SDL surface */
	surface->surface = spritectl_sdl_create_surface(width, height, format);
	if (!surface->surface) {
		free(surface);
		return SPRITECTL_INVALID_SURFACE;
	}

	/* Initialize fields */
	surface->texture = NULL;
	surface->renderer = NULL;
	surface->width = width;
	surface->height = height;
	surface->format = format;
	surface->locked = 0;
	surface->ref_count = 1;

	return surface;
}

void spritectl_destroy_surface(spritectl_surface_t surface) {
	if (!surface) {
		return;
	}

	surface->ref_count--;
	if (surface->ref_count > 0) {
		return;  /* Still referenced */
	}

	/* Free SDL resources */
	if (surface->texture) {
		SDL_DestroyTexture(surface->texture);
	}
	if (surface->surface) {
		SDL_FreeSurface(surface->surface);
	}

	free(surface);
}

int spritectl_lock_surface(spritectl_surface_t surface, spritectl_surface_info_t* info) {
	if (!surface || !info) {
		return -1;
	}

	/* Lock SDL surface */
	if (SDL_LockSurface(surface->surface) != 0) {
		fprintf(stderr, "SpriteLib Backend: Failed to lock surface: %s\n", SDL_GetError());
		return -1;
	}

	surface->locked++;

	/* Fill info structure */
	info->width = surface->width;
	info->height = surface->height;
	info->pitch = surface->surface->pitch;
	info->pixels = surface->surface->pixels;
	info->format = surface->format;

	return 0;
}

int spritectl_unlock_surface(spritectl_surface_t surface) {
	if (!surface || surface->locked == 0) {
		return -1;
	}

	SDL_UnlockSurface(surface->surface);
	surface->locked--;

	return 0;
}

int spritectl_clear_surface(spritectl_surface_t surface, uint32_t color) {
	SDL_Rect rect;

	if (!surface) {
		return -1;
	}

	/* Fill entire surface with color */
	rect.x = 0;
	rect.y = 0;
	rect.w = surface->width;
	rect.h = surface->height;

	SDL_FillRect(surface->surface, &rect, (Uint32)color);

	return 0;
}

int spritectl_get_surface_size(spritectl_surface_t surface, int* width, int* height) {
	if (!surface) {
		return -1;
	}

	if (width) *width = surface->width;
	if (height) *height = surface->height;

	return 0;
}

/* ============================================================================
 * Sprite Functions
 * ============================================================================ */

spritectl_sprite_t spritectl_create_sprite(int width, int height, int format,
                                           const void* pixels, size_t data_size) {
	spritectl_sprite_t sprite;

	if (!pixels || data_size == 0) {
		return SPRITECTL_INVALID_SPRITE;
	}

	/* Allocate sprite structure */
	sprite = (spritectl_sprite_t)malloc(sizeof(struct spritectl_sprite_s));
	if (!sprite) {
		fprintf(stderr, "SpriteLib Backend: Failed to allocate sprite\n");
		return SPRITECTL_INVALID_SPRITE;
	}

	/* Initialize fields */
	sprite->width = width;
	sprite->height = height;
	sprite->format = format;
	sprite->data_size = data_size;
	sprite->rgba_pixels = NULL;
	sprite->scanline_rle = NULL;
	sprite->scanline_lens = NULL;
	sprite->has_rle = 0;
	sprite->ref_count = 1;

	/* Allocate and copy pixel data */
	sprite->pixels = (uint16_t*)malloc(data_size);
	if (!sprite->pixels) {
		free(sprite);
		return SPRITECTL_INVALID_SPRITE;
	}
	memcpy(sprite->pixels, pixels, data_size);

	return sprite;
}

void spritectl_destroy_sprite(spritectl_sprite_t sprite) {
	if (!sprite) {
		return;
	}

	sprite->ref_count--;
	if (sprite->ref_count > 0) {
		return;  /* Still referenced */
	}

	/* Free pixel data */
	if (sprite->pixels) {
		free(sprite->pixels);
	}
	if (sprite->rgba_pixels) {
		free(sprite->rgba_pixels);
	}

	/* Free RLE data */
	if (sprite->scanline_rle) {
		for (int y = 0; y < sprite->height; y++) {
			if (sprite->scanline_rle[y]) {
				free(sprite->scanline_rle[y]);
			}
		}
		free(sprite->scanline_rle);
	}
	if (sprite->scanline_lens) {
		free(sprite->scanline_lens);
	}

	free(sprite);
}

int spritectl_get_sprite_info(spritectl_sprite_t sprite, spritectl_sprite_info_t* info) {
	if (!sprite || !info) {
		return -1;
	}

	info->width = sprite->width;
	info->height = sprite->height;
	info->format = sprite->format;
	info->data_size = sprite->data_size;

	return 0;
}

size_t spritectl_get_sprite_data(spritectl_sprite_t sprite, void* buffer, size_t buffer_size) {
	if (!sprite) {
		return 0;
	}

	if (!buffer) {
		return sprite->data_size;  /* Return required size */
	}

	if (buffer_size < sprite->data_size) {
		return 0;  /* Buffer too small */
	}

	memcpy(buffer, sprite->pixels, sprite->data_size);
	return sprite->data_size;
}

/* ============================================================================
 * Blitting Functions
 * ============================================================================ */

/**
 * RLE-based sprite rendering - mimics original DirectX BltClipWidth
 *
 * This function directly processes RLE data and writes only non-transparent
 * pixels to the destination surface, exactly like the original DirectX implementation.
 *
 * Key difference from SDL blitting:
 * - Transparent pixels are SKIPPED, not blended
 * - This means the destination surface's original content shows through
 */
int spritectl_blt_sprite_rle(spritectl_surface_t dest, int x, int y,
                              spritectl_sprite_t sprite, int flags, int alpha) {
	if (!dest || !sprite || !sprite->has_rle || !sprite->scanline_rle) {
		return -1;
	}

	SDL_Surface* sdl_surface = dest->surface;
	if (!sdl_surface) {
		return -1;
	}

	/* Lock destination surface for direct pixel access */
	if (SDL_MUSTLOCK(sdl_surface)) {
		SDL_LockSurface(sdl_surface);
	}

	/* Get destination surface info */
	int dest_width = sdl_surface->w;
	int dest_height = sdl_surface->h;
	int dest_pitch = sdl_surface->pitch;
	int dest_bytes_per_pixel = sdl_surface->format->BytesPerPixel;
	int dest_stride = dest_pitch / dest_bytes_per_pixel;  /* Number of pixels per row */

	/* Calculate clipping rect */
	int clip_left = (x < 0) ? -x : 0;
	int clip_top = (y < 0) ? -y : 0;
	int clip_right = (x + sprite->width > dest_width) ? dest_width - x : sprite->width;
	int clip_bottom = (y + sprite->height > dest_height) ? dest_height - y : sprite->height;

	if (clip_left >= clip_right || clip_top >= clip_bottom) {
		if (SDL_MUSTLOCK(sdl_surface)) {
			SDL_UnlockSurface(sdl_surface);
		}
		return 0;  /* Completely clipped, but not an error */
	}

	/* Debug: print surface info on first call */
	static int debug_printed = 0;
	if (debug_printed < 3) {
		SPRITELIB_SDL_LOG("[RLE BLT] sprite=%dx%d, pos=(%d,%d), surface=%dx%d, bpp=%d, pitch=%d, stride=%d\n",
		        sprite->width, sprite->height, x, y, dest_width, dest_height, dest_bytes_per_pixel, dest_pitch, dest_stride);
		debug_printed++;
	}

	/* Process each scanline */
	for (int sy = clip_top; sy < clip_bottom; sy++) {
		if (!sprite->scanline_rle[sy] || sprite->scanline_lens[sy] == 0) {
			continue;  /* Empty scanline */
		}

		/* Get destination row pointer - use uint8_t* for format flexibility */
		uint8_t* dest_row_bytes = (uint8_t*)sdl_surface->pixels + (y + sy) * dest_pitch;

		/* Process RLE segments */
		uint16_t* rle_data = sprite->scanline_rle[sy];
		uint16_t rle_data_size = sprite->scanline_lens[sy];
		int rle_index = 0;

		/* Validate RLE data */
		if (rle_data_size < 1) {
			if (debug_printed < 3) {
				SPRITELIB_SDL_LOG("[RLE BLT] Invalid RLE data size: %d at scanline %d\n", rle_data_size, sy);
			}
			continue;
		}

		int seg_count = rle_data[rle_index++];
		if (rle_index + seg_count * 2 > rle_data_size) {
			if (debug_printed < 3) {
				SPRITELIB_SDL_LOG("[RLE BLT] RLE data corruption: seg_count=%d, size=%d at scanline %d\n",
				        seg_count, rle_data_size, sy);
			}
			continue;
		}

		int sx = 0;

		for (int seg = 0; seg < seg_count; seg++) {
			int trans_count = rle_data[rle_index++];
			int color_count = rle_data[rle_index++];

			/* Skip transparent pixels */
			sx += trans_count;

			/* Safety: if we've gone past sprite width, skip this entire segment */
			if (sx >= sprite->width) {
				rle_index += color_count;  /* Skip all color pixels in this segment */
				continue;  /* Move to next segment */
			}

			/* Copy color pixels */
			for (int c = 0; c < color_count && sx < sprite->width; c++) {
				/* Check if we're about to read past RLE data */
				if (rle_index >= rle_data_size) {
					if (debug_printed < 3) {
						SPRITELIB_SDL_LOG("[RLE BLT] RLE data overrun: rle_index=%d, size=%d, sx=%d\n",
						        rle_index, rle_data_size, sx);
					}
					break;
				}

				/* Check if this pixel is within clipping bounds */
				if (sx >= clip_left && sx < clip_right) {
					uint16_t pixel = rle_data[rle_index];

					/* Calculate actual destination position */
					int dest_x = x + sx;
					if (dest_x >= 0 && dest_x < dest_stride) {
						/* Write pixel based on destination format */
						if (dest_bytes_per_pixel == 2) {
							/* RGB565 destination - write directly */
							uint16_t* dest_row_16 = (uint16_t*)dest_row_bytes;
							dest_row_16[dest_x] = pixel;
						} else if (dest_bytes_per_pixel == 4) {
							/* RGBA32 destination - convert RGB565 to RGBA32 */
							uint8_t r, g, b;
							spritectl_565_to_rgb(pixel, &r, &g, &b);
							uint32_t* dest_row_32 = (uint32_t*)dest_row_bytes;

							/* Apply alpha blending if needed */
							if (flags & SPRITECTL_BLT_ALPHA) {
								/* Alpha blending with destination */
								uint32_t dest_pixel = dest_row_32[dest_x];
								uint8_t dest_r = dest_pixel & 0xFF;
								uint8_t dest_g = (dest_pixel >> 8) & 0xFF;
								uint8_t dest_b = (dest_pixel >> 16) & 0xFF;

								/* Blend: src * alpha/255 + dst * (1 - alpha/255) */
								uint8_t blend_r = (r * alpha + dest_r * (255 - alpha)) / 255;
								uint8_t blend_g = (g * alpha + dest_g * (255 - alpha)) / 255;
								uint8_t blend_b = (b * alpha + dest_b * (255 - alpha)) / 255;

								dest_row_32[dest_x] = (255 << 24) | (blend_b << 16) | (blend_g << 8) | blend_r;
							} else {
								/* Opaque write */
								dest_row_32[dest_x] = (255 << 24) | (b << 16) | (g << 8) | r;
							}
						} else if (debug_printed < 3) {
							SPRITELIB_SDL_LOG("[RLE BLT] Unsupported dest BPP: %d\n", dest_bytes_per_pixel);
						}
					} else if (debug_printed < 3) {
						SPRITELIB_SDL_LOG("[RLE BLT] OUT OF BOUNDS: dest_x=%d, stride=%d, sx=%d, x=%d\n",
						        dest_x, dest_stride, sx, x);
					}
				}

				rle_index++;  /* Consume pixel even if clipped */
				sx++;
			}
		}
	}

	/* Unlock destination surface */
	if (SDL_MUSTLOCK(sdl_surface)) {
		SDL_UnlockSurface(sdl_surface);
	}

	return 0;
}

spritectl_sprite_t spritectl_create_sprite_rle(int width, int height) {
	spritectl_sprite_t sprite;

	if (width <= 0 || height <= 0) {
		return SPRITECTL_INVALID_SPRITE;
	}

	/* Allocate sprite structure */
	sprite = (spritectl_sprite_t)malloc(sizeof(struct spritectl_sprite_s));
	if (!sprite) {
		fprintf(stderr, "SpriteLib Backend: Failed to allocate RLE sprite\n");
		return SPRITECTL_INVALID_SPRITE;
	}

	/* Initialize fields */
	sprite->width = width;
	sprite->height = height;
	sprite->format = SPRITECTL_FORMAT_RGB565;  /* Assume RGB565 for RLE sprites */
	sprite->data_size = 0;
	sprite->rgba_pixels = NULL;
	sprite->pixels = NULL;  /* RLE sprites don't have decoded pixels */
	sprite->ref_count = 1;
	sprite->has_rle = 1;  /* Mark as RLE sprite */

	/* Allocate RLE arrays */
	sprite->scanline_lens = (uint16_t*)calloc(height, sizeof(uint16_t));
	if (!sprite->scanline_lens) {
		free(sprite);
		return SPRITECTL_INVALID_SPRITE;
	}

	sprite->scanline_rle = (uint16_t**)calloc(height, sizeof(uint16_t*));
	if (!sprite->scanline_rle) {
		free(sprite->scanline_lens);
		free(sprite);
		return SPRITECTL_INVALID_SPRITE;
	}

	return sprite;
}

int spritectl_sprite_set_scanline_rle(spritectl_sprite_t sprite, int y,
                                       const uint16_t* rle_data, int rle_size) {
	if (!sprite || !sprite->has_rle) {
		return -1;
	}

	if (y < 0 || y >= sprite->height) {
		return -1;
	}

	if (rle_size <= 0) {
		return -1;
	}

	/* Free existing RLE data for this scanline */
	if (sprite->scanline_rle[y]) {
		free(sprite->scanline_rle[y]);
	}

	/* Allocate and copy RLE data */
	sprite->scanline_rle[y] = (uint16_t*)malloc(rle_size * sizeof(uint16_t));
	if (!sprite->scanline_rle[y]) {
		return -1;
	}

	memcpy(sprite->scanline_rle[y], rle_data, rle_size * sizeof(uint16_t));
	sprite->scanline_lens[y] = rle_size;

	return 0;
}

int spritectl_blt_sprite(spritectl_surface_t dest, int x, int y,
                         spritectl_sprite_t sprite, int flags, int alpha) {
	if (!dest || !sprite) {
		return -1;
	}

	/* If sprite has RLE data, use RLE-based rendering (like original DirectX) */
	if (sprite->has_rle && sprite->scanline_rle) {
		return spritectl_blt_sprite_rle(dest, x, y, sprite, flags, alpha);
	}

	/* fix: font glyphs are created as plain RGBA32 sprites (no RLE - see
	 * spritectl_create_sprite in TextBackendSDL.cpp), so every single
	 * character drawn fell through to the generic SDL_Surface path below:
	 * SDL_CreateRGBSurface/Lock/SetBlendMode/SetAlphaMod/BlitSurface every
	 * call. A CPU profile (Visual Studio, ~2min idle session) showed this
	 * one function at 57% of *total* process CPU time - because the 32bpp
	 * RGBA scratch surface never matches the game's 16bpp (555/565)
	 * backbuffer format, SDL_BlitSurface can't use its fast matching-format
	 * path and instead does a full per-pixel converting+blending blit
	 * through its generic blitter every call. Handle the common RGBA32
	 * case with a direct manual alpha blend straight into the destination
	 * pixels instead, skipping SDL's surface machinery entirely. */
	if (sprite->format == SPRITECTL_FORMAT_RGBA32 && sprite->pixels) {
		SDL_Surface* sdl_surface = dest->surface;
		if (!sdl_surface) {
			return -1;
		}

		bool was_locked_fast = (dest->locked > 0);
		if (!was_locked_fast) {
			if (SDL_MUSTLOCK(sdl_surface)) {
				SDL_LockSurface(sdl_surface);
			}
		}

		int dest_width = sdl_surface->w;
		int dest_height = sdl_surface->h;
		int dest_pitch = sdl_surface->pitch;
		int dest_bpp = sdl_surface->format->BytesPerPixel;

		int clip_left = (x < 0) ? -x : 0;
		int clip_top = (y < 0) ? -y : 0;
		int clip_right = (x + sprite->width > dest_width) ? dest_width - x : sprite->width;
		int clip_bottom = (y + sprite->height > dest_height) ? dest_height - y : sprite->height;

		if (clip_left < clip_right && clip_top < clip_bottom) {
			const uint32_t* src_pixels = (const uint32_t*)sprite->pixels;
			uint8_t* dest_bytes = (uint8_t*)sdl_surface->pixels;
			bool applyAlphaMod = (flags & SPRITECTL_BLT_ALPHA) != 0;

			for (int sy = clip_top; sy < clip_bottom; sy++) {
				const uint32_t* src_row = src_pixels + sy * sprite->width;
				uint8_t* dest_row_bytes = dest_bytes + (y + sy) * dest_pitch;

				for (int sx = clip_left; sx < clip_right; sx++) {
					uint32_t srcPixel = src_row[sx];
					uint8_t srcAlpha = (uint8_t)((srcPixel >> 24) & 0xFF);
					if (srcAlpha == 0) {
						continue;
					}
					uint8_t srcR = (uint8_t)(srcPixel & 0xFF);
					uint8_t srcG = (uint8_t)((srcPixel >> 8) & 0xFF);
					uint8_t srcB = (uint8_t)((srcPixel >> 16) & 0xFF);
					if (applyAlphaMod) {
						srcAlpha = (uint8_t)((srcAlpha * alpha) / 255);
						if (srcAlpha == 0) {
							continue;
						}
					}

					int dest_x = x + sx;

					if (dest_bpp == 2) {
						uint16_t* dest_pixel_ptr = (uint16_t*)(dest_row_bytes) + dest_x;
						if (srcAlpha == 255) {
							*dest_pixel_ptr = spritectl_rgb_to_565(srcR, srcG, srcB);
						} else {
							uint8_t dr, dg, db;
							spritectl_565_to_rgb(*dest_pixel_ptr, &dr, &dg, &db);
							uint8_t blendR = (uint8_t)((srcR * srcAlpha + dr * (255 - srcAlpha)) / 255);
							uint8_t blendG = (uint8_t)((srcG * srcAlpha + dg * (255 - srcAlpha)) / 255);
							uint8_t blendB = (uint8_t)((srcB * srcAlpha + db * (255 - srcAlpha)) / 255);
							*dest_pixel_ptr = spritectl_rgb_to_565(blendR, blendG, blendB);
						}
					} else if (dest_bpp == 4) {
						uint32_t* dest_pixel_ptr = (uint32_t*)(dest_row_bytes) + dest_x;
						if (srcAlpha == 255) {
							*dest_pixel_ptr = (255u << 24) | (srcB << 16) | (srcG << 8) | srcR;
						} else {
							uint32_t destPixel = *dest_pixel_ptr;
							uint8_t dr = (uint8_t)(destPixel & 0xFF);
							uint8_t dg = (uint8_t)((destPixel >> 8) & 0xFF);
							uint8_t db = (uint8_t)((destPixel >> 16) & 0xFF);
							uint8_t blendR = (uint8_t)((srcR * srcAlpha + dr * (255 - srcAlpha)) / 255);
							uint8_t blendG = (uint8_t)((srcG * srcAlpha + dg * (255 - srcAlpha)) / 255);
							uint8_t blendB = (uint8_t)((srcB * srcAlpha + db * (255 - srcAlpha)) / 255);
							*dest_pixel_ptr = (255u << 24) | (blendB << 16) | (blendG << 8) | blendR;
						}
					}
				}
			}
		}

		if (!was_locked_fast) {
			if (SDL_MUSTLOCK(sdl_surface)) {
				SDL_UnlockSurface(sdl_surface);
			}
		}

		return 0;
	}

	/* fix: shadow sprites (CShadowSprite, see get_backend_shadow_sprite in
	 * CSpriteSurface_Adapter.cpp) are created as plain RGB565 sprites with
	 * no RLE data - same anti-pattern as the RGBA32 glyph case above, just
	 * a different format. A CPU profile showed CSpriteSurface::BltShadowSprite
	 * / BltShadowSpriteDarkness alone accounting for roughly half of total
	 * game CPU (drawn for every visible creature/player every frame), almost
	 * entirely inside the SDL_BlitSurface fallback below. Colorkey
	 * transparency for these sprites is a simple binary check (pixel==0x0000
	 * is transparent, see spritectl_convert_565_to_rgba above), so this can
	 * skip SDL's surface machinery entirely too, same as the RGBA32 path. */
	if ((sprite->format == SPRITECTL_FORMAT_RGB565 || sprite->format == SPRITECTL_FORMAT_RGB555) && sprite->pixels) {
		SDL_Surface* sdl_surface = dest->surface;
		if (!sdl_surface) {
			return -1;
		}

		bool was_locked_fast = (dest->locked > 0);
		if (!was_locked_fast) {
			if (SDL_MUSTLOCK(sdl_surface)) {
				SDL_LockSurface(sdl_surface);
			}
		}

		int dest_width = sdl_surface->w;
		int dest_height = sdl_surface->h;
		int dest_pitch = sdl_surface->pitch;
		int dest_bpp = sdl_surface->format->BytesPerPixel;

		int clip_left = (x < 0) ? -x : 0;
		int clip_top = (y < 0) ? -y : 0;
		int clip_right = (x + sprite->width > dest_width) ? dest_width - x : sprite->width;
		int clip_bottom = (y + sprite->height > dest_height) ? dest_height - y : sprite->height;

		if (clip_left < clip_right && clip_top < clip_bottom) {
			const uint16_t COLORKEY = 0x0000;
			bool is555 = (sprite->format == SPRITECTL_FORMAT_RGB555);
			bool applyAlphaMod = (flags & SPRITECTL_BLT_ALPHA) != 0;
			const uint16_t* src_pixels = (const uint16_t*)sprite->pixels;
			uint8_t* dest_bytes = (uint8_t*)sdl_surface->pixels;

			for (int sy = clip_top; sy < clip_bottom; sy++) {
				const uint16_t* src_row = src_pixels + sy * sprite->width;
				uint8_t* dest_row_bytes = dest_bytes + (y + sy) * dest_pitch;

				for (int sx = clip_left; sx < clip_right; sx++) {
					uint16_t srcPixel = src_row[sx];
					if (srcPixel == COLORKEY) {
						continue;
					}

					int dest_x = x + sx;

					/* Fast path: opaque pixel, 565 source into 565 dest -
					 * copy the packed pixel directly, no 8-bit round-trip. */
					if (!applyAlphaMod && !is555 && dest_bpp == 2) {
						*((uint16_t*)(dest_row_bytes) + dest_x) = srcPixel;
						continue;
					}

					uint8_t srcR, srcG, srcB;
					if (is555) {
						spritectl_555_to_rgb(srcPixel, &srcR, &srcG, &srcB);
					} else {
						spritectl_565_to_rgb(srcPixel, &srcR, &srcG, &srcB);
					}

					uint8_t srcAlpha = 255;
					if (applyAlphaMod) {
						srcAlpha = (uint8_t)((255 * alpha) / 255);
						if (srcAlpha == 0) {
							continue;
						}
					}

					if (dest_bpp == 2) {
						uint16_t* dest_pixel_ptr = (uint16_t*)(dest_row_bytes) + dest_x;
						if (srcAlpha == 255) {
							*dest_pixel_ptr = spritectl_rgb_to_565(srcR, srcG, srcB);
						} else {
							uint8_t dr, dg, db;
							spritectl_565_to_rgb(*dest_pixel_ptr, &dr, &dg, &db);
							uint8_t blendR = (uint8_t)((srcR * srcAlpha + dr * (255 - srcAlpha)) / 255);
							uint8_t blendG = (uint8_t)((srcG * srcAlpha + dg * (255 - srcAlpha)) / 255);
							uint8_t blendB = (uint8_t)((srcB * srcAlpha + db * (255 - srcAlpha)) / 255);
							*dest_pixel_ptr = spritectl_rgb_to_565(blendR, blendG, blendB);
						}
					} else if (dest_bpp == 4) {
						uint32_t* dest_pixel_ptr = (uint32_t*)(dest_row_bytes) + dest_x;
						if (srcAlpha == 255) {
							*dest_pixel_ptr = (255u << 24) | (srcB << 16) | (srcG << 8) | srcR;
						} else {
							uint32_t destPixel = *dest_pixel_ptr;
							uint8_t dr = (uint8_t)(destPixel & 0xFF);
							uint8_t dg = (uint8_t)((destPixel >> 8) & 0xFF);
							uint8_t db = (uint8_t)((destPixel >> 16) & 0xFF);
							uint8_t blendR = (uint8_t)((srcR * srcAlpha + dr * (255 - srcAlpha)) / 255);
							uint8_t blendG = (uint8_t)((srcG * srcAlpha + dg * (255 - srcAlpha)) / 255);
							uint8_t blendB = (uint8_t)((srcB * srcAlpha + db * (255 - srcAlpha)) / 255);
							*dest_pixel_ptr = (255u << 24) | (blendB << 16) | (blendG << 8) | blendR;
						}
					}
				}
			}
		}

		if (!was_locked_fast) {
			if (SDL_MUSTLOCK(sdl_surface)) {
				SDL_UnlockSurface(sdl_surface);
			}
		}

		return 0;
	}

	/* Fallback to old method for sprites without RLE data */
	static int fallback_count = 0;
	if (fallback_count < 3) {
		fprintf(stderr, "[SpriteLib] WARNING: Using fallback rendering for sprite without RLE data (sprite=%p, has_rle=%d)\n",
		        (void*)sprite, sprite->has_rle);
		fallback_count++;
	}

	/* Fallback to old method for sprites without RLE data */
	SDL_Rect dest_rect;
	SDL_Surface* src_surface = NULL;
	int result = -1;

	// CRITICAL: Destination surface must NOT be locked when blitting
	// Unlock before blit, then re-lock after to maintain expected state
	bool was_locked = (dest->locked > 0);
	int saved_lock_count = dest->locked;

	if (was_locked) {
		// Unlock the surface for blitting
		while (dest->locked > 0) {
			SDL_UnlockSurface(dest->surface);
			dest->locked--;
		}
	}

	/* NOTE: We do NOT lock the destination surface here.
	 * SDL_BlitSurface will handle any necessary locking internally.
	 * Locking the destination surface before blitting is incorrect and will cause errors.
	 */

	/* Reuse a grow-only scratch surface instead of creating/destroying a new
	 * SDL surface on every call. This path runs on every glyph draw (font
	 * glyphs are created without RLE data - see spritectl_create_sprite in
	 * TextBackendSDL.cpp), so at UI-heavy-text framerates this was doing
	 * thousands of SDL_CreateRGBSurface/SDL_FreeSurface cycles per second,
	 * which is both slow on its own and fragments the heap - causing a
	 * progressive FPS decay the longer the process ran (steadily slower
	 * mallocs/frees), not just a flat constant overhead. */
	static SDL_Surface* s_scratch_surface = NULL;
	static int s_scratch_w = 0;
	static int s_scratch_h = 0;

	if (!s_scratch_surface || s_scratch_w < sprite->width || s_scratch_h < sprite->height) {
		if (s_scratch_surface) {
			SDL_FreeSurface(s_scratch_surface);
		}
		int new_w = (s_scratch_w > sprite->width) ? s_scratch_w : sprite->width;
		int new_h = (s_scratch_h > sprite->height) ? s_scratch_h : sprite->height;
		s_scratch_surface = SDL_CreateRGBSurface(0, new_w, new_h, 32,
		                                        0xFF, 0xFF00, 0xFF0000, 0xFF000000);
		if (!s_scratch_surface) {
			s_scratch_w = 0;
			s_scratch_h = 0;
			return -1;
		}
		s_scratch_w = new_w;
		s_scratch_h = new_h;
	}
	src_surface = s_scratch_surface;

	/* Use cached RGBA pixels if available, otherwise convert and cache */
	if (sprite->rgba_pixels == NULL && sprite->format != SPRITECTL_FORMAT_RGBA32) {
		/* Convert and cache RGBA pixels for non-RGBA32 formats */
		sprite->rgba_pixels = (uint32_t*)malloc(sprite->width * sprite->height * sizeof(uint32_t));
		if (sprite->rgba_pixels) {
			if (sprite->format == SPRITECTL_FORMAT_RGB565) {
				spritectl_convert_565_to_rgba(sprite->pixels, sprite->rgba_pixels,
				                              sprite->width * sprite->height, 0x0000);
			} else if (sprite->format == SPRITECTL_FORMAT_RGB555) {
				spritectl_convert_555_to_rgba(sprite->pixels, sprite->rgba_pixels,
				                              sprite->width * sprite->height, 0x0000);
			} else {
				/* Unknown format, fill with opaque white */
				for (int i = 0; i < sprite->width * sprite->height; i++) {
					sprite->rgba_pixels[i] = 0xFFFFFFFF;
				}
			}
		}
	}

	/* Determine pixel source */
	const uint32_t* pixel_src;
	if (sprite->format == SPRITECTL_FORMAT_RGBA32) {
		pixel_src = (const uint32_t*)sprite->pixels;
	} else if (sprite->rgba_pixels) {
		pixel_src = sprite->rgba_pixels;
	} else {
		/* Fallback: fill with opaque white */
		static uint32_t* fallback_pixels = NULL;
		static int fallback_size = 0;
		if (!fallback_pixels || fallback_size < sprite->width * sprite->height) {
			free(fallback_pixels);
			fallback_pixels = (uint32_t*)malloc(sprite->width * sprite->height * sizeof(uint32_t));
			fallback_size = sprite->width * sprite->height;
			if (fallback_pixels) {
				for (int i = 0; i < sprite->width * sprite->height; i++) {
					fallback_pixels[i] = 0xFFFFFFFF;
				}
			}
		}
		pixel_src = fallback_pixels;
	}

	/* Copy to scratch surface, row by row since it may be wider than the
	 * sprite (grow-only reuse means src_surface->w/h can exceed sprite size) */
	SDL_LockSurface(src_surface);
	{
		uint8_t* dst_bytes = (uint8_t*)src_surface->pixels;
		for (int row = 0; row < sprite->height; row++) {
			memcpy(dst_bytes + row * src_surface->pitch,
			       pixel_src + row * sprite->width,
			       sprite->width * sizeof(uint32_t));
		}
	}
	SDL_UnlockSurface(src_surface);

	/* Handle alpha blending */
	if (flags & SPRITECTL_BLT_ALPHA) {
		SDL_SetSurfaceAlphaMod(src_surface, alpha);
	}

	/* 关键: 始终启用混合模式以支持 alpha 通道 */
	/* 即使不是 alpha 模式，也需要 BLEND 模式来处理 colorkey 产生的透明像素 */
	if (SDL_SetSurfaceBlendMode(src_surface, SDL_BLENDMODE_BLEND) != 0) {
		fprintf(stderr, "[SpriteLib] SDL_SetSurfaceBlendMode failed: %s\n",
		        SDL_GetError());
	}

	/* Set up destination rectangle */
	dest_rect.x = x;
	dest_rect.y = y;
	dest_rect.w = sprite->width;
	dest_rect.h = sprite->height;

	/* Source rect must be limited to the sprite's actual size - the scratch
	 * surface can be larger than the sprite since it's reused/grow-only. */
	SDL_Rect src_rect;
	src_rect.x = 0;
	src_rect.y = 0;
	src_rect.w = sprite->width;
	src_rect.h = sprite->height;

	/* Blit sprite to destination */
	if (SDL_BlitSurface(src_surface, &src_rect, dest->surface, &dest_rect) != 0) {
		fprintf(stderr, "SpriteLib Backend: SDL_BlitSurface failed: %s\n", SDL_GetError());
		result = -1;
	} else {
		result = 0;
	}

	/* NOTE: src_surface is the reused scratch surface - do not free it here. */

	// Re-lock the surface if it was locked before (to maintain expected state)
	if (was_locked) {
		for (int i = 0; i < saved_lock_count; i++) {
			SDL_LockSurface(dest->surface);
			dest->locked++;
		}
	}

	return result;
}

/* fix: matches the original DirectDraw client's CShadowSprite::BltDarkness /
 * memcpyShadowDarkness (see CShadowSprite.cpp in the old client) - a real
 * "darken what's already there" shadow, not a flat color paint. The old
 * client did this with a precomputed per-channel bitmask + a single shift
 * on the packed 16-bit word (a period-appropriate SIMD-ish trick to avoid
 * bit bleed between channels); doing the shift on each channel separately
 * here is simpler and exactly equivalent. */
static inline uint16_t spritectl_darken_565(uint16_t pixel, int darkBits) {
	uint16_t r = (pixel >> 11) & 0x1F;
	uint16_t g = (pixel >> 5) & 0x3F;
	uint16_t b = pixel & 0x1F;
	r >>= darkBits;
	g >>= darkBits;
	b >>= darkBits;
	return (uint16_t)((r << 11) | (g << 5) | b);
}

int spritectl_blt_shadow_darken(spritectl_surface_t dest, int x, int y,
                                spritectl_sprite_t sprite, int darkBits) {
	if (!dest || !sprite || !sprite->pixels) {
		return -1;
	}
	if (sprite->format != SPRITECTL_FORMAT_RGB565) {
		return -1;
	}

	SDL_Surface* sdl_surface = dest->surface;
	if (!sdl_surface || sdl_surface->format->BytesPerPixel != 2) {
		return -1;
	}

	bool was_locked_fast = (dest->locked > 0);
	if (!was_locked_fast && SDL_MUSTLOCK(sdl_surface)) {
		SDL_LockSurface(sdl_surface);
	}

	int dest_width = sdl_surface->w;
	int dest_height = sdl_surface->h;
	int dest_pitch = sdl_surface->pitch;

	int clip_left = (x < 0) ? -x : 0;
	int clip_top = (y < 0) ? -y : 0;
	int clip_right = (x + sprite->width > dest_width) ? dest_width - x : sprite->width;
	int clip_bottom = (y + sprite->height > dest_height) ? dest_height - y : sprite->height;

	if (clip_left < clip_right && clip_top < clip_bottom) {
		const uint16_t* src_pixels = (const uint16_t*)sprite->pixels;
		uint8_t* dest_bytes = (uint8_t*)sdl_surface->pixels;

		for (int sy = clip_top; sy < clip_bottom; sy++) {
			const uint16_t* src_row = src_pixels + sy * sprite->width;
			uint16_t* dest_row = (uint16_t*)(dest_bytes + (y + sy) * dest_pitch);

			for (int sx = clip_left; sx < clip_right; sx++) {
				if (src_row[sx] == 0x0000) {
					continue; /* transparent - leave ground pixel alone */
				}
				int dest_x = x + sx;
				dest_row[dest_x] = spritectl_darken_565(dest_row[dest_x], darkBits);
			}
		}
	}

	if (!was_locked_fast && SDL_MUSTLOCK(sdl_surface)) {
		SDL_UnlockSurface(sdl_surface);
	}

	return 0;
}

int spritectl_blt_sprite_scaled(spritectl_surface_t dest, int x, int y,
                                spritectl_sprite_t sprite, int scale, int flags) {
	int scaled_width, scaled_height;

	if (!dest || !sprite) {
		return -1;
	}

	/* Calculate scaled dimensions (scale is in 1/256 units) */
	scaled_width = (sprite->width * scale) / 256;
	scaled_height = (sprite->height * scale) / 256;

	if (scaled_width <= 0 || scaled_height <= 0) {
		return 0;  /* Too small to see */
	}

	/* Same anti-pattern as the old glyph blit: this used to create+destroy
	 * two fresh 32-bit SDL_CreateRGBSurface surfaces on every single call
	 * (half-size world image objects go through here every frame), which
	 * is heap-churn on top of a slow converting SDL_BlitSurface into the
	 * 16-bit backbuffer. Reuse grow-only scratch surfaces instead, and
	 * blit the final (always-32-bit) scaled surface into dest with a
	 * direct pixel copy instead of SDL_BlitSurface. */
	static SDL_Surface* s_src_scratch = NULL;
	static int s_src_scratch_w = 0;
	static int s_src_scratch_h = 0;

	if (!s_src_scratch || s_src_scratch_w < sprite->width || s_src_scratch_h < sprite->height) {
		if (s_src_scratch) {
			SDL_FreeSurface(s_src_scratch);
		}
		int new_w = (s_src_scratch_w > sprite->width) ? s_src_scratch_w : sprite->width;
		int new_h = (s_src_scratch_h > sprite->height) ? s_src_scratch_h : sprite->height;
		s_src_scratch = SDL_CreateRGBSurface(0, new_w, new_h, 32,
		                                    0xFF, 0xFF00, 0xFF0000, 0xFF000000);
		if (!s_src_scratch) {
			s_src_scratch_w = 0;
			s_src_scratch_h = 0;
			return -1;
		}
		s_src_scratch_w = new_w;
		s_src_scratch_h = new_h;
	}
	SDL_Surface* src_surface = s_src_scratch;

	/* Use cached RGBA pixels if available, otherwise convert and cache */
	if (sprite->rgba_pixels == NULL && sprite->format != SPRITECTL_FORMAT_RGBA32) {
		/* Convert and cache RGBA pixels for non-RGBA32 formats */
		sprite->rgba_pixels = (uint32_t*)malloc(sprite->width * sprite->height * sizeof(uint32_t));
		if (sprite->rgba_pixels) {
			if (sprite->format == SPRITECTL_FORMAT_RGB565) {
				spritectl_convert_565_to_rgba(sprite->pixels, sprite->rgba_pixels,
				                              sprite->width * sprite->height, 0x0000);
			} else if (sprite->format == SPRITECTL_FORMAT_RGB555) {
				spritectl_convert_555_to_rgba(sprite->pixels, sprite->rgba_pixels,
				                              sprite->width * sprite->height, 0x0000);
			} else {
				/* Unknown format, fill with opaque white */
				for (int i = 0; i < sprite->width * sprite->height; i++) {
					sprite->rgba_pixels[i] = 0xFFFFFFFF;
				}
			}
		}
	}

	/* Determine pixel source */
	const uint32_t* pixel_src;
	if (sprite->format == SPRITECTL_FORMAT_RGBA32) {
		pixel_src = (const uint32_t*)sprite->pixels;
	} else if (sprite->rgba_pixels) {
		pixel_src = sprite->rgba_pixels;
	} else {
		/* Fallback: fill with opaque white */
		static uint32_t* fallback_pixels = NULL;
		static int fallback_size = 0;
		if (!fallback_pixels || fallback_size < sprite->width * sprite->height) {
			free(fallback_pixels);
			fallback_pixels = (uint32_t*)malloc(sprite->width * sprite->height * sizeof(uint32_t));
			fallback_size = sprite->width * sprite->height;
			if (fallback_pixels) {
				for (int i = 0; i < sprite->width * sprite->height; i++) {
					fallback_pixels[i] = 0xFFFFFFFF;
				}
			}
		}
		pixel_src = fallback_pixels;
	}

	/* Copy to scratch surface, row by row since it may be wider/taller than
	 * the sprite (grow-only reuse means src_surface->w/h can exceed sprite
	 * size, so pitch may not equal sprite->width * 4). */
	SDL_LockSurface(src_surface);
	{
		uint8_t* dst_bytes = (uint8_t*)src_surface->pixels;
		for (int row = 0; row < sprite->height; row++) {
			memcpy(dst_bytes + row * src_surface->pitch,
			       pixel_src + row * sprite->width,
			       sprite->width * sizeof(uint32_t));
		}
	}
	SDL_UnlockSurface(src_surface);

	/* Reuse a grow-only scratch surface for the scaled output too. */
	static SDL_Surface* s_scaled_scratch = NULL;
	static int s_scaled_scratch_w = 0;
	static int s_scaled_scratch_h = 0;

	if (!s_scaled_scratch || s_scaled_scratch_w < scaled_width || s_scaled_scratch_h < scaled_height) {
		if (s_scaled_scratch) {
			SDL_FreeSurface(s_scaled_scratch);
		}
		int new_w = (s_scaled_scratch_w > scaled_width) ? s_scaled_scratch_w : scaled_width;
		int new_h = (s_scaled_scratch_h > scaled_height) ? s_scaled_scratch_h : scaled_height;
		s_scaled_scratch = SDL_CreateRGBSurface(0, new_w, new_h, 32,
		                                       0xFF, 0xFF00, 0xFF0000, 0xFF000000);
		if (!s_scaled_scratch) {
			s_scaled_scratch_w = 0;
			s_scaled_scratch_h = 0;
			return -1;
		}
		s_scaled_scratch_w = new_w;
		s_scaled_scratch_h = new_h;
	}
	SDL_Surface* scaled_surface = s_scaled_scratch;

	/* Scale the surface. Explicit src/dest rects since both scratch
	 * surfaces may be larger than sprite/scaled_width,height (grow-only). */
	SDL_Rect stretch_src_rect = { 0, 0, sprite->width, sprite->height };
	SDL_Rect stretch_dest_rect = { 0, 0, scaled_width, scaled_height };
	SDL_SoftStretch(src_surface, &stretch_src_rect, scaled_surface, &stretch_dest_rect);

	/* Blit scaled sprite directly into dest's pixels instead of going
	 * through SDL_BlitSurface - same fix as the RGBA32 glyph fast path
	 * above: scaled_surface is always 32-bit RGBA, but dest is typically
	 * the 16-bit RGB565 backbuffer, so SDL_BlitSurface would have to take
	 * its slow generic converting path on every world image object. */
	SDL_Surface* sdl_dest = dest->surface;
	if (!sdl_dest) {
		return -1;
	}

	bool was_locked_fast = (dest->locked > 0);
	if (!was_locked_fast && SDL_MUSTLOCK(sdl_dest)) {
		SDL_LockSurface(sdl_dest);
	}

	int dest_width = sdl_dest->w;
	int dest_height = sdl_dest->h;
	int dest_pitch = sdl_dest->pitch;
	int dest_bpp = sdl_dest->format->BytesPerPixel;

	int clip_left = (x < 0) ? -x : 0;
	int clip_top = (y < 0) ? -y : 0;
	int clip_right = (x + scaled_width > dest_width) ? dest_width - x : scaled_width;
	int clip_bottom = (y + scaled_height > dest_height) ? dest_height - y : scaled_height;

	if (clip_left < clip_right && clip_top < clip_bottom) {
		SDL_LockSurface(scaled_surface);
		const uint8_t* scaled_bytes = (const uint8_t*)scaled_surface->pixels;
		int scaled_pitch = scaled_surface->pitch;
		uint8_t* dest_bytes = (uint8_t*)sdl_dest->pixels;

		for (int sy = clip_top; sy < clip_bottom; sy++) {
			const uint32_t* src_row = (const uint32_t*)(scaled_bytes + sy * scaled_pitch);
			uint8_t* dest_row_bytes = dest_bytes + (y + sy) * dest_pitch;

			for (int sx = clip_left; sx < clip_right; sx++) {
				uint32_t srcPixel = src_row[sx];
				uint8_t srcAlpha = (uint8_t)((srcPixel >> 24) & 0xFF);
				if (srcAlpha == 0) {
					continue;
				}
				uint8_t srcR = (uint8_t)(srcPixel & 0xFF);
				uint8_t srcG = (uint8_t)((srcPixel >> 8) & 0xFF);
				uint8_t srcB = (uint8_t)((srcPixel >> 16) & 0xFF);

				int dest_x = x + sx;

				if (dest_bpp == 2) {
					uint16_t* dest_pixel_ptr = (uint16_t*)(dest_row_bytes) + dest_x;
					if (srcAlpha == 255) {
						*dest_pixel_ptr = spritectl_rgb_to_565(srcR, srcG, srcB);
					} else {
						uint8_t dr, dg, db;
						spritectl_565_to_rgb(*dest_pixel_ptr, &dr, &dg, &db);
						uint8_t blendR = (uint8_t)((srcR * srcAlpha + dr * (255 - srcAlpha)) / 255);
						uint8_t blendG = (uint8_t)((srcG * srcAlpha + dg * (255 - srcAlpha)) / 255);
						uint8_t blendB = (uint8_t)((srcB * srcAlpha + db * (255 - srcAlpha)) / 255);
						*dest_pixel_ptr = spritectl_rgb_to_565(blendR, blendG, blendB);
					}
				} else if (dest_bpp == 4) {
					uint32_t* dest_pixel_ptr = (uint32_t*)(dest_row_bytes) + dest_x;
					if (srcAlpha == 255) {
						*dest_pixel_ptr = (255u << 24) | (srcB << 16) | (srcG << 8) | srcR;
					} else {
						uint32_t destPixel = *dest_pixel_ptr;
						uint8_t dr = (uint8_t)(destPixel & 0xFF);
						uint8_t dg = (uint8_t)((destPixel >> 8) & 0xFF);
						uint8_t db = (uint8_t)((destPixel >> 16) & 0xFF);
						uint8_t blendR = (uint8_t)((srcR * srcAlpha + dr * (255 - srcAlpha)) / 255);
						uint8_t blendG = (uint8_t)((srcG * srcAlpha + dg * (255 - srcAlpha)) / 255);
						uint8_t blendB = (uint8_t)((srcB * srcAlpha + db * (255 - srcAlpha)) / 255);
						*dest_pixel_ptr = (255u << 24) | (blendB << 16) | (blendG << 8) | blendR;
					}
				}
			}
		}

		SDL_UnlockSurface(scaled_surface);
	}

	if (!was_locked_fast && SDL_MUSTLOCK(sdl_dest)) {
		SDL_UnlockSurface(sdl_dest);
	}

	return 0;
}

int spritectl_blt_surface(spritectl_surface_t dest,
                          const RECT* dest_rect,
                          spritectl_surface_t src,
                          const RECT* src_rect) {
	SDL_Rect sdl_dest_rect, sdl_src_rect;

	if (!dest || !src) {
		return -1;
	}

	/* Convert RECT to SDL_Rect */
	if (dest_rect) {
		sdl_dest_rect.x = dest_rect->left;
		sdl_dest_rect.y = dest_rect->top;
		sdl_dest_rect.w = dest_rect->right - dest_rect->left;
		sdl_dest_rect.h = dest_rect->bottom - dest_rect->top;
	} else {
		sdl_dest_rect.x = 0;
		sdl_dest_rect.y = 0;
		sdl_dest_rect.w = dest->width;
		sdl_dest_rect.h = dest->height;
	}

	if (src_rect) {
		sdl_src_rect.x = src_rect->left;
		sdl_src_rect.y = src_rect->top;
		sdl_src_rect.w = src_rect->right - src_rect->left;
		sdl_src_rect.h = src_rect->bottom - src_rect->top;
	} else {
		sdl_src_rect.x = 0;
		sdl_src_rect.y = 0;
		sdl_src_rect.w = src->width;
		sdl_src_rect.h = src->height;
	}

	/* Blit surface to surface */
	if (SDL_BlitSurface(src->surface, &sdl_src_rect, dest->surface, &sdl_dest_rect) != 0) {
		fprintf(stderr, "SpriteLib Backend: SDL_BlitSurface failed: %s\n", SDL_GetError());
		return -1;
	}

	return 0;
}

/* ============================================================================
 * Sprite Pack Functions
 * ============================================================================ */

int spritectl_load_pack(const char* filename, spritectl_pack_t* pack) {
	FILE* file;
	uint16_t count;
	spritectl_pack_t pack_ptr = NULL;
	int result = -1;

	if (!filename || !pack) {
		return -1;
	}

	/* Open pack file */
	file = fopen(filename, "rb");
	if (!file) {
		fprintf(stderr, "SpriteLib Backend: Failed to open pack file: %s\n", filename);
		return -2;
	}

	/* Read sprite count (2 bytes, little-endian) */
	if (fread(&count, 2, 1, file) != 1) {
		fclose(file);
		return -4;
	}

	/* Allocate pack structure */
	pack_ptr = (spritectl_pack_t)malloc(sizeof(struct spritectl_pack_s));
	if (!pack_ptr) {
		fclose(file);
		return -3;
	}

	/* Initialize pack structure */
	pack_ptr->filename = strdup(filename);
	pack_ptr->file = file;
	pack_ptr->sprite_count = count;
	pack_ptr->lazy_loading = 0;  /* Full load */
	pack_ptr->ref_count = 1;

	/* Allocate sprite array */
	if (count > 0) {
		pack_ptr->sprites = (spritectl_sprite_t*)calloc(count, sizeof(spritectl_sprite_t));
		if (!pack_ptr->sprites) {
			free(pack_ptr->filename);
			free(pack_ptr);
			fclose(file);
			return -3;
		}

		pack_ptr->loaded_flags = (int*)calloc(count, sizeof(int));
		if (!pack_ptr->loaded_flags) {
			free(pack_ptr->sprites);
			free(pack_ptr->filename);
			free(pack_ptr);
			fclose(file);
			return -3;
		}

		pack_ptr->file_offsets = NULL;  /* Not needed for full load */
	} else {
		pack_ptr->sprites = NULL;
		pack_ptr->loaded_flags = NULL;
		pack_ptr->file_offsets = NULL;
	}

	/* Load each sprite */
	uint16_t colorkey = 0xFFFF;  /* Default color key */
	for (uint16_t i = 0; i < count; i++) {
		int load_result = spritectl_load_sprite_from_file(file, &pack_ptr->sprites[i], colorkey);
		if (load_result != 0) {
			pack_ptr->sprites[i] = SPRITECTL_INVALID_SPRITE;
			pack_ptr->loaded_flags[i] = 0;
		} else {
			pack_ptr->loaded_flags[i] = 1;
		}
	}

	fclose(file);
	pack_ptr->file = NULL;

	*pack = pack_ptr;
	return 0;
}

void spritectl_free_pack(spritectl_pack_t pack) {
	if (!pack) {
		return;
	}

	/* Free all sprites */
	if (pack->sprites) {
		for (int i = 0; i < pack->sprite_count; i++) {
			if (pack->sprites[i] != SPRITECTL_INVALID_SPRITE) {
				spritectl_destroy_sprite(pack->sprites[i]);
			}
		}
		free(pack->sprites);
		pack->sprites = NULL;
	}

	/* Free other resources */
	if (pack->filename) {
		free(pack->filename);
		pack->filename = NULL;
	}
	if (pack->loaded_flags) {
		free(pack->loaded_flags);
		pack->loaded_flags = NULL;
	}
	if (pack->file_offsets) {
		free(pack->file_offsets);
		pack->file_offsets = NULL;
	}
	if (pack->file) {
		fclose(pack->file);
		pack->file = NULL;
	}

	free(pack);
}

int spritectl_get_sprite_from_pack(spritectl_pack_t pack, int index,
                                   spritectl_sprite_t* sprite) {
	if (!pack || !sprite) {
		return -1;
	}

	/* Check index bounds */
	if (index < 0 || index >= pack->sprite_count) {
		return -2;
	}

	/* Return sprite (or invalid handle if not loaded) */
	*sprite = pack->sprites[index];
	return 0;
}

int spritectl_get_pack_count(spritectl_pack_t pack) {
	if (!pack) {
		return 0;
	}
	return pack->sprite_count;
}

/* ============================================================================
 * Pixel Conversion Functions
 * ============================================================================ */

void spritectl_convert_565_to_555(const uint16_t* src, uint16_t* dest, int pixel_count) {
	int i;
	for (i = 0; i < pixel_count; i++) {
		uint16_t pixel = src[i];
		/* Convert 565 to 555: drop the low bit of green */
		dest[i] = ((pixel & 0xFFC0) >> 1) | (pixel & 0x001F);
	}
}

void spritectl_convert_555_to_565(const uint16_t* src, uint16_t* dest, int pixel_count) {
	int i;
	for (i = 0; i < pixel_count; i++) {
		uint16_t pixel = src[i];
		/* Convert 555 to 565: insert a zero bit for green's low bit */
		dest[i] = ((pixel & 0x7FE0) << 1) | (pixel & 0x001F);
	}
}

void spritectl_convert_565_to_rgba(const uint16_t* src, uint32_t* dest,
                                   int pixel_count, uint16_t colorkey) {
	int i;
	for (i = 0; i < pixel_count; i++) {
		uint16_t pixel = src[i];
		uint8_t r, g, b, a;

		if (pixel == colorkey) {
			/* Transparent pixel */
			a = 0;
		} else {
			a = 255;
		}

		spritectl_565_to_rgb(pixel, &r, &g, &b);
		dest[i] = spritectl_rgb_to_rgba(r, g, b, a);
	}
}

void spritectl_convert_555_to_rgba(const uint16_t* src, uint32_t* dest,
                                   int pixel_count, uint16_t colorkey) {
	int i;
	for (i = 0; i < pixel_count; i++) {
		uint16_t pixel = src[i];
		uint8_t r, g, b, a;

		if (pixel == colorkey) {
			/* Transparent pixel */
			a = 0;
		} else {
			a = 255;
		}

		spritectl_555_to_rgb(pixel, &r, &g, &b);
		dest[i] = spritectl_rgb_to_rgba(r, g, b, a);
	}
}

/* ============================================================================
 * RLE Sprite Decoding
 * ============================================================================ */

/**
 * Decode RLE-compressed sprite data to RGB565 pixels
 *
 * RLE Format (per scanline):
 *   - Length (2 bytes, uint16_t) - number of WORDs in RLE data
 *   - RLE Data (Length * 2 bytes):
 *     - Count (1 WORD) - number of segments
 *     - For each segment:
 *       - Trans Count (1 WORD) - transparent pixel count
 *       - Color Count (1 WORD) - color pixel count
 *       - Pixels (Color Count WORDs) - RGB565 pixel data
 */
int spritectl_decode_rle_data(const uint16_t* rle_data, int rle_len,
                               uint16_t* pixels_out, int width) {
	int rle_index = 0;
	int x = 0;

	/* Read segment count */
	if (rle_len == 0) {
		return 0;  /* Empty scanline */
	}

	int count = rle_data[rle_index++];
	rle_index = 1;

	/* Process each segment */
	for (int seg = 0; seg < count && x < width; seg++) {
		if (rle_index >= rle_len) break;

		/* Read transparent and color counts */
		int trans_count = rle_data[rle_index++];
		int color_count = rle_data[rle_index++];

		/* Skip transparent pixels (leave as 0) */
		x += trans_count;

		/* Copy color pixels */
		for (int c = 0; c < color_count && x < width; c++) {
			if (rle_index >= rle_len) break;
			pixels_out[x] = rle_data[rle_index++];
			x++;
		}
	}

	return 0;
}

int spritectl_decode_rle_sprite(const uint8_t* compressed, size_t compressed_size,
                                int width, int height,
                                uint16_t* pixels_out, size_t pixels_size) {
	FILE* file;
	uint16_t* scanline_lengths = NULL;
	uint8_t** scanlines = NULL;
	int result = -1;

	/* For now, we'll load from FILE pointer instead of raw buffer */
	/* This is a simpler implementation that matches the original */
	fprintf(stderr, "SpriteLib Backend: Use spritectl_load_sprite_from_file instead\n");
	return -1;
}

/**
 * Load sprite from file (RLE encoded)
 * File format:
 *   - Width (2 bytes, uint16_t)
 *   - Height (2 bytes, uint16_t)
 *   - For each scanline:
 *     - Length (2 bytes, uint16_t)
 *     - RLE data (Length * 2 bytes)
 */
int spritectl_load_sprite_from_file(FILE* file, spritectl_sprite_t* sprite_out,
                                    uint16_t colorkey) {
	uint16_t width = 0, height = 0;
	uint16_t* scanline_lengths = NULL;
	uint16_t** scanline_rle = NULL;
	spritectl_sprite_t sprite = NULL;
	size_t pixel_count = 0;
	uint16_t* pixels = NULL;
	int result = -1;

	if (!file || !sprite_out) {
		return -1;
	}

	/* Read width and height */
	if (fread(&width, 2, 1, file) != 1) goto cleanup;
	if (fread(&height, 2, 1, file) != 1) goto cleanup;

	/* Handle empty sprite */
	if (width == 0 || height == 0) {
		*sprite_out = SPRITECTL_INVALID_SPRITE;
		return 0;
	}

	pixel_count = width * height;

	/* Allocate scanline length array */
	scanline_lengths = (uint16_t*)calloc(height, sizeof(uint16_t));
	if (!scanline_lengths) goto cleanup;

	/* Allocate RLE data array */
	scanline_rle = (uint16_t**)calloc(height, sizeof(uint16_t*));
	if (!scanline_rle) goto cleanup;

	/* Read each scanline */
	for (int y = 0; y < height; y++) {
		/* Read scanline data length */
		if (fread(&scanline_lengths[y], 2, 1, file) != 1) goto cleanup;

		if (scanline_lengths[y] > 0) {
			/* Allocate and read RLE data */
			scanline_rle[y] = (uint16_t*)malloc(scanline_lengths[y] * sizeof(uint16_t));
			if (!scanline_rle[y]) goto cleanup;

			if (fread(scanline_rle[y], 2, scanline_lengths[y], file) != scanline_lengths[y]) {
				free(scanline_rle[y]);
				scanline_rle[y] = NULL;
				goto cleanup;
			}
		}
	}

	/* Decode RLE to raw pixels */
	pixels = (uint16_t*)calloc(pixel_count, sizeof(uint16_t));
	if (!pixels) goto cleanup;

	/* Decode each scanline */
	for (int y = 0; y < height; y++) {
		uint16_t* row = pixels + (y * width);

		if (scanline_rle[y] && scanline_lengths[y] > 0) {
			/* Decode RLE data */
			int rle_index = 0;
			int x = 0;

			/* Read segment count */
			int count = scanline_rle[y][rle_index++];

			/* Process each segment */
			for (int seg = 0; seg < count && x < width; seg++) {
				int trans_count = scanline_rle[y][rle_index++];
				int color_count = scanline_rle[y][rle_index++];

				/* Skip transparent pixels (keep as 0) */
				x += trans_count;

				/* Copy color pixels */
				for (int c = 0; c < color_count && x < width; c++) {
					row[x] = scanline_rle[y][rle_index++];
					x++;
				}
			}
		}
	}

	/* Create sprite structure */
	sprite = spritectl_create_sprite(width, height, SPRITECTL_FORMAT_RGB565,
	                                  pixels, pixel_count * sizeof(uint16_t));
	if (!sprite) {
		free(pixels);
		goto cleanup;
	}

	free(pixels);  /* Sprite copied the data */

	/* Preserve RLE data for correct transparency rendering */
	sprite->scanline_rle = scanline_rle;
	sprite->scanline_lens = scanline_lengths;
	sprite->has_rle = 1;
	scanline_rle = NULL;  /* Don't free in cleanup */
	scanline_lengths = NULL;

	*sprite_out = sprite;
	result = 0;

cleanup:
	if (scanline_rle) {
		for (int y = 0; y < height; y++) {
			if (scanline_rle[y]) {
				free(scanline_rle[y]);
			}
		}
		free(scanline_rle);
	}
	if (scanline_lengths) {
		free(scanline_lengths);
	}

	return result;
}

/* ============================================================================
 * Present Surface to Renderer
 * ============================================================================ */

int spritectl_present_surface(spritectl_surface_t surface, void* renderer_ptr) {
	if (!surface || !renderer_ptr) {
		return -1;
	}

	SDL_Renderer* renderer = (SDL_Renderer*)renderer_ptr;

	/* Get SDL surface from backend surface */
	SDL_Surface* sdl_surface = surface->surface;
	if (!sdl_surface) {
		fprintf(stderr, "SpriteLib Backend: Surface has no SDL surface\n");
		return -1;
	}

	/* Reuse one streaming texture across frames instead of creating and
	 * destroying a brand new GPU texture every single present call - that
	 * was the main reason this build's FPS was much lower than the legacy
	 * DirectDraw client's even with vsync off: texture create/destroy is
	 * one of the more expensive things you can do per-frame on a GPU, and
	 * this ran on every present. Recreate only if the surface's size or
	 * pixel format actually changes (rare - basically only on a mode/
	 * resolution switch). */
	static SDL_Texture* s_presentTexture = NULL;
	static SDL_Renderer* s_presentRenderer = NULL;
	static int s_presentW = 0, s_presentH = 0;
	static Uint32 s_presentFormat = 0;

	if (s_presentTexture == NULL ||
	    s_presentRenderer != renderer ||
	    s_presentW != sdl_surface->w ||
	    s_presentH != sdl_surface->h ||
	    s_presentFormat != sdl_surface->format->format) {
		if (s_presentTexture != NULL) {
			SDL_DestroyTexture(s_presentTexture);
		}
		s_presentTexture = SDL_CreateTexture(renderer, sdl_surface->format->format,
		                                      SDL_TEXTUREACCESS_STREAMING,
		                                      sdl_surface->w, sdl_surface->h);
		if (!s_presentTexture) {
			fprintf(stderr, "SpriteLib Backend: Failed to create texture: %s\n", SDL_GetError());
			s_presentRenderer = NULL;
			return -1;
		}
		s_presentRenderer = renderer;
		s_presentW = sdl_surface->w;
		s_presentH = sdl_surface->h;
		s_presentFormat = sdl_surface->format->format;
	}
	SDL_Texture* texture = s_presentTexture;

	/* Software gamma/brightness (see CSDLGraphics::ApplyGammaToBuffer) -
	 * applied once here to the whole backbuffer right before upload,
	 * mirroring how the old DirectDraw client's hardware gamma ramp acted
	 * on the final composited frame rather than per-sprite. No-op (single
	 * bool check) when the user hasn't touched the gamma slider and no
	 * screen-tint event is active. */
	if (!CSDLGraphics::IsGammaNeutral() && sdl_surface->format->BytesPerPixel == 2) {
		CSDLGraphics::ApplyGammaToBuffer((uint16_t*)sdl_surface->pixels,
		                                  sdl_surface->w, sdl_surface->h, sdl_surface->pitch);
	}

	SDL_UpdateTexture(texture, NULL, sdl_surface->pixels, sdl_surface->pitch);

	/* Startup fade-in: the original client had no coded fade here either,
	 * but a hard instant pop from black to the title art reads as broken
	 * next to everything else being smooth - ramp alpha 0->255 over the
	 * first ~500ms of real time (not frame count - with vsync off this can
	 * run at 200+ fps, where a frame-counted ramp finishes in under 150ms
	 * and barely reads as a fade at all). */
	{
		static Uint32 s_fadeStartMs = 0;
		static bool s_fadeDone = false;
		const Uint32 FADE_MS = 500;
		if (!s_fadeDone) {
			if (s_fadeStartMs == 0) {
				s_fadeStartMs = SDL_GetTicks();
			}
			Uint32 elapsed = SDL_GetTicks() - s_fadeStartMs;
			if (elapsed >= FADE_MS) {
				s_fadeDone = true;
				// texture is now cached/reused across frames (see above) -
				// reset it back to normal opaque rendering once, or every
				// frame from here on would stay stuck at the last partial
				// fade alpha/blend mode this ever set.
				SDL_SetTextureAlphaMod(texture, 255);
				SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_NONE);
			} else {
				Uint8 fadeAlpha = (Uint8)(255 * elapsed / FADE_MS);
				SDL_SetTextureAlphaMod(texture, fadeAlpha);
				SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
			}
		}
	}

	// DEBUG: Check texture format
	static int texture_debug_count = 0;
	if (texture_debug_count < 3) {
		Uint32 format;
		if (SDL_QueryTexture(texture, &format, NULL, NULL, NULL) == 0) {
			const char* format_name = SDL_GetPixelFormatName(format);
		SPRITELIB_SDL_LOG("Texture created: surface_format=%s, texture_format=%s\n",
				SDL_GetPixelFormatName(sdl_surface->format->format), format_name);
			texture_debug_count++;
		}
	}

	/* Render texture to screen */
	SDL_Rect dest_rect;
	dest_rect.x = 0;
	dest_rect.y = 0;
	dest_rect.w = sdl_surface->w;
	dest_rect.h = sdl_surface->h;

	if (SDL_RenderCopy(renderer, texture, NULL, &dest_rect) != 0) {
		fprintf(stderr, "SpriteLib Backend: Failed to render texture: %s\n", SDL_GetError());
		return -1;
	}

	/* texture is cached across frames (s_presentTexture) - do not destroy it here */

	return 0;
}
