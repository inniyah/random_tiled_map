// Copyright (c) 2013, Miriam Ruiz <miriam@debian.org> - All rights reserved
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution. 
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.
//
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <sys/types.h>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cmath>


class TileSet {
public:
	enum {
		// B(lock) + S(olid)/E(mpty) + U(p)/D(own)/L(eft)/R(ight)
		BS = 1 << 0, BSU = 1 << 1, BSD = 1 << 2, BSL = 1 << 3, BSR = 1 << 4,
		BE = 1 << 5, BEU = 1 << 6, BED = 1 << 7, BEL = 1 << 8, BER = 1 << 9,
	};

	enum {
		EMPTY, SOLID,
		HALF_UP, HALF_DOWN, HALF_LEFT, HALF_RIGHT,

		SYMMETRIC_EDGES_MAX = HALF_RIGHT,

		// E(mpty)/S(olid)/H(alf)/O(ppositehalf) + COR(ner) + _ + UP/DOWN/LEFT/RIGHT + T(angential)/N(ormal)
		ECOR_UPT = ((SYMMETRIC_EDGES_MAX + 1) & 254),
			     ECOR_UPN,    SCOR_UPT,    SCOR_UPN,    HCOR_UPT,    HCOR_UPN,    OCOR_UPT,    OCOR_UPN,
		ECOR_DOWNT,  ECOR_DOWNN,  SCOR_DOWNT,  SCOR_DOWNN,  HCOR_DOWNT,  HCOR_DOWNN,  OCOR_DOWNT,  OCOR_DOWNN,
		ECOR_LEFTT,  ECOR_LEFTN,  SCOR_LEFTT,  SCOR_LEFTN,  HCOR_LEFTT,  HCOR_LEFTN,  OCOR_LEFTT,  OCOR_LEFTN,
		ECOR_RIGHTT, ECOR_RIGHTN, SCOR_RIGHTT, SCOR_RIGHTN, HCOR_RIGHTT, HCOR_RIGHTN, OCOR_RIGHTT, OCOR_RIGHTN,

		// E(mpty)/S(olid) + BI + CORNER + T(angential)/N(ormal)
		EBICORNERT,  EBICORNERN,  SBICORNERT,  SBICORNERN,

		COMPLEMENTARY_EDGES_MAX = SBICORNERN,
	};

	int HMirrorEdge(int edge) const {
		return edge;
	}

	int VMirrorEdge(int edge) const {
		return edge;
	}

	int EdgesMatchError(int e1, int e2) const {
		if (e1 <= SYMMETRIC_EDGES_MAX || e2 <= SYMMETRIC_EDGES_MAX) {
			if (e1 == e2)
				return 0;
			return 100;
		}

		if (e1 <= COMPLEMENTARY_EDGES_MAX || e2 <= COMPLEMENTARY_EDGES_MAX) {
			if (e1 == e2)
				return 50;
			if ((e1 & 254) == (e2 & 254))
				return 0;
			return 100;
		}

		return 100;
	}

	struct TileConfig {
		const char * FileName;
		uint32_t SolidFlags;
		int EdgeUp;
		int EdgeDown;
		int EdgeLeft;
		int EdgeRight;
		int Fill;
	};

	static const TileConfig TileData[];
	static unsigned int TileDataNumAllTiles;

	unsigned int NumTiles() const {
		return TileDataNumAllTiles;
	}

	inline const TileConfig &getTile(unsigned int index) const {
		return TileData[index];
	}
	inline const char * FileName(unsigned int index) const {
		return TileData[index].FileName;
	}
	inline uint32_t SolidFlags(unsigned int index) const {
		return TileData[index].SolidFlags;
	}
	inline int EdgeUp(unsigned int index) const {
		return TileData[index].EdgeUp;
	}
	inline int EdgeDown(unsigned int index) const {
		return TileData[index].EdgeDown;
	}
	inline int EdgeLeft(unsigned int index) const {
		return TileData[index].EdgeLeft;
	}
	inline int EdgeRight(unsigned int index) const {
		return TileData[index].EdgeRight;
	}
	inline int Fill(unsigned int index) const {
		return TileData[index].Fill;
	}

	enum {
		TILE_EMPTY = 0,
		TILE_SOLID = 1,
		TILE_HALF_U = 4,
		TILE_HALF_D = 2,
		TILE_HALF_L = 3,
		TILE_HALF_R = 5,
		TILE_HALF_UL = 32,
		TILE_HALF_UR = 33,
		TILE_HALF_DL = 31,
		TILE_HALF_DR = 30,
		TILE_ECOR_UL = 5,
		TILE_ECOR_UR = 6,
		TILE_ECOR_DL = 8,
		TILE_ECOR_DR = 7,
		TILE_SCOR_UL = 9,
		TILE_SCOR_UR = 10,
		TILE_SCOR_DL = 12,
		TILE_SCOR_DR = 11,
	};

