//----------------------------------------------------------------------
// CDirectDraw.cpp
//
// SDL2 Implementation (Cross-platform)
// Windows DirectX implementation removed - using SDL2 on all platforms
// NOTE: Static member definitions are in CDirectDraw_StaticMembers.cpp
//----------------------------------------------------------------------

#include "CDirectDraw.h"

//-----------------------------------------------------------------------------
// Static member initialization for DirectDraw objects
// Note: These are opaque pointers/stubs for SDL2 backend
//-----------------------------------------------------------------------------
LPDIRECTDRAW7					CSDLGraphics::m_pDD					= NULL;
LPDIRECTDRAWSURFACE7			CSDLGraphics::m_pDDSPrimary			= NULL;
LPDIRECTDRAWSURFACE7			CSDLGraphics::m_pDDSBack				= NULL;
LPDIRECTDRAWGAMMACONTROL	CSDLGraphics::m_pDDGammaControl		= NULL;

HWND								CSDLGraphics::m_hWnd					= NULL;

bool								CSDLGraphics::m_bFullscreen			= true;
WORD								CSDLGraphics::m_ScreenWidth			= 0;
WORD								CSDLGraphics::m_ScreenHeight			= 0;
bool								CSDLGraphics::m_b565					= true;
bool								CSDLGraphics::m_b3D					= true;
bool								CSDLGraphics::m_bMMX					= false;
bool								CSDLGraphics::m_bGammaControl		= false;
DDGAMMARAMP						CSDLGraphics::m_DDGammaRamp;
WORD								CSDLGraphics::m_GammaStep				= 100;
WORD								CSDLGraphics::m_AddGammaStep[3];

uint8_t								CSDLGraphics::m_GammaLUT[3][256];
bool								CSDLGraphics::m_bGammaNeutral			= true;

RECT								CSDLGraphics::m_rcWindow;
RECT								CSDLGraphics::m_rcScreen;
RECT								CSDLGraphics::m_rcViewport;

// Note: Color mask static members are defined in CDirectDraw_StaticMembers.cpp

//-----------------------------------------------------------------------------
// Constructor/Destructor (stub - not implemented)
//-----------------------------------------------------------------------------
CSDLGraphics::CSDLGraphics()
{
}

CSDLGraphics::~CSDLGraphics()
{
}

//-----------------------------------------------------------------------------
// InitMask
//-----------------------------------------------------------------------------
void CSDLGraphics::InitMask(bool b565)
{
	// 5:6:5 format for SDL2
	s_wMASK_SHIFT[0] = 11;
	s_wMASK_SHIFT[1] = 5;
	s_wMASK_SHIFT[2] = 0;
	s_wMASK_SHIFT[3] = 0;
	s_wMASK_SHIFT[4] = 0;

	s_dwMASK_SHIFT[0] = 0xF800;
	s_dwMASK_SHIFT[1] = 0x07E0;
	s_dwMASK_SHIFT[2] = 0x001F;
	s_dwMASK_SHIFT[3] = 0;
	s_dwMASK_SHIFT[4] = 0;

	s_wMASK_RGB[0] = 0;
	s_wMASK_RGB[1] = 11;
	s_wMASK_RGB[2] = 5;
	s_wMASK_RGB[3] = 0;
	s_wMASK_RGB[4] = 0;
	s_wMASK_RGB[5] = 0;

	s_dwMASK_RGB[0] = 0x0000F800;
	s_dwMASK_RGB[1] = 0x000007E0;
	s_dwMASK_RGB[2] = 0x0000001F;
	s_dwMASK_RGB[3] = 0;
	s_dwMASK_RGB[4] = 0;
	s_dwMASK_RGB[5] = 0;

	s_qwMASK_RGB[0] = 0x000000000000F800;
	s_qwMASK_RGB[1] = 0x0000000000007E0;
	s_qwMASK_RGB[2] = 0x00000000000001F;
	s_qwMASK_RGB[3] = 0;
	s_qwMASK_RGB[4] = 0;
	s_qwMASK_RGB[5] = 0;

	s_bSHIFT_R = 3;
	s_bSHIFT_G = 2;
	s_bSHIFT_B = 3;
	s_bSHIFT_A = 4;

	s_dwMASK_SHIFT_COUNT[0] = 5;
	s_dwMASK_SHIFT_COUNT[1] = 6;
	s_dwMASK_SHIFT_COUNT[2] = 5;
	s_dwMASK_SHIFT_COUNT[3] = 0;
	s_dwMASK_SHIFT_COUNT[4] = 0;

	s_dwMASK_RGB_COUNT[0] = 0;
	s_dwMASK_RGB_COUNT[1] = 5;
	s_dwMASK_RGB_COUNT[2] = 6;
	s_dwMASK_RGB_COUNT[3] = 0;
	s_dwMASK_RGB_COUNT[4] = 0;
	s_dwMASK_RGB_COUNT[5] = 0;

	(void)b565;  // Parameter kept for compatibility
}

//-----------------------------------------------------------------------------
// Bitmask methods for SDL2
//-----------------------------------------------------------------------------

int CSDLGraphics::Get_Count_Rbit()
{
	// For 5:6:5 format, R uses 5 bits
	return 5;
}

int CSDLGraphics::Get_Count_Gbit()
{
	// For 5:6:5 format, G uses 6 bits
	return 6;
}

int CSDLGraphics::Get_Count_Bbit()
{
	// For 5:6:5 format, B uses 5 bits
	return 5;
}

