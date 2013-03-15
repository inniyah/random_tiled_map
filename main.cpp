#include <sys/types.h>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cmath>

enum {
	BS = 1 << 0, BSU = 1 << 1, BSD = 1 << 2, BSL = 1 << 3, BSR = 1 << 4,
	BE = 1 << 5, BEU = 1 << 6, BED = 1 << 7, BEL = 1 << 8, BER = 1 << 9,
};

enum {
	EMPTY, SOLID,
	HALF_UP, HALF_DOWN, HALF_LEFT, HALF_RIGHT,
	SYMMETRIC_EDGES_MAX = HALF_RIGHT,

	ECOR_UPT = ((SYMMETRIC_EDGES_MAX + 1) & 254),
	ECOR_UPN,    SCOR_UPT,    SCOR_UPN,
	ECOR_DOWNT,  ECOR_DOWNN,  SCOR_DOWNT,  SCOR_DOWNN,
	ECOR_LEFTT,  ECOR_LEFTN,  SCOR_LEFTT,  SCOR_LEFTN,
	ECOR_RIGHTT, ECOR_RIGHTN, SCOR_RIGHTT, SCOR_RIGHTN,
	COMPLEMENTARY_EDGES_MAX = SCOR_RIGHTN,
};

int HMirrorEdge(int edge) {
	return edge;
}

int VMirrorEdge(int edge) {
	return edge;
}