	inline unsigned int SolidTile() const {
		return TILE_SOLID;
	}

	inline unsigned int EmptyTile() const {
		return TILE_EMPTY;
	}

	// env is a binary number representing flags that describe the environment:
	// 0b(ul)(u)(ur)(l)(c)(r)(dl)(d)(dr)
	inline unsigned int InitialTileGuess(uint32_t env) const {
		switch (env) {
			case 0b000000111: return TILE_HALF_U;
			case 0b111000000: return TILE_HALF_D;
			case 0b001001001: return TILE_HALF_L;
			case 0b100100100: return TILE_HALF_R;
			case 0b000001011: return TILE_HALF_UL;
			case 0b000100110: return TILE_HALF_UR;
			case 0b011001000: return TILE_HALF_DL;
			case 0b110100000: return TILE_HALF_DR;
			case 0b000000001: return TILE_ECOR_UL;
			case 0b000000100: return TILE_ECOR_UR;
			case 0b001000000: return TILE_ECOR_DL;
			case 0b100000000: return TILE_ECOR_DR;
			case 0b111100100: return TILE_SCOR_UL;
			case 0b111001001: return TILE_SCOR_UR;
			case 0b100100111: return TILE_SCOR_DL;
			case 0b001001111: return TILE_SCOR_DR;
			default:          return TILE_SOLID;
		}
	}

private:

};