DWORD CSDLGraphics::Get_R_Bitmask()
{
	// 5:6:5 format: R is at bits 11-15
	return 0xF800;
}

DWORD CSDLGraphics::Get_G_Bitmask()
{
	// 5:6:5 format: G is at bits 5-10
	return 0x07E0;
}

DWORD CSDLGraphics::Get_B_Bitmask()
{
	// 5:6:5 format: B is at bits 0-4
	return 0x001F;
}

DWORD CSDLGraphics::Get_BPP()
{
	// SDL2 typically uses 16-bit color
	return 16;
}

// Packs 8-bit RGB into RGB565, rounding to the nearest 5/6-bit level
// instead of truncating (matches SpriteLib's spritectl_rgb_to_565 - not
// shared directly since SpriteLibBackendSDL.h is only usable from the
// SpriteLib backend translation unit).
static inline uint16_t GammaPackRGB565(uint8_t r, uint8_t g, uint8_t b)
{
	uint16_t r5 = (uint16_t)((r + 4) >> 3);
	uint16_t g6 = (uint16_t)((g + 2) >> 2);
	uint16_t b5 = (uint16_t)((b + 4) >> 3);
	if (r5 > 0x1F) r5 = 0x1F;
	if (g6 > 0x3F) g6 = 0x3F;
	if (b5 > 0x1F) b5 = 0x1F;
	return (uint16_t)((r5 << 11) | (g6 << 5) | b5);
}

//-----------------------------------------------------------------------------
// Software gamma/brightness control
//
// The original DirectDraw client drove a real hardware DDGAMMARAMP: it read
// the driver's baseline (linear/identity) ramp once, then per-channel
// lerped it toward black (step<100) or white (step>100), applied by the
// GPU at scanout for free. There is no equivalent hardware ramp under
// SDL2, so this rebuilds the same lerp math as an 8-bit LUT and applies it
// in software, once per frame, to the backbuffer pixels right before they
// go to the GPU (see ApplyGammaToBuffer / spritectl_present_surface).
//-----------------------------------------------------------------------------
void CSDLGraphics::RebuildGammaLUT()
{
	bool neutral = true;

	for (int ch = 0; ch < 3; ch++) {
		int addGammaStep = (int)(short)m_AddGammaStep[ch];
		if (addGammaStep > 100) addGammaStep = 100;
		if (addGammaStep < -100) addGammaStep = -100;

		int step;
		if (addGammaStep > 0) {
			step = m_GammaStep + (200 - (int)m_GammaStep) * addGammaStep / 100;
		} else {
			step = m_GammaStep + (int)m_GammaStep * addGammaStep / 100;
		}

		int maxValue, stepValue;
		if (step < 100) {
			maxValue = 0;
			stepValue = 100 - step;
		} else {
			maxValue = 255;
			stepValue = step - 100;
		}

		if (stepValue != 0) {
			neutral = false;
		}

		for (int v = 0; v < 256; v++) {
			int out = v + (maxValue - v) * stepValue / 100;
			if (out < 0) out = 0;
			if (out > 255) out = 255;
			m_GammaLUT[ch][v] = (uint8_t)out;
		}
	}

	m_bGammaNeutral = neutral;
}

// step: 50 (dark) ~ 100 (normal, default) ~ 150 (bright). 0xffff re-applies
// the last step set (used to refresh after an add-gamma tint changes).
void CSDLGraphics::SetGammaRamp(WORD step)
{
	if (step == (WORD)-1)
		step = m_GammaStep;
	else
		m_GammaStep = step;

	RebuildGammaLUT();
}

void CSDLGraphics::RestoreGammaRamp()
{
	m_GammaStep = 100;
	m_AddGammaStep[0] = m_AddGammaStep[1] = m_AddGammaStep[2] = 0;
	RebuildGammaLUT();
}

// Per-channel additive screen tint on top of the base gamma step - used by
// MEventManager for server-driven EVENTFLAG_FADE_SCREEN effects. Called
// with no args to clear the tint back to neutral.
void CSDLGraphics::SetAddGammaRamp(WORD rStep, WORD gStep, WORD bStep)
{
	m_AddGammaStep[0] = rStep;
	m_AddGammaStep[1] = gStep;
	m_AddGammaStep[2] = bStep;

	RebuildGammaLUT();
}

void CSDLGraphics::ApplyGammaToBuffer(uint16_t* pixels, int width, int height, int pitchBytes)
{
	if (m_bGammaNeutral || pixels == NULL || width <= 0 || height <= 0) {
		return;
	}

	const uint8_t* lutR = m_GammaLUT[0];
	const uint8_t* lutG = m_GammaLUT[1];
	const uint8_t* lutB = m_GammaLUT[2];

	uint8_t* rowBytes = (uint8_t*)pixels;
	for (int y = 0; y < height; y++) {
		uint16_t* row = (uint16_t*)(rowBytes + (size_t)y * pitchBytes);
		for (int x = 0; x < width; x++) {
			uint16_t pixel = row[x];
			uint8_t r5 = (pixel >> 11) & 0x1F;
			uint8_t g6 = (pixel >> 5) & 0x3F;
			uint8_t b5 = pixel & 0x1F;

			uint8_t r8 = (r5 << 3) | (r5 >> 2);
			uint8_t g8 = (g6 << 2) | (g6 >> 4);
			uint8_t b8 = (b5 << 3) | (b5 >> 2);

			row[x] = GammaPackRGB565(lutR[r8], lutG[g8], lutB[b8]);
		}
	}
}