int EdgesMatchError(int e1, int e2) {
	if (e1 <= SYMMETRIC_EDGES_MAX) {
		if (e1 == e2)
			return 0;
		return 100;
	}

	if (e1 <= COMPLEMENTARY_EDGES_MAX && e2 <= COMPLEMENTARY_EDGES_MAX) {
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

const TileConfig tiles[] = {
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
/*
	{ "tiles/G11.png", BEU+BER+BEL,         },
	{ "tiles/G12.png", BEU+BER+BEL,         },
	{ "tiles/G13.png", BER+BED+BEU,         },
	{ "tiles/G14.png", BER+BED+BEU,         },
	{ "tiles/G15.png", BED+BEL+BER,         },
	{ "tiles/G16.png", BED+BEL+BER,         },
	{ "tiles/G17.png", BEL+BEU+BED,         },
	{ "tiles/G18.png", BEL+BEU+BED,         },
	{ "tiles/G21.png", BSU+BSR+BSL,         },
	{ "tiles/G22.png", BSU+BSR+BSL,         },
	{ "tiles/G23.png", BSR+BSD+BSU,         },
	{ "tiles/G24.png", BSR+BSD+BSU,         },
	{ "tiles/G25.png", BSD+BSL+BSR,         },
	{ "tiles/G26.png", BSD+BSL+BSR,         },
	{ "tiles/G27.png", BSL+BSU+BSD,         },
	{ "tiles/G28.png", BSL+BSU+BSD,         },
	{ "tiles/H11.png", BER,                 },
	{ "tiles/H12.png", BEL,                 },
	{ "tiles/H13.png", BED,                 },
	{ "tiles/H14.png", BEU,                 },
	{ "tiles/H15.png", BEL,                 },
	{ "tiles/H16.png", BER,                 },
	{ "tiles/H17.png", BEU,                 },
	{ "tiles/H18.png", BED,                 },
	{ "tiles/H21.png", BSR,                 },
	{ "tiles/H22.png", BSL,                 },
	{ "tiles/H23.png", BSD,                 },
	{ "tiles/H24.png", BSU,                 },
	{ "tiles/H25.png", BSL,                 },
	{ "tiles/H26.png", BSR,                 },
	{ "tiles/H27.png", BSU,                 },
	{ "tiles/H28.png", BSD,                 },

	{ "tiles/I11.png", BEU+BER+BSD+BEL,     },
	{ "tiles/I12.png", BEU+BER+BED+BSL,     },
	{ "tiles/I13.png", BSU+BER+BED+BEL,     },
	{ "tiles/I14.png", BEU+BSR+BED+BEL,     },
	{ "tiles/I21.png", BSU+BSR+BED+BSL,     },
	{ "tiles/I22.png", BSU+BSR+BSD+BEL,     },
	{ "tiles/I23.png", BEU+BSR+BSD+BSL,     },
	{ "tiles/I24.png", BSU+BER+BSD+BSL,     },
	{ "tiles/J11.png", BE,                  },
	{ "tiles/J12.png", BE,                  },
	{ "tiles/J13.png", BE,                  },
	{ "tiles/J14.png", BE,                  },
	{ "tiles/J21.png", BS,                  },
	{ "tiles/J22.png", BS,                  },
	{ "tiles/J23.png", BS,                  },
	{ "tiles/J24.png", BS,                  },
	{ "tiles/K1.png",  BE,                  },
	{ "tiles/K2.png",  BS,                  },
	{ "tiles/L11.png", BSD,                 },
	{ "tiles/L12.png", BSL,                 },
	{ "tiles/L13.png", BSU,                 },
	{ "tiles/L14.png", BSR,                 },
	{ "tiles/L21.png", BED,                 },
	{ "tiles/L22.png", BEL,                 },
	{ "tiles/L23.png", BEU,                 },
	{ "tiles/L24.png", BER,                 },
*/
};

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

	NUM_TILES = sizeof(tiles) / sizeof(TileConfig)
};

struct MapCell {
	signed int Elevation;
	unsigned char TileID;
	bool Fixed;
};

struct Map {
	Map(unsigned int w, unsigned int h, const TileConfig * tiles) :
	Width(w), Height(h), Tiles(tiles), MaxElevation(100), MinElevation(-100) {
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

	bool AdjustTiles(unsigned int iterations = 100) {
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
						int c0 = rand() % NUM_TILES;
						for (int ci = 0; ci < NUM_TILES; ++ci) {
							int c = (c0+ci) % NUM_TILES; // Current tile
							int e = 0;  // Error

							if (x > 0) {
								int d = Cells[(x-1)+y*Width].TileID;
								e += EdgesMatchError(Tiles[d].EdgeRight,
									Tiles[c].EdgeLeft)*3;
							} else {
								int d = Cells[(x+1)+y*Width].TileID;
								e += EdgesMatchError(HMirrorEdge(Tiles[d].EdgeLeft),
									Tiles[c].EdgeLeft)*3; // Mirror
							}

							if (x < Width - 1) {
								int d = Cells[(x+1)+y*Width].TileID;
								e += EdgesMatchError(Tiles[c].EdgeRight,
									 Tiles[d].EdgeLeft)*3;
							} else {
								int d = Cells[(x-1)+y*Width].TileID;
								e += EdgesMatchError(Tiles[c].EdgeRight,
									HMirrorEdge(Tiles[d].EdgeRight))*3; // Mirror
							}

							if (y > 0) {
								int d = Cells[x+(y-1)*Width].TileID;
								e += EdgesMatchError(Tiles[d].EdgeDown,
									Tiles[c].EdgeUp)*3;
							} else {
								int d = Cells[x+(y+1)*Width].TileID;
								e += EdgesMatchError(VMirrorEdge(Tiles[d].EdgeUp),
									Tiles[c].EdgeUp)*3; // Mirror
							}

							if (y < Height - 1) {
								int d = Cells[x+(y+1)*Width].TileID;
								e += EdgesMatchError(Tiles[c].EdgeDown,
									Tiles[d].EdgeUp)*3;
							} else {
								int d = Cells[x+(y-1)*Width].TileID;
								e += EdgesMatchError(Tiles[c].EdgeDown,
									VMirrorEdge(Tiles[d].EdgeDown))*3; // Mirror
							}

							if (best_err == -1 || e < best_err) {
								best_tile = c;
								best_err = e;
							}
						}
						if (Cells[x+y*Width].TileID != best_tile) ++changes;
						Cells[x+y*Width].TileID = best_tile;
						if (best_err) {
							tiles_ok[x+y*Width] = false;
							++wrong;
						} else {
							tiles_ok[x+y*Width] = true;
						}
					}
				}
			}
			printf("Iter=%d, Changes= %d, Wrong=%d\n", k, changes, wrong);
			if (!changes && wrong) {
				for (unsigned int y=0; y<Height; ++y) {
					for (unsigned int x=0; x<Width; ++x) {
						if (!tiles_ok[x+y*Width] && !Cells[x+y*Width].Fixed) {
							if (Cells[x+y*Width].Elevation >= 0) {
								Cells[x+y*Width].TileID = TILE_SOLID;
							} else {
								Cells[x+y*Width].TileID = TILE_EMPTY;
							}
						}
					}
				}
			}
			if (!changes && !wrong) return true; // No wrong tiles
		}
		return false; // We still have wrong tiles, but we give up
	}

	void SetupTiles() {
		for (unsigned int y=0; y<Height; ++y) {
			for (unsigned int x=0; x<Width; ++x) {

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

				Cells[x+y*Width].TileID = TILE_SOLID;
				Cells[x+y*Width].Fixed = false;
				if (c & u & d & l & r) {
					Cells[x+y*Width].TileID = TILE_SOLID;
					Cells[x+y*Width].Fixed = true;
				} else if (!c & !u & !d & !l & !r) {
					Cells[x+y*Width].TileID = TILE_EMPTY;
					Cells[x+y*Width].Fixed = false;
				} else {
					uint32_t env =
						(ul?0x100:0)+( u?0x080:0)+(ur?0x040:0)+
						( l?0x020:0)+( c?0x010:0)+( r?0x008:0)+
						(dl?0x004:0)+( d?0x002:0)+(dr?0x001:0);
					switch (env) {
						case 0b000000111: Cells[x+y*Width].TileID = TILE_HALF_U; break;
						case 0b111000000: Cells[x+y*Width].TileID = TILE_HALF_D; break;
						case 0b001001001: Cells[x+y*Width].TileID = TILE_HALF_L; break;
						case 0b100100100: Cells[x+y*Width].TileID = TILE_HALF_R; break;
						case 0b000001011: Cells[x+y*Width].TileID = TILE_HALF_UL; break;
						case 0b000100110: Cells[x+y*Width].TileID = TILE_HALF_UR; break;
						case 0b011001000: Cells[x+y*Width].TileID = TILE_HALF_DL; break;
						case 0b110100000: Cells[x+y*Width].TileID = TILE_HALF_DR; break;
						case 0b000000001: Cells[x+y*Width].TileID = TILE_ECOR_UL; break;
						case 0b000000100: Cells[x+y*Width].TileID = TILE_ECOR_UR; break;
						case 0b001000000: Cells[x+y*Width].TileID = TILE_ECOR_DL; break;
						case 0b100000000: Cells[x+y*Width].TileID = TILE_ECOR_DR; break;
						case 0b111100100: Cells[x+y*Width].TileID = TILE_SCOR_UL; break;
						case 0b111001001: Cells[x+y*Width].TileID = TILE_SCOR_UR; break;
						case 0b100100111: Cells[x+y*Width].TileID = TILE_SCOR_DL; break;
						case 0b001001111: Cells[x+y*Width].TileID = TILE_SCOR_DR; break;
					}
				}
			}
		}
	}

	bool Random() {
		for (unsigned int y=0; y<Height; ++y) {
			for (unsigned int x=0; x<Width; ++x) {
				Cells[x+y*Width].Elevation = MinElevation + (rand() % (MaxElevation - MinElevation));
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

		SetupTiles();
		return AdjustTiles();
	}

	unsigned int Width;
	unsigned int Height;
	const TileConfig *Tiles;
	MapCell *Cells;
	signed int MaxElevation;
	signed int MinElevation;
};

int main()
{
	srand((unsigned)time(0));

	Map map(25, 20, tiles);
	map.Random();

	// Create the main rendering window
	sf::RenderWindow App(sf::VideoMode(1024, 768, 32), "SFML TileMap");

	// Load the sprite images and create the sprited
	sf::Image tiles_images[NUM_TILES];
	sf::Sprite tiles_sprites[NUM_TILES];
	for (int i=0; i<NUM_TILES;++i) {
		printf("Loading '%s'\n", tiles[i].FileName);
		if (!tiles_images[i].LoadFromFile(tiles[i].FileName)) {
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

		// Get elapsed time
		float ElapsedTime = App.GetFrameTime();

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