const TileSet::TileConfig TileSet::TileData[] = {
	// FileName         SolidFlags          EdgeUp       EdgeDown     EdgeLeft     EdgeRight    Fill
	{ "tiles/A1.png",  BE+BEU+BED+BER+BEL, EMPTY,       EMPTY,       EMPTY,       EMPTY       ,   0 }, // " "
	{ "tiles/A2.png",  BS+BSU+BSD+BSR+BSL, SOLID,       SOLID,       SOLID,       SOLID       , 100 }, // "█"
	{ "tiles/B1.png",  BEU+BSD,            EMPTY,       SOLID,       HALF_DOWN,   HALF_DOWN   ,  50 }, // "▄"
	{ "tiles/B2.png",  BER+BSL,            HALF_LEFT,   HALF_LEFT,   SOLID,       EMPTY       ,  50 }, // "▌"
	{ "tiles/B3.png",  BED+BSU,            SOLID,       EMPTY,       HALF_UP,     HALF_UP     ,  50 }, // "▀"
	{ "tiles/B4.png",  BEL+BSR,            HALF_RIGHT,  HALF_RIGHT,  EMPTY,       SOLID       ,  50 }, // "▐"
	{ "tiles/C11.png", BEU+BEL,            EMPTY,       HALF_RIGHT,  EMPTY,       HALF_DOWN   ,  25 }, // "▗"
	{ "tiles/C12.png", BEU+BER,            EMPTY,       HALF_LEFT,   HALF_DOWN,   EMPTY       ,  25 }, // "▖"
	{ "tiles/C13.png", BED+BER,            HALF_LEFT,   EMPTY,       HALF_UP,     EMPTY       ,  25 }, // "▘"
	{ "tiles/C14.png", BED+BEL,            HALF_RIGHT,  EMPTY,       EMPTY,       HALF_UP     ,  25 }, // "▝"
	{ "tiles/C21.png", BSU+BSL,            SOLID,       HALF_LEFT,   SOLID,       HALF_UP     ,  75 }, // "▛"
	{ "tiles/C22.png", BSU+BSR,            SOLID,       HALF_RIGHT,  HALF_UP,     SOLID       ,  75 }, // "▜"
	{ "tiles/C23.png", BSD+BSR,            HALF_RIGHT,  SOLID,       HALF_DOWN,   SOLID       ,  75 }, // "▟"
	{ "tiles/C24.png", BSD+BSL,            HALF_LEFT,   SOLID,       SOLID,       HALF_DOWN   ,  75 }, // "▙"

	// Open Corners
	{ "tiles/D11.png", BSD+BEU,            EMPTY,       SCOR_LEFTN,  ECOR_DOWNN,  HALF_DOWN   ,  35 },
	{ "tiles/D12.png", BSD+BEU,            EMPTY,       SCOR_RIGHTN, HALF_DOWN,   ECOR_DOWNN  ,  35 },
	{ "tiles/D13.png", BSL+BER,            ECOR_LEFTN,  HALF_LEFT,   SCOR_UPN,    EMPTY       ,  35 },
	{ "tiles/D14.png", BSL+BER,            HALF_LEFT,   ECOR_LEFTN,  SCOR_DOWNN,  EMPTY       ,  35 },
	{ "tiles/D15.png", BSU+BED,            SCOR_RIGHTN, EMPTY,       HALF_UP,     ECOR_UPN    ,  35 },
	{ "tiles/D16.png", BSU+BED,            SCOR_LEFTN,  EMPTY,       ECOR_UPN,    HALF_UP     ,  35 },
	{ "tiles/D17.png", BSR+BEL,            HALF_RIGHT,  ECOR_RIGHTN, EMPTY,       SCOR_DOWNN  ,  35 },
	{ "tiles/D18.png", BSR+BEL,            ECOR_RIGHTN, HALF_RIGHT,  EMPTY,       SCOR_UPN    ,  35 },
	{ "tiles/D21.png", BED+BSU,            SOLID,       ECOR_LEFTN,  SCOR_DOWNN,  HALF_UP     ,  65 },
	{ "tiles/D22.png", BED+BSU,            SOLID,       ECOR_RIGHTN, HALF_UP,     SCOR_UPN    ,  65 },
	{ "tiles/D23.png", BEL+BSR,            SCOR_LEFTN,  HALF_RIGHT,  ECOR_UPN,    SOLID       ,  65 },
	{ "tiles/D24.png", BEL+BSR,            HALF_RIGHT,  SCOR_LEFTN,  ECOR_DOWNN,  SOLID       ,  65 },
	{ "tiles/D25.png", BEU+BSD,            ECOR_RIGHTN, SOLID,       HALF_DOWN,   SCOR_UPN    ,  65 },
	{ "tiles/D26.png", BEU+BSD,            ECOR_LEFTN,  SOLID,       SCOR_UPN,    HALF_DOWN   ,  65 },
	{ "tiles/D27.png", BER+BSL,            HALF_LEFT,   SCOR_RIGHTN, SOLID,       ECOR_DOWNN  ,  65 },
	{ "tiles/D28.png", BER+BSL,            SCOR_RIGHTN, HALF_LEFT,   SOLID,       ECOR_UPN    ,  65 },

	// Oblique Tiles
	{ "tiles/E1.png",  BEU+BSR+BSD+BEL,    ECOR_RIGHTN, SCOR_LEFTN,  ECOR_DOWNN,  SCOR_UPN    ,  50 },
	{ "tiles/E2.png",  BEU+BER+BSD+BSL,    ECOR_LEFTN,  SCOR_RIGHTN, SCOR_UPN,    ECOR_DOWNN  ,  50 },
	{ "tiles/E3.png",  BSU+BER+BED+BSL,    SCOR_RIGHTN, ECOR_LEFTN,  SCOR_DOWNN,  ECOR_UPN    ,  50 },
	{ "tiles/E4.png",  BSU+BSR+BED+BEL,    SCOR_LEFTN,  ECOR_RIGHTN, ECOR_UPN,    SCOR_DOWNN  ,  50 },
	{ "tiles/F11.png", BE+BEU+BED+BER+BEL, ECOR_LEFTT,  EMPTY,       ECOR_UPT,    EMPTY       ,   5 }, // "⦍"
	{ "tiles/F12.png", BE+BEU+BED+BER+BEL, ECOR_RIGHTT, EMPTY,       EMPTY,       ECOR_UPT    ,   5 }, // "⦐"
	{ "tiles/F13.png", BE+BEU+BED+BER+BEL, EMPTY,       ECOR_RIGHTT, EMPTY,       ECOR_DOWNT  ,   5 }, // "⦎"
	{ "tiles/F14.png", BE+BEU+BED+BER+BEL, EMPTY,       ECOR_LEFTT,  ECOR_DOWNT,  EMPTY       ,   5 }, // "⦏"
	{ "tiles/F15.png", BE+BEU+BED+BER+BEL, ECOR_LEFTT,  ECOR_RIGHTT, ECOR_UPT,    ECOR_DOWNT  ,  10 },
	{ "tiles/F16.png", BE+BEU+BED+BER+BEL, ECOR_RIGHTT, ECOR_LEFTT,  ECOR_DOWNT,  ECOR_UPT    ,  10 },
	{ "tiles/F21.png", BS+BSU+BSD+BSR+BSL, SCOR_LEFTT,  SOLID,       SCOR_UPT,    SOLID       ,  95 }, // "⦍"
	{ "tiles/F22.png", BS+BSU+BSD+BSR+BSL, SCOR_RIGHTT, SOLID,       SOLID,       SCOR_UPT    ,  95 }, // "⦐"
	{ "tiles/F23.png", BS+BSU+BSD+BSR+BSL, SOLID,       SCOR_RIGHTT, SOLID,       SCOR_DOWNT  ,  95 }, // "⦎"
	{ "tiles/F24.png", BS+BSU+BSD+BSR+BSL, SOLID,       SCOR_LEFTT,  SCOR_DOWNT,  SOLID       ,  95 }, // "⦏"
	{ "tiles/F25.png", BS+BSU+BSD+BSR+BSL, SCOR_LEFTT,  SCOR_RIGHTT, SCOR_UPT,    SCOR_DOWNT  ,  90 },
	{ "tiles/F26.png", BS+BSU+BSD+BSR+BSL, SCOR_RIGHTT, SCOR_LEFTT,  SCOR_DOWNT,  SCOR_UPT    ,  90 },

	// Close Corners
	{ "tiles/G11.png", BEU+BER+BEL,        EMPTY,       HCOR_LEFTN,  ECOR_DOWNN,  EMPTY       ,  15 },
	{ "tiles/G12.png", BEU+BER+BEL,        EMPTY,       HCOR_RIGHTN, EMPTY,       ECOR_DOWNN  ,  15 },
	{ "tiles/G13.png", BER+BED+BEU,        ECOR_LEFTN,  EMPTY,       HCOR_UPN,    EMPTY       ,  15 },
	{ "tiles/G14.png", BER+BED+BEU,        EMPTY,       ECOR_LEFTN,  HCOR_DOWNN,  EMPTY       ,  15 },
	{ "tiles/G15.png", BED+BEL+BER,        HCOR_RIGHTN, EMPTY,       EMPTY,       ECOR_UPN    ,  15 },
	{ "tiles/G16.png", BED+BEL+BER,        HCOR_LEFTN,  EMPTY,       ECOR_UPN,    EMPTY       ,  15 },
	{ "tiles/G17.png", BEL+BEU+BED,        EMPTY,       HCOR_RIGHTN, EMPTY,       SCOR_DOWNN  ,  15 },
	{ "tiles/G18.png", BEL+BEU+BED,        HCOR_RIGHTN, EMPTY,       EMPTY,       SCOR_UPN    ,  15 },
	{ "tiles/G21.png", BSU+BSR+BSL,        SOLID,       OCOR_LEFTN,  SCOR_DOWNN,  SOLID       ,  85 },
	{ "tiles/G22.png", BSU+BSR+BSL,        SOLID,       OCOR_RIGHTN, SOLID,       SCOR_DOWNN  ,  85 },
	{ "tiles/G23.png", BSR+BSD+BSU,        SCOR_LEFTN,  SOLID,       OCOR_UPN,    SOLID       ,  85 },
	{ "tiles/G24.png", BSR+BSD+BSU,        SOLID,       SCOR_LEFTN,  OCOR_DOWNN,  SOLID       ,  85 },
	{ "tiles/G25.png", BSD+BSL+BSR,        OCOR_RIGHTN, SOLID,       SOLID,       SCOR_UPN    ,  85 },
	{ "tiles/G26.png", BSD+BSL+BSR,        OCOR_LEFTN,  SOLID,       SCOR_UPN,    SOLID       ,  85 },
	{ "tiles/G27.png", BSL+BSU+BSD,        SOLID,       SCOR_RIGHTN, SOLID,       OCOR_DOWNN  ,  85 },
	{ "tiles/G28.png", BSL+BSU+BSD,        SCOR_RIGHTN, SOLID,       SOLID,       OCOR_UPN    ,  85 },
	{ "tiles/H11.png", BER,                HCOR_LEFTT,  HALF_LEFT,   SCOR_UPT,    EMPTY       ,  45 },
	{ "tiles/H12.png", BEL,                HCOR_RIGHTT, HALF_RIGHT,  EMPTY,       SCOR_UPT    ,  45 },
	{ "tiles/H13.png", BED,                SCOR_RIGHTT, EMPTY,       HALF_UP,     HCOR_UPT    ,  45 },
	{ "tiles/H14.png", BEU,                EMPTY,       SCOR_RIGHTT, HALF_DOWN,   HCOR_DOWNT  ,  45 },
	{ "tiles/H15.png", BEL,                HALF_RIGHT,  HCOR_RIGHTT, EMPTY,       SCOR_DOWNT  ,  45 },
	{ "tiles/H16.png", BER,                HALF_LEFT,   HCOR_LEFTT,  SCOR_DOWNT,  EMPTY       ,  45 },
	{ "tiles/H17.png", BEU,                EMPTY,       SCOR_LEFTT,  HCOR_DOWNT,  HALF_DOWN   ,  45 },
	{ "tiles/H18.png", BED,                SCOR_LEFTT,  EMPTY,       HCOR_UPT,    HALF_UP     ,  45 },
	{ "tiles/H21.png", BSR,                OCOR_LEFTT,  HALF_RIGHT,  ECOR_UPT,    SOLID       ,  55 },
	{ "tiles/H22.png", BSL,                OCOR_RIGHTT, HALF_LEFT,   SOLID,       ECOR_UPT    ,  55 },
	{ "tiles/H23.png", BSD,                ECOR_RIGHTT, SOLID,       HALF_DOWN,   OCOR_UPT    ,  55 },
	{ "tiles/H24.png", BSU,                SOLID,       ECOR_RIGHTT, HALF_UP,     OCOR_DOWNT  ,  55 },
	{ "tiles/H25.png", BSL,                HALF_LEFT,   OCOR_RIGHTT, SOLID,       ECOR_DOWNT  ,  55 },
	{ "tiles/H26.png", BSR,                HALF_RIGHT,  OCOR_LEFTT,  ECOR_DOWNT,  SOLID       ,  55 },
	{ "tiles/H27.png", BSU,                SOLID,       ECOR_LEFTT,  OCOR_DOWNT,  HALF_UP     ,  55 },
	{ "tiles/H28.png", BSD,                ECOR_LEFTT,  SOLID,       OCOR_UPT,    HALF_DOWN   ,  55 },

	// Oblique Corners
	{ "tiles/I11.png", BEU+BER+BSD+BEL,    EMPTY,       SBICORNERN,  ECOR_DOWNN,  ECOR_DOWNN  ,  20 },
	{ "tiles/I12.png", BEU+BER+BED+BSL,    ECOR_LEFTN,  ECOR_LEFTN,  SBICORNERN,  EMPTY       ,  20 },
	{ "tiles/I13.png", BSU+BER+BED+BEL,    SBICORNERN,  EMPTY,       ECOR_UPN,    ECOR_UPN    ,  20 },
	{ "tiles/I14.png", BEU+BSR+BED+BEL,    ECOR_RIGHTN, ECOR_RIGHTN, EMPTY,       SBICORNERN  ,  20 },
	{ "tiles/I21.png", BSU+BSR+BED+BSL,    SOLID,       EBICORNERN,  SCOR_DOWNN,  SCOR_DOWNN  ,  80 },
	{ "tiles/I22.png", BSU+BSR+BSD+BEL,    SCOR_LEFTN,  SCOR_LEFTN,  EBICORNERN,  SOLID       ,  80 },
	{ "tiles/I23.png", BEU+BSR+BSD+BSL,    EBICORNERN,  SOLID,       SCOR_UPN,    SCOR_UPN    ,  80 },
	{ "tiles/I24.png", BSU+BER+BSD+BSL,    ECOR_RIGHTN, ECOR_RIGHTN, SOLID,       EBICORNERN  ,  80 },
	{ "tiles/J11.png", BE,                 SBICORNERT,  SOLID,       SCOR_UPT,    SCOR_UPT    ,  90 },
	{ "tiles/J12.png", BE,                 SCOR_RIGHTT, SCOR_RIGHTT, SOLID,       SBICORNERT  ,  90 },
	{ "tiles/J13.png", BE,                 SOLID,       SBICORNERT,  SCOR_DOWNT,  SCOR_DOWNT  ,  90 },
	{ "tiles/J14.png", BE,                 SCOR_LEFTT,  SCOR_LEFTT,  SBICORNERT,  SOLID       ,  90 },
	{ "tiles/J21.png", BS,                 EBICORNERT,  EMPTY,       ECOR_UPT,    ECOR_UPT    ,  10 },
	{ "tiles/J22.png", BS,                 ECOR_RIGHTT, ECOR_RIGHTT, EMPTY,       EBICORNERT  ,  10 },
	{ "tiles/J23.png", BS,                 EMPTY,       EBICORNERT,  ECOR_DOWNT,  ECOR_DOWNT  ,  10 },
	{ "tiles/J24.png", BS,                 ECOR_LEFTT,  ECOR_LEFTT,  EBICORNERT,  EMPTY       ,  10 },

	// Singularities
	{ "tiles/K1.png",  BE,                 EBICORNERT,  EBICORNERT,  EBICORNERT,  EBICORNERT  ,  80 },
	{ "tiles/K2.png",  BS,                 SBICORNERT,  SBICORNERT,  SBICORNERT,  SBICORNERT  ,  20 },
	{ "tiles/L11.png", BSD,                EBICORNERT,  SOLID,       OCOR_UPT,    OCOR_UPT    ,  60 },
	{ "tiles/L12.png", BSL,                OCOR_RIGHTT, OCOR_RIGHTT, SOLID,       EBICORNERT  ,  60 },
	{ "tiles/L13.png", BSU,                SOLID,       EBICORNERT,  OCOR_DOWNT,  OCOR_DOWNT  ,  60 },
	{ "tiles/L14.png", BSR,                OCOR_LEFTT,  OCOR_LEFTT,  EBICORNERT,  SOLID       ,  60 },
	{ "tiles/L21.png", BED,                SBICORNERT,  EMPTY,       HCOR_UPT,    HCOR_UPT    ,  40 },
	{ "tiles/L22.png", BEL,                HCOR_RIGHTT, HCOR_RIGHTT, EMPTY,       SBICORNERT  ,  40 },
	{ "tiles/L23.png", BEU,                EMPTY,       SBICORNERT,  HCOR_DOWNT,  HCOR_DOWNT  ,  40 },
	{ "tiles/L24.png", BER,                HCOR_LEFTT,  HCOR_LEFTT,  SBICORNERT,  EMPTY       ,  40 },
};

