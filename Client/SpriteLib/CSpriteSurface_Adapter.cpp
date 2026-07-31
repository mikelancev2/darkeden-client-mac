/*-----------------------------------------------------------------------------

	CSpriteSurface_Adapter.cpp

	SDL2 backend adapter implementation for CSpriteSurface.
	This file contains all BltSprite* methods implemented using SpriteLibBackend.

	NOTE: This file is included by CSpriteSurface_SDL.cpp
	      Do not compile separately.

	2025.01.14

-----------------------------------------------------------------------------*/

/* No #ifdef SPRITELIB_BACKEND_SDL here - included by SDL-only file */

#include "client_PCH.h"
#include "CSprite.h"
#include "CAlphaSprite.h"
#include "CIndexSprite.h"
#include "CShadowSprite.h"
#include "CSpriteSurface.h"
#include "CFilter.h"

#include "SpriteLibBackend.h"
#include "DebugLog.h"

/* ============================================================================
 * Debug Configuration
 * ============================================================================ */

// Enable detailed debug logging for Sprite adapter
#ifndef SPRITE_ADAPTER_DEBUG
#define SPRITE_ADAPTER_DEBUG 0
#endif

// Enable tracking of backend sprite lifecycle
#ifndef SPRITE_ADAPTER_DEBUG_LIFECYCLE
#define SPRITE_ADAPTER_DEBUG_LIFECYCLE 0
#endif