unsigned int TileSet::TileDataNumAllTiles = sizeof(TileSet::TileData) / sizeof(TileSet::TileConfig);

struct MapCell {
	signed int Elevation;
	unsigned char TileID;
	bool Fixed;
};

struct MapLayer {
	const TileSet * Tiles;
	signed int Elevation;
};

struct Map {
	Map(unsigned int w, unsigned int h, signed int min_elev, signed int max_elev) :
	Width(w), Height(h), CurrentLayer(NULL), MaxElevation(max_elev), MinElevation(min_elev) {
		Cells = new MapCell[h*w];
		memset(Cells, 0, h*w*sizeof(MapCell));
	}

	~Map() {
		delete[] Cells;
	}

	inline unsigned int getWidth() { return Width; }
	inline unsigned int getHeight() { return Height; }

	void GaussianBlur(float radius)
	{
		float temp[Width * Height];
		float sigma2 = radius*radius;
		unsigned int size = 7;

		for(unsigned int y = 0; y < Height; ++y) {
			for(unsigned int x = 0; x < Width; ++x) {
				float value = 0;
				float sum = 0;
				unsigned int min_i = x > size ? x - size : 0;
				unsigned int max_i = x + size < Width - 1 ? x + size : Width - 1;
				for(unsigned int i = min_i; i <= max_i; ++i) {
					float inc = i-x;
					float factor = exp(-inc*inc / (2*sigma2));
					sum += factor;
					value += factor * Cells[i + y * Width].Elevation;
				};
				temp[x + y * Width] = value / sum;
			};
		};

		for(unsigned int y = 0; y < Height; ++y) {
			for(unsigned int x = 0; x < Width; ++x) {
				float value = 0;
				float sum = 0;
				unsigned int min_i = y > size ? y - size : 0;
				unsigned int max_i = y + size < Height - 1 ? y + size : Height - 1;
				for(unsigned int i = min_i; i <= max_i; i++) {
					float inc = i-y;
					float factor = exp(-inc*inc / (2*sigma2));
					sum += factor;
					value += factor * temp[x + i * Width];
				};
				Cells[x + y * Width].Elevation = value / sum;
			};
		};
	};

	bool AdjustTiles(unsigned int iterations = 200) {
		const TileSet * Tiles = CurrentLayer->Tiles;
		bool tiles_ok[Width * Height];
		for (unsigned int k=0; k<iterations; ++k) {
			unsigned int changes = 0;
			unsigned int wrong = 0;
			unsigned int y0 = rand() % Height;
			for (unsigned int yi=0; yi<Height; ++yi) {
				unsigned int y = (y0+yi) % Height;
				unsigned int x0 = rand() % Width;
				for (unsigned int xi=0; xi<Width; ++xi) {
					unsigned int x = (x0+xi) % Width;
					int best_err = -1;
					if (!Cells[x + y*Width].Fixed) {
						unsigned char best_tile = 0;
						best_err = -1;
						int c0 = rand() % (Tiles->NumTiles());
						for (unsigned int ci = 0; ci < Tiles->NumTiles(); ++ci) {
							int c = (c0+ci) % (Tiles->NumTiles()); // Current tile
							int e = 0;  // Error

							// Checking the tile to the left
							if (x > 0) {
								int d = Cells[(x-1)+y*Width].TileID;
								e += Tiles->EdgesMatchError(Tiles->EdgeRight(d),
									Tiles->EdgeLeft(c))*3;
							} else {
								int d = Cells[(x+1)+y*Width].TileID;
								e += Tiles->EdgesMatchError(Tiles->HMirrorEdge(Tiles->EdgeLeft(d)),
									Tiles->EdgeLeft(c))*3; // Mirror
							}

							// Checking the tile to the right
							if (x < Width - 1) {
								int d = Cells[(x+1)+y*Width].TileID;
								e += Tiles->EdgesMatchError(Tiles->EdgeRight(c),
									 Tiles->EdgeLeft(d))*3;
							} else {
								int d = Cells[(x-1)+y*Width].TileID;
								e += Tiles->EdgesMatchError(Tiles->EdgeRight(c),
									Tiles->HMirrorEdge(Tiles->EdgeRight(d)))*3; // Mirror
							}

							// Checking the tile above
							if (y > 0) {
								int d = Cells[x+(y-1)*Width].TileID;
								e += Tiles->EdgesMatchError(Tiles->EdgeDown(d),
									Tiles->EdgeUp(c))*3;
							} else {
								int d = Cells[x+(y+1)*Width].TileID;
								e += Tiles->EdgesMatchError(Tiles->VMirrorEdge(Tiles->EdgeUp(d)),
									Tiles->EdgeUp(c))*3; // Mirror
							}

							// Checking the tile below
							if (y < Height - 1) {
								int d = Cells[x+(y+1)*Width].TileID;
								e += Tiles->EdgesMatchError(Tiles->EdgeDown(c),
									Tiles->EdgeUp(d))*3;
							} else {
								int d = Cells[x+(y-1)*Width].TileID;
								e += Tiles->EdgesMatchError(Tiles->EdgeDown(c),
									Tiles->VMirrorEdge(Tiles->EdgeDown(d)))*3; // Mirror
							}

							if (best_err == -1 || e < best_err) {
								best_tile = c;
								best_err = e;
							}
						} // for (unsigned int ci = 0; ci < Tiles->NumTiles(); ++ci)

						if (Cells[x+y*Width].TileID != best_tile) ++changes;
						Cells[x+y*Width].TileID = best_tile;
						if (best_err) {
							tiles_ok[x+y*Width] = false;
							++wrong;
						} else {
							tiles_ok[x+y*Width] = true;
						}
					} // if (!Cells[x + y*Width].Fixed)
				} // for (unsigned int xi=0; xi<Width; ++xi)
			} // for (unsigned int yi=0; yi<Height; ++yi)
			printf("Iter=%d, Changes= %d, Wrong=%d\n", k, changes, wrong);
			if (wrong && !changes) {
				for (unsigned int y=0; y<Height; ++y) {
					for (unsigned int x=0; x<Width; ++x) {
						if (!tiles_ok[x+y*Width] && !Cells[x+y*Width].Fixed) {
							if (Cells[x+y*Width].Elevation >= 0) {
								Cells[x+y*Width].TileID = Tiles->SolidTile();
							} else {
								Cells[x+y*Width].TileID = Tiles->EmptyTile();
							}
						}
					}
				}
			}
			if (!changes && !wrong) return true; // No wrong tiles
		} // for (unsigned int k=0; k<iterations; ++k
		return false; // We still have wrong tiles, but we give up
	}