#if SPRITE_ADAPTER_DEBUG
#define SA_DEBUG(fmt, ...) \
	fprintf(stderr, "[SpriteAdapter] %s:%d: " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define SA_DEBUG(fmt, ...) do {} while(0)
#endif

#if SPRITE_ADAPTER_DEBUG_LIFECYCLE
#define SA_DEBUG_LIFECYCLE(fmt, ...) \
	fprintf(stderr, "[SpriteAdapter LIFECYCLE] %s:%d: " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define SA_DEBUG_LIFECYCLE(fmt, ...) do {} while(0)
#endif

/* Error logging - always enabled */
#define SA_DEBUG_ERROR(fmt, ...) \
	fprintf(stderr, "[SpriteAdapter ERROR] %s:%d: " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)

/* ============================================================================
 * Helper Functions
 * ============================================================================ */

/**
 * Get or create backend sprite from CSprite
 * Handles lazy creation and synchronization
 */
static spritectl_sprite_t get_backend_sprite(CSprite* pSprite)
{
	if (!pSprite || !pSprite->IsInit()) {
		return SPRITECTL_INVALID_SPRITE;
	}

	/* Lazy creation: create backend sprite if doesn't exist */
	if (pSprite->GetBackendSprite() == SPRITECTL_INVALID_SPRITE) {
		WORD width = pSprite->GetWidth();
		WORD height = pSprite->GetHeight();

		/* Create backend sprite with RLE data for correct transparency */
		spritectl_sprite_t new_sprite = spritectl_create_sprite_rle(width, height);
		if (!new_sprite) {
			return SPRITECTL_INVALID_SPRITE;
		}

		/* Copy RLE data from CSprite to backend sprite */
		for (WORD y = 0; y < height; y++) {
			WORD* src_line = pSprite->GetPixelLine(y);
			if (!src_line) {
				SA_DEBUG_ERROR("get_backend_sprite: Invalid scanline at y=%d", y);
				spritectl_destroy_sprite(new_sprite);
				return SPRITECTL_INVALID_SPRITE;
			}

			/* Get RLE data size with bounds checking */
			WORD* pSrc = src_line;
			WORD* pSrcStart = src_line;  /* Remember start for validation */

			int count = *pSrc++;  /* Number of runs */

			/* Validate count to prevent infinite loops or memory corruption */
			if (count < 0 || count > 16384) {  /* Arbitrary reasonable limit */
				SA_DEBUG_ERROR("get_backend_sprite: Invalid RLE count=%d at y=%d", count, y);
				spritectl_destroy_sprite(new_sprite);
				return SPRITECTL_INVALID_SPRITE;
			}

			/* Calculate total RLE data size (count + runs data) with bounds checking */
			int rle_size = 1;  /* count byte */
			if (count > 0) {
				for (int j = 0; j < count; j++) {
					/* Check if we have enough data for transCount and colorCount */
					if ((pSrc - pSrcStart) > width * 2) {  /* Safety check */
						SA_DEBUG_ERROR("get_backend_sprite: RLE data exceeds bounds at segment %d, y=%d", j, y);
						spritectl_destroy_sprite(new_sprite);
						return SPRITECTL_INVALID_SPRITE;
					}

					pSrc++;  /* Skip transCount */
					int colorCount = *pSrc++;  /* Color pixels */

					/* Validate colorCount */
					if (colorCount < 0 || colorCount > width) {
						SA_DEBUG_ERROR("get_backend_sprite: Invalid colorCount=%d at segment %d, y=%d", colorCount, j, y);
						spritectl_destroy_sprite(new_sprite);
						return SPRITECTL_INVALID_SPRITE;
					}

					rle_size += 2 + colorCount;  /* trans + color count + pixel data */
					pSrc += colorCount;
				}
			}

			/* Validate final rle_size */
			if (rle_size <= 0 || rle_size > 65535) {
				SA_DEBUG_ERROR("get_backend_sprite: Invalid rle_size=%d at y=%d", rle_size, y);
				spritectl_destroy_sprite(new_sprite);
				return SPRITECTL_INVALID_SPRITE;
			}

			/* Set RLE data using helper function */
			if (rle_size > 1) {  /* Non-empty scanline */
				if (spritectl_sprite_set_scanline_rle(new_sprite, y, src_line, rle_size) != 0) {
					/* Cleanup and fall back */
					spritectl_destroy_sprite(new_sprite);
					return SPRITECTL_INVALID_SPRITE;
				}
			}
		}

		pSprite->SetBackendSprite(new_sprite);
		pSprite->SetBackendDirty(false);
	}
	/* Sync if dirty */
	else if (pSprite->IsBackendDirty()) {
		/* Destroy old sprite and recreate */
		spritectl_destroy_sprite(pSprite->GetBackendSprite());
		pSprite->SetBackendSprite(SPRITECTL_INVALID_SPRITE);

		/* Recreate (will be created on next call) */
		return get_backend_sprite(pSprite);
	}

	return pSprite->GetBackendSprite();
}

/**
 * Get or create backend sprite from CAlphaSprite
 * Handles lazy creation and synchronization
 * NOTE: CAlphaSprite uses RLE compression with alpha channel
 */
static spritectl_sprite_t get_backend_alpha_sprite(CAlphaSprite* pSprite)
{
	if (!pSprite || !pSprite->IsInit()) {
		return SPRITECTL_INVALID_SPRITE;
	}

	/* Lazy creation: create backend sprite if doesn't exist */
	if (pSprite->GetBackendSprite() == SPRITECTL_INVALID_SPRITE) {
		WORD width = pSprite->GetWidth();
		WORD height = pSprite->GetHeight();

		size_t pixel_count = width * height;
		size_t data_size = pixel_count * sizeof(WORD);

		/* Allocate and decompress pixel data */
		WORD* pixels = (WORD*)malloc(data_size);
		if (!pixels) {
			return SPRITECTL_INVALID_SPRITE;
		}

		/* Decompress RLE format to raw pixels */
		/* Initialize with transparent color */
		WORD colorkey = CAlphaSprite::GetColorkey();
		for (size_t i = 0; i < pixel_count; i++) {
			pixels[i] = colorkey;
		}

		/* Decompress each line */
		for (WORD y = 0; y < height; y++) {
			WORD* pPixels = pSprite->GetPixelLine(y);
			WORD* dst_line = pixels + (y * width);

			int count = *pPixels++;  // RLE run count
			int x = 0;

			if (count > 0) {
				for (int i = 0; i < count; i++) {
					int transCount = *pPixels++;   // transparent pixel count
					int colorCount = *pPixels++;   // colored pixel count

					x += transCount;  // skip transparent pixels

					/* Copy colored pixels with alpha */
					for (int j = 0; j < colorCount; j++) {
						WORD alpha2 = *pPixels++;  // alpha value
						WORD color = *pPixels++;   // color value

						if (x < width) {
							/* Store as pre-multiplied alpha or similar format */
							/* For now, just store the color directly */
							dst_line[x] = color;
						}
						x++;
					}
				}
			}
		}

		/* Create backend sprite */
		spritectl_sprite_t new_sprite = spritectl_create_sprite(
			width, height, SPRITECTL_FORMAT_RGB565,
			pixels, data_size);

		free(pixels);
		pSprite->SetBackendSprite(new_sprite);
		pSprite->SetBackendDirty(false);
	}
	/* Sync if dirty */
	else if (pSprite->IsBackendDirty()) {
		/* Destroy old sprite and recreate */
		spritectl_destroy_sprite(pSprite->GetBackendSprite());
		pSprite->SetBackendSprite(SPRITECTL_INVALID_SPRITE);

		/* Recreate (will be created on next call) */
		return get_backend_alpha_sprite(pSprite);
	}

	return pSprite->GetBackendSprite();
}

/**
 * Get or create backend sprite from CShadowSprite
 * Handles lazy creation and synchronization
 */
static spritectl_sprite_t get_backend_shadow_sprite(CShadowSprite* pSprite)
{
	SA_DEBUG_LIFECYCLE("get_backend_shadow_sprite: pSprite=%p, IsInit=%d",
	                   (void*)pSprite, pSprite ? pSprite->IsInit() : 0);

	if (!pSprite || !pSprite->IsInit()) {
		return SPRITECTL_INVALID_SPRITE;
	}

	/* Lazy creation: create backend sprite if doesn't exist */
	if (pSprite->GetBackendSprite() == SPRITECTL_INVALID_SPRITE) {
		WORD width = pSprite->GetWidth();
		WORD height = pSprite->GetHeight();

		size_t pixel_count = width * height;
		size_t data_size = pixel_count * sizeof(WORD);

		SA_DEBUG_LIFECYCLE("get_backend_shadow_sprite: Creating backend sprite, size=%dx%d (%zu pixels, %zu bytes)",
		                   width, height, pixel_count, data_size);

		/* Allocate and decode pixel data */
		WORD* pixels = (WORD*)malloc(data_size);
		if (!pixels) {
			SA_DEBUG_LIFECYCLE("get_backend_shadow_sprite: Failed to allocate pixel buffer");
			return SPRITECTL_INVALID_SPRITE;
		}

		SA_DEBUG_LIFECYCLE("get_backend_shadow_sprite: Allocated temp pixels=%p", (void*)pixels);

		/* fix: CShadowSprite::Blt() only ever writes literal 0x0000 for the
		 * shadow shape itself (see CShadowSprite.cpp - the RLE "skip" runs
		 * leave pixels untouched, the "paint" runs memset to 0).
		 *
		 * CShadowSprite::GetColorkey() looked like the right "background"
		 * marker to pre-fill with (matches the class's own SetColorkey API,
		 * used this way in the old DirectDraw client's SetPixel() encoder),
		 * but confirmed by direct testing: s_Colorkey defaults to 0
		 * (CShadowSprite.cpp:17) and the only SetColorkey(0x001F) call
		 * (MTopView.cpp:1112) is dead - it sits inside a commented-out
		 * `/* ... *\/` block, in BOTH the old and new client. So
		 * GetColorkey() is always 0 at runtime - identical to what Blt()
		 * paints - making shadow and background indistinguishable again,
		 * same bug under a different name. Use a fixed sentinel instead
		 * that's guaranteed to differ from Blt()'s hardcoded 0x0000. */
		const WORD SHADOW_BACKGROUND_SENTINEL = 0xFFFF;
		WORD backgroundMarker = SHADOW_BACKGROUND_SENTINEL;
		for (size_t i = 0; i < pixel_count; i++) {
			pixels[i] = backgroundMarker;
		}
		pSprite->Blt(pixels, width * sizeof(WORD));

		/* Remap into the generic 0x0000-is-transparent convention: this
		 * backend sprite is consumed as a pure shape MASK by
		 * spritectl_blt_shadow_darken (see SpriteLibBackendSDL.cpp) - it
		 * only checks whether each pixel is 0x0000 (skip) or not (darken
		 * the destination there), never reads the actual color, so any
		 * fixed non-zero marker works here. */
		const WORD SHADOW_PRESENT_MARKER = 0x0001;
		for (size_t i = 0; i < pixel_count; i++) {
			pixels[i] = (pixels[i] == backgroundMarker) ? 0x0000 : SHADOW_PRESENT_MARKER;
		}

		/* Create backend sprite */
		spritectl_sprite_t new_sprite = spritectl_create_sprite(
			width, height, SPRITECTL_FORMAT_RGB565,
			pixels, data_size);

		SA_DEBUG_LIFECYCLE("get_backend_shadow_sprite: Created backend sprite=%p from temp pixels=%p",
		                   (void*)new_sprite, (void*)pixels);

		// Free temp pixels AFTER creating the sprite (the sprite copies the data)
		free(pixels);
		pixels = NULL;  // Prevent dangling pointer

		pSprite->SetBackendSprite(new_sprite);
		pSprite->SetBackendDirty(false);

		SA_DEBUG_LIFECYCLE("get_backend_shadow_sprite: Set backend sprite=%p for CShadowSprite=%p",
		                   (void*)new_sprite, (void*)pSprite);
	}
	/* Sync if dirty */
	else if (pSprite->IsBackendDirty()) {
		SA_DEBUG_LIFECYCLE("get_backend_shadow_sprite: Backend dirty, destroying old sprite=%p",
		                   (void*)pSprite->GetBackendSprite());
		/* Destroy old sprite and recreate */
		spritectl_destroy_sprite(pSprite->GetBackendSprite());
		pSprite->SetBackendSprite(SPRITECTL_INVALID_SPRITE);

		/* Recreate (will be created on next call) */
		return get_backend_shadow_sprite(pSprite);
	}
	else {
		SA_DEBUG_LIFECYCLE("get_backend_shadow_sprite: Reusing existing backend sprite=%p",
		                   (void*)pSprite->GetBackendSprite());
	}

	return pSprite->GetBackendSprite();
}

/**
 * Get or create backend sprite from CIndexSprite
 * Handles lazy creation and synchronization
 */
static spritectl_sprite_t get_backend_index_sprite(CIndexSprite* pSprite)
{
	if (!pSprite || !pSprite->IsInit()) {
		return SPRITECTL_INVALID_SPRITE;
	}

	/* fix: CIndexSprite::Blt() resolves each pixel's color from the *current*
	 * static CIndexSprite::s_IndexValue[0]/[1] (set per-draw by
	 * CIndexSprite::SetUsingColorSet(), e.g. per-creature helmet/gear
	 * recolor). This frame object is shared and reused by many different
	 * creatures/players, each with their own colorSet, so a single cached
	 * backend sprite isn't enough: it either shows stale colors (the
	 * original visual bug - one creature's color "locked in" for everyone
	 * reusing the frame) or, if invalidated+rebuilt on every mismatch,
	 * constantly destroys and recreates the sprite as different creatures
	 * take turns using it - malloc/free churn on every draw call, which is
	 * what caused a progressive FPS decay (confirmed via a native heap
	 * snapshot diff: ever-growing "unsigned int[]" allocations from this
	 * exact call site, never freed because a *different* colorSet's use of
	 * the same frame kept tearing down and rebuilding the cache before the
	 * previous one's lifetime was really over). The old DirectDraw client
	 * never had this problem because it resolved colors straight into the
	 * live framebuffer on every draw, with nothing cached to go stale.
	 * Fix: cache one backend sprite per distinct colorSet pair actually
	 * seen for this frame (bounded - see AddBackendSprite), instead of one
	 * single slot that whichever caller drew last exclusively owns.
	 */
	if (pSprite->IsBackendDirty()) {
		/* Pixel data changed - every color variant cached below is stale
		 * (SetBackendDirty(true) already cleared them; this just resets
		 * the flag). */
		pSprite->SetBackendDirty(false);
	}

	int colorSet0 = CIndexSprite::GetUsingColorSet(0);
	int colorSet1 = CIndexSprite::GetUsingColorSet(1);

	spritectl_sprite_t cached = pSprite->FindBackendSprite(colorSet0, colorSet1);
	if (cached != SPRITECTL_INVALID_SPRITE) {
		return cached;
	}

	/* Not cached for this colorSet yet: decode and cache it. */
	WORD width = pSprite->GetWidth();
	WORD height = pSprite->GetHeight();

	size_t pixel_count = width * height;
	size_t data_size = pixel_count * sizeof(WORD);

	/* Allocate and decode pixel data (index sprites are RLE-compressed) */
	WORD* pixels = (WORD*)malloc(data_size);
	if (!pixels) {
		return SPRITECTL_INVALID_SPRITE;
	}
	memset(pixels, 0, data_size);
	pSprite->Blt(pixels, width * sizeof(WORD));

	/* Create backend sprite */
	spritectl_sprite_t new_sprite = spritectl_create_sprite(
		width, height, SPRITECTL_FORMAT_RGB565,
		pixels, data_size);

	free(pixels);
	pSprite->AddBackendSprite(colorSet0, colorSet1, new_sprite);

	return new_sprite;
}

/* ============================================================================
 * BltSprite Methods
 * ============================================================================ */

void CSpriteSurface::BltSprite(POINT* pPoint, CSprite* pSprite) {
	if (!pPoint || !pSprite) {
		return;
	}

	/* Get backend sprite */
	spritectl_sprite_t backend_sprite = get_backend_sprite(pSprite);
	if (!backend_sprite) {
		return;
	}

	/* Blit to backend surface */
	int flags = 0;
	int alpha = 255;
	spritectl_blt_sprite(m_backend_surface, pPoint->x, pPoint->y,
	                    backend_sprite, flags, alpha);
}

void CSpriteSurface::BltSpriteNoClip(POINT* pPoint, CSprite* pSprite) {
	/* For now, same as BltSprite */
	BltSprite(pPoint, pSprite);
}

void CSpriteSurface::BltSpriteHalf(POINT* pPoint, CSprite* pSprite) {
	if (!pPoint || !pSprite) {
		return;
	}

	/* Get backend sprite */
	spritectl_sprite_t backend_sprite = get_backend_sprite(pSprite);
	if (!backend_sprite) {
		return;
	}

	/* Scale factor: 128 = 0.5x */
	int scale = 128;
	int flags = 0;
	spritectl_blt_sprite_scaled(m_backend_surface, pPoint->x, pPoint->y,
	                            backend_sprite, scale, flags);
}

void CSpriteSurface::BltSpriteAlpha(POINT* pPoint, CSprite* pSprite, BYTE alphaDepth) {
	if (!pPoint || !pSprite) {
		return;
	}

	/* Get backend sprite */
	spritectl_sprite_t backend_sprite = get_backend_sprite(pSprite);
	if (!backend_sprite) {
		return;
	}

	/* Blit with alpha */
	int flags = SPRITECTL_BLT_ALPHA;
	spritectl_blt_sprite(m_backend_surface, pPoint->x, pPoint->y,
	                    backend_sprite, flags, alphaDepth);
}

void CSpriteSurface::BltSpriteScale(POINT* pPoint, CSprite* pSprite, int scale) {
	if (!pPoint || !pSprite) {
		return;
	}

	/* Get backend sprite */
	spritectl_sprite_t backend_sprite = get_backend_sprite(pSprite);
	if (!backend_sprite) {
		return;
	}

	/* Scale parameter: 256 = 1x, 128 = 0.5x, 512 = 2x */
	int scale_factor = scale;
	int flags = 0;
	spritectl_blt_sprite_scaled(m_backend_surface, pPoint->x, pPoint->y,
	                            backend_sprite, scale_factor, flags);
}

/* ============================================================================
 * Stub implementations for other BltSprite variants
 * These will be implemented in future iterations
 * ============================================================================ */

void CSpriteSurface::BltSpriteColor(POINT* pPoint, CSprite* pSprite, BYTE rgb) {
	/* TODO: Implement color tinting */
	BltSprite(pPoint, pSprite);
}

void CSpriteSurface::BltSpriteDarkness(POINT* pPoint, CSprite* pSprite, BYTE DarkBits) {
	/* TODO: Implement darkness effect */
	BltSprite(pPoint, pSprite);
}

void CSpriteSurface::BltSpriteColorSet(POINT* pPoint, CSprite* pSprite, WORD colorSet) {
	/* TODO: Implement color set */
	BltSprite(pPoint, pSprite);
}

void CSpriteSurface::BltSpriteEffect(POINT* pPoint, CSprite* pSprite) {
	/* TODO: Implement effect */
	BltSprite(pPoint, pSprite);
}

void CSpriteSurface::BltSpriteAlpha4444SmallNotTrans(POINT* pPoint, CSprite* pSprite, BYTE alpha, BYTE shift) {
	/* TODO: Implement */
	BltSpriteAlpha(pPoint, pSprite, alpha);
}

void CSpriteSurface::BltSpriteAlpha4444NotTrans(POINT* pPoint, CSprite* pSprite, BYTE alpha) {
	/* TODO: Implement */
	BltSpriteAlpha(pPoint, pSprite, alpha);
}

void CSpriteSurface::BltSprite1555SmallNotTrans(POINT* pPoint, CSprite* pSprite, BYTE shift) {
	/* TODO: Implement */
	BltSprite(pPoint, pSprite);
}

void CSpriteSurface::BltSprite1555NotTrans(POINT* pPoint, CSprite* pSprite) {
	/* TODO: Implement */
	BltSprite(pPoint, pSprite);
}

void CSpriteSurface::BltSpritePalEffect(POINT* pPoint, CSpritePal* pSprite, MPalette &pal) {
	/* TODO: Implement palette effect */
}

void CSpriteSurface::BltSpritePal1555SmallNotTrans(POINT* pPoint, CSpritePal* pSprite, BYTE shift, MPalette &pal) {
	/* TODO: Implement */
}

void CSpriteSurface::BltSpritePal1555NotTrans(POINT* pPoint, CSpritePal* pSprite, MPalette &pal) {
	/* TODO: Implement */
}

void CSpriteSurface::BltSpriteAlphaFilter(POINT* pPoint, CSprite* pSprite) {
	/* TODO: Implement alpha filter */
	BltSprite(pPoint, pSprite);
}

void CSpriteSurface::BltSpriteAlphaFilterDarkness(POINT* pPoint, CSprite* pSprite, BYTE DarkBits) {
	/* TODO: Implement */
	BltSpriteDarkness(pPoint, pSprite, DarkBits);
}

void CSpriteSurface::BltSpriteDarkerFilter(POINT* pPoint, CSprite* pSprite) {
	/* TODO: Implement darker filter */
	BltSprite(pPoint, pSprite);
}

/* ============================================================================
 * Alpha Sprite Methods
 * ============================================================================ */

void CSpriteSurface::BltAlphaSprite(POINT* pPoint, CAlphaSprite* pSprite) {
	if (!pPoint || !pSprite) {
		return;
	}

	/* Get backend sprite */
	spritectl_sprite_t backend_sprite = get_backend_alpha_sprite(pSprite);
	if (!backend_sprite) {
		return;
	}

	/* Blit to backend surface */
	int flags = SPRITECTL_BLT_ALPHA;
	int alpha = 255;
	spritectl_blt_sprite(m_backend_surface, pPoint->x, pPoint->y,
	                    backend_sprite, flags, alpha);
}

void CSpriteSurface::BltAlphaSpriteAlpha(POINT* pPoint, CAlphaSprite* pSprite, BYTE alpha) {
	if (!pPoint || !pSprite) {
		return;
	}

	/* Get backend sprite */
	spritectl_sprite_t backend_sprite = get_backend_alpha_sprite(pSprite);
	if (!backend_sprite) {
		return;
	}

	/* Blit to backend surface with alpha */
	int flags = SPRITECTL_BLT_ALPHA;
	spritectl_blt_sprite(m_backend_surface, pPoint->x, pPoint->y,
	                    backend_sprite, flags, alpha);
}

void CSpriteSurface::BltAlphaSprite4444(POINT* pPoint, CAlphaSprite* pSprite) {
	/* TODO: Implement 4444 format conversion */
	/* For now, use regular blit */
	BltAlphaSprite(pPoint, pSprite);
}

void CSpriteSurface::BltAlphaSprite4444NotTrans(POINT* pPoint, CAlphaSprite* pSprite) {
	/* TODO: Implement 4444 NotTrans */
	BltAlphaSprite(pPoint, pSprite);
}

void CSpriteSurface::BltAlphaSprite4444SmallNotTrans(POINT* pPoint, CAlphaSprite* pSprite, BYTE shift) {
	/* TODO: Implement scaling */
	BltAlphaSprite(pPoint, pSprite);
}

void CSpriteSurface::BltAlphaSpritePal(POINT* pPoint, CAlphaSpritePal* pSprite, MPalette &pal) {
	if (!pPoint || !pSprite) {
		LOG_ERROR("[BltAlphaSpritePal] ERROR: Invalid parameters (pPoint=%p, pSprite=%p)\n", pPoint, pSprite);
		return;
	}

	/* Check if sprite is initialized */
	if (pSprite->IsNotInit()) {
		LOG_ERROR("[BltAlphaSpritePal] ERROR: Sprite not initialized\n");
		return;
	}

	/* Basic clipping check - skip if completely outside surface */
	int spriteWidth = pSprite->GetWidth();
	int spriteHeight = pSprite->GetHeight();

	/* Get surface dimensions */
	int surfaceWidth = m_width;
	int surfaceHeight = m_height;

	/* Check if sprite is completely outside the surface */
	bool outsideBounds = (pPoint->x >= surfaceWidth) ||
	                     (pPoint->y >= surfaceHeight) ||
	                     (pPoint->x + spriteWidth <= 0) ||
	                     (pPoint->y + spriteHeight <= 0);

	if (outsideBounds) {
		/* Sprite is completely outside, skip rendering */
		static int skipCount = 0;
		if (skipCount < 5) {
			LOG_WARN("[BltAlphaSpritePal] WARNING: Sprite at (%d,%d) size=%dx%d outside surface %dx%d, skipping\n",
			       pPoint->x, pPoint->y, spriteWidth, spriteHeight, surfaceWidth, surfaceHeight);
			skipCount++;
		}
		return;
	}

	/* Additional check: if sprite starts outside surface bounds, skip for now */
	/* TODO: Implement proper partial clipping */
	if (pPoint->x < 0 || pPoint->y < 0 ||
	    pPoint->x + spriteWidth > surfaceWidth ||
	    pPoint->y + spriteHeight > surfaceHeight) {
		static int partialCount = 0;
		if (partialCount < 5) {
			LOG_WARN("[BltAlphaSpritePal] WARNING: Partial clipping at (%d,%d) size=%dx%d, skipping (TODO: implement)\n",
			       pPoint->x, pPoint->y, spriteWidth, spriteHeight);
			partialCount++;
		}
		return;
	}

	/* Lock backend surface for direct pixel access */
	spritectl_surface_info_t surface_info;
	if (spritectl_lock_surface(m_backend_surface, &surface_info) != 0) {
		static int lockFailCount = 0;
		if (lockFailCount < 3) {
			LOG_WARN("[BltAlphaSpritePal] ERROR: Failed to lock surface\n");
			lockFailCount++;
		}
		return;
	}

	/* Get pixel pointer and pitch (pitch is in bytes, like Windows) */
	WORD* pixels = (WORD*)surface_info.pixels;
	int pitch = surface_info.pitch;

	/* Calculate destination pointer with offset */
	WORD* pDest = (WORD*)((BYTE*)pixels + pPoint->y * pitch + (pPoint->x << 1));

	/* Call sprite's Blt method to render with palette */
	/* Pass pitch in bytes (same as Windows version) */
	/* TODO: Implement proper clipping for partially visible sprites */
	pSprite->Blt(pDest, pitch, pal);

	/* Unlock surface */
	spritectl_unlock_surface(m_backend_surface);
}

void CSpriteSurface::BltAlphaSpritePalAlpha(POINT* pPoint, CAlphaSpritePal* pSprite, BYTE alpha, MPalette &pal) {
	/* TODO: Implement */
}

void CSpriteSurface::BltAlphaSpritePal4444(POINT* pPoint, CAlphaSpritePal* pSprite, MPalette &pal) {
	/* TODO: Implement */
}

void CSpriteSurface::BltAlphaSpritePal4444NotTrans(POINT* pPoint, CAlphaSpritePal* pSprite, MPalette &pal) {
	/* TODO: Implement */
}

void CSpriteSurface::BltAlphaSpritePal4444SmallNotTrans(POINT* pPoint, CAlphaSpritePal* pSprite, BYTE shift, MPalette &pal) {
	/* TODO: Implement */
}

/* ============================================================================
 * Index Sprite Methods
 * ============================================================================ */

void CSpriteSurface::BltIndexSprite(POINT* pPoint, CIndexSprite* pSprite) {
	if (!pPoint || !pSprite) {
		return;
	}

	/* Get backend sprite */
	spritectl_sprite_t backend_sprite = get_backend_index_sprite(pSprite);
	if (!backend_sprite) {
		return;
	}

	/* Blit to backend surface */
	int flags = 0;
	int alpha = 255;
	spritectl_blt_sprite(m_backend_surface, pPoint->x, pPoint->y,
	                    backend_sprite, flags, alpha);
}

void CSpriteSurface::BltIndexSpriteDarkness(POINT* pPoint, CIndexSprite* pSprite, BYTE DarkBits) {
	/* TODO: Implement darkness effect */
	BltIndexSprite(pPoint, pSprite);
}

void CSpriteSurface::BltIndexSpriteAlpha(POINT* pPoint, CIndexSprite* pSprite, BYTE alpha) {
	if (!pPoint || !pSprite) {
		return;
	}

	/* Get backend sprite */
	spritectl_sprite_t backend_sprite = get_backend_index_sprite(pSprite);
	if (!backend_sprite) {
		return;
	}

	/* Blit to backend surface with alpha */
	int flags = SPRITECTL_BLT_ALPHA;
	spritectl_blt_sprite(m_backend_surface, pPoint->x, pPoint->y,
	                    backend_sprite, flags, alpha);
}

void CSpriteSurface::BltIndexSpriteColor(POINT* pPoint, CIndexSprite* pSprite, BYTE rgb) {
	/* TODO: Implement color tinting */
	BltIndexSprite(pPoint, pSprite);
}

void CSpriteSurface::BltIndexSpriteColorSet(POINT* pPoint, CIndexSprite* pSprite, WORD colorSet) {
	/* TODO: Implement color set */
	BltIndexSprite(pPoint, pSprite);
}

void CSpriteSurface::BltIndexSpriteEffect(POINT* pPoint, CIndexSprite* pSprite) {
	/* TODO: Implement effect */
	BltIndexSprite(pPoint, pSprite);
}

void CSpriteSurface::BltIndexSpriteBrightness(POINT* pPoint, CIndexSprite* pSprite, BYTE BrightBits) {
	/* TODO: Implement brightness */
	BltIndexSprite(pPoint, pSprite);
}

/* ============================================================================
 * Shadow Sprite Methods
 * ============================================================================ */

void CSpriteSurface::BltShadowSprite(POINT* pPoint, CShadowSprite* pSprite) {
	if (!pPoint || !pSprite) {
		return;
	}

	/* Get backend sprite */
	spritectl_sprite_t backend_sprite = get_backend_shadow_sprite(pSprite);
	if (!backend_sprite) {
		return;
	}

	/* fix: the old DirectDraw client's plain (non-Darkness) BltShadowSprite
	 * paints literal opaque black (CShadowSprite::Blt's memset(dst,0,...)),
	 * not a flat gray/placeholder color. Reuse the darken blit with a
	 * shift big enough (>=6, since RGB565's widest channel is 6 bits) to
	 * crush every channel to 0 regardless of what was underneath - exactly
	 * equivalent to painting black, using the same shape-mask sprite. */
	spritectl_blt_shadow_darken(m_backend_surface, pPoint->x, pPoint->y,
	                             backend_sprite, 16);
}

void CSpriteSurface::BltShadowSpriteSmall(POINT* pPoint, CShadowSprite* pSprite, BYTE shift) {
	/* TODO: Implement scaling */
	BltShadowSprite(pPoint, pSprite);
}

void CSpriteSurface::BltShadowSpriteDarkness(POINT* pPoint, CShadowSprite* pSprite, BYTE DarkBits) {
	if (!pPoint || !pSprite) {
		return;
	}

	/* fix: real shadow darkening (see CShadowSprite::BltDarkness in the old
	 * DirectDraw client) - darkens whatever ground pixels are already at
	 * this position by right-shifting each RGB channel by DarkBits, rather
	 * than painting a flat gray color over them. This is what makes shadows
	 * read as translucent (terrain shows through, just dimmed) instead of
	 * an opaque colored patch. */
	spritectl_sprite_t backend_sprite = get_backend_shadow_sprite(pSprite);
	if (!backend_sprite) {
		return;
	}

	spritectl_blt_shadow_darken(m_backend_surface, pPoint->x, pPoint->y,
	                             backend_sprite, DarkBits);
}

void CSpriteSurface::BltShadowSprite4444(POINT* pPoint, CShadowSprite* pSprite, WORD pixel) {
	/* TODO: Implement 4444 format */
	BltShadowSprite(pPoint, pSprite);
}

void CSpriteSurface::BltShadowSpriteSmall4444(POINT* pPoint, CShadowSprite* pSprite, WORD pixel, BYTE shift) {
	/* TODO: Implement scaling + 4444 */
	BltShadowSprite(pPoint, pSprite);
}

/* ============================================================================
 * Sprite Outline Methods
 * ============================================================================ */

void CSpriteSurface::BltSpriteOutline(CSpriteOutlineManager *pSOM, WORD color) {
	/* TODO: Implement sprite outline */
}

void CSpriteSurface::BltSpriteOutlineOnly(CSpriteOutlineManager* pSOM, WORD color) {
	/* TODO: Implement */
}

void CSpriteSurface::BltSpriteOutlineDarkness(CSpriteOutlineManager* pSOM, WORD color, BYTE DarkBits) {
	/* TODO: Implement */
}