	void SetupInitialTiles() {
		const TileSet * Tiles = CurrentLayer->Tiles;
		for (unsigned int y=0; y<Height; ++y) {
			for (unsigned int x=0; x<Width; ++x) {
				if (!Cells[x + y*Width].Fixed) {

					unsigned int xm = x > 0        ? x - 1 : x;
					unsigned int xp = x < Width-1  ? x + 1 : x;
					unsigned int ym = y > 0        ? y - 1 : y;
					unsigned int yp = y < Height-1 ? y + 1 : y;

					bool c =  (Cells[ x  + y  *Width].Elevation >= 0);
					bool l =  (Cells[ xm + y  *Width].Elevation >= 0);
					bool r =  (Cells[ xp + y  *Width].Elevation >= 0);
					bool u =  (Cells[ x  + yp *Width].Elevation >= 0);
					bool d =  (Cells[ x  + ym *Width].Elevation >= 0);
					bool ul = (Cells[ xm + yp *Width].Elevation >= 0);
					bool ur = (Cells[ xp + yp *Width].Elevation >= 0);
					bool dl = (Cells[ xm + ym *Width].Elevation >= 0);
					bool dr = (Cells[ xp + ym *Width].Elevation >= 0);

					if (!u && !d && !l && !r) c = false;
					if (u && d && l && r) c = true;

					Cells[x+y*Width].TileID = Tiles->SolidTile();
					Cells[x+y*Width].Fixed = false;
					if (c & u & d & l & r) {
						Cells[x+y*Width].TileID = Tiles->SolidTile();
						//if (ul & ur & dl & dr) {
						//	Cells[x+y*Width].Fixed = true;
						//}
					} else if (!c & !u & !d & !l & !r) {
						Cells[x+y*Width].TileID = Tiles->EmptyTile();
						//if (!ul & !ur & !dl & !dr) {
						//	Cells[x+y*Width].Fixed = true;
						//}
					} else {
						uint32_t env =
							(ul?0x100:0)+( u?0x080:0)+(ur?0x040:0)+
							( l?0x020:0)+( c?0x010:0)+( r?0x008:0)+
							(dl?0x004:0)+( d?0x002:0)+(dr?0x001:0);
						Cells[x+y*Width].TileID = Tiles->InitialTileGuess(env);
					}
				} // if (!Cells[x + y*Width].Fixed)
			} // for (unsigned int x=0; x<Width; ++x)
		} // for (unsigned int y=0; y<Height; ++y)
	}

	void Random() {
		for (unsigned int y=0; y<Height; ++y) {
			for (unsigned int x=0; x<Width; ++x) {
				Cells[x+y*Width].Elevation = MinElevation + (rand() % (MaxElevation - MinElevation));
				Cells[x+y*Width].Fixed = false;
			}
		}

		GaussianBlur(5);

		for(unsigned int y = 0; y < Height; ++y) {
			for(unsigned int x = 0; x < Width; ++x) {
				printf("%3d ", Cells[x+y*Width].Elevation);
			}
			printf("\n");
		}
		printf("\n");
	}

	bool AddTilesInLayer()
	{
		for (unsigned int tries = 0 ; tries < 1; ++tries) {
			SetupInitialTiles();
			if (AdjustTiles()) return true;
		}
		return false;
	}

	inline void SetCurrentLayer(MapLayer & layer) {
		CurrentLayer = &layer;
	}

	unsigned int Width;
	unsigned int Height;
	const MapLayer * CurrentLayer;
	MapCell *Cells;
	signed int MaxElevation;
	signed int MinElevation;
};

int main()
{
	srand((unsigned)time(0));

	TileSet tiles;
	Map map(32, 24, -100, 100);
	map.Random();

	MapLayer layer = {&tiles , -50};
	map.SetCurrentLayer(layer);
	map.AddTilesInLayer();

	// Create the main rendering window
	sf::RenderWindow App(sf::VideoMode(1024, 768, 32), "SFML TileMap");

	// Load the sprite images and create the sprited
	sf::Image tiles_images[tiles.NumTiles()];
	sf::Sprite tiles_sprites[tiles.NumTiles()];
	for (unsigned int i=0; i<tiles.NumTiles();++i) {
		//printf("Loading '%s'\n", tiles[i].FileName);
		if (!tiles_images[i].LoadFromFile(tiles.FileName(i))) {
			return EXIT_FAILURE;
		}
		tiles_images[i].SetSmooth(false);
		tiles_sprites[i] = sf::Sprite(tiles_images[i]);
	}

	// Start game loop
	while (App.IsOpened()) {
		// Process events
		sf::Event Event;
		while (App.GetEvent(Event)) {
			// Close window : exit
			if (Event.Type == sf::Event::Closed)
			App.Close();
		}

		// Clear screen
		App.Clear();

		for (unsigned int y=0; y<map.getHeight(); ++y) {
			for (unsigned int x=0; x<map.getWidth(); ++x) {
				int tile_id = map.Cells[x+y*map.getWidth()].TileID;
				// Get the tile's image
				const sf::Image* image = tiles_sprites[tile_id].GetImage();
				// Get the width and height of the image
				int width = image->GetWidth();
				int height = image->GetHeight();
				// Adjust the offset by using the width
				tiles_sprites[tile_id].SetPosition(x * width, y * height);
				// Draw the tile
				App.Draw(tiles_sprites[tile_id]);
			}
		}

		// Display window contents on screen
		App.Display();

		sf::Sleep(1.0f / 60.0f);
	}

	return EXIT_SUCCESS;
}

