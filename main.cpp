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
	ECOR_UP, ECOR_DOWN, ECOR_LEFT, ECOR_RIGHT,
	SCOR_UP, SCOR_DOWN, SCOR_LEFT, SCOR_RIGHT,
};

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
	{ "tiles/A1.png",  BE+BEU+BED+BER+BEL, EMPTY,      EMPTY,      EMPTY,      EMPTY      ,   0 },
	{ "tiles/A2.png",  BS+BSU+BSD+BSR+BSL, SOLID,      SOLID,      SOLID,      SOLID      , 100 },
	{ "tiles/B1.png",  BEU+BSD,            EMPTY,      SOLID,      HALF_DOWN,  HALF_DOWN  ,  50 },
	{ "tiles/B2.png",  BER+BSL,            HALF_LEFT,  HALF_LEFT,  SOLID,      EMPTY      ,  50 },
	{ "tiles/B3.png",  BED+BSU,            SOLID,      EMPTY,      HALF_UP,    HALF_UP    ,  50 },
	{ "tiles/B4.png",  BEL+BSR,            HALF_RIGHT, HALF_RIGHT, EMPTY,      SOLID      ,  50 },
	{ "tiles/C11.png", BEU+BEL,            EMPTY,      HALF_RIGHT, EMPTY,      HALF_DOWN  ,  25 },
	{ "tiles/C12.png", BEU+BER,            EMPTY,      HALF_LEFT,  HALF_DOWN,  EMPTY      ,  25 },
	{ "tiles/C13.png", BED+BER,            HALF_LEFT,  EMPTY,      HALF_UP,    EMPTY      ,  25 },
	{ "tiles/C14.png", BED+BEL,            HALF_RIGHT, EMPTY,      EMPTY,      HALF_UP    ,  25 },
	{ "tiles/C21.png", BSU+BSL,            SOLID,      HALF_LEFT,  SOLID,      HALF_UP    ,  75 },
	{ "tiles/C22.png", BSU+BSR,            SOLID,      HALF_RIGHT, HALF_UP,    SOLID      ,  75 },
	{ "tiles/C23.png", BSD+BSR,            HALF_RIGHT, SOLID,      HALF_DOWN,  SOLID      ,  75 },
	{ "tiles/C24.png", BSD+BSL,            HALF_LEFT,  SOLID,      SOLID,      HALF_DOWN  ,  75 },
};

enum { TILE_EMPTY=0, TILE_SOLID=1, NUM_TILES = sizeof(tiles) / sizeof(TileConfig) };

bool doTilesMatchVert(const TileConfig & up, const TileConfig & down) {
	if (up.EdgeDown == down.EdgeUp) return true;
	return false;
}

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

	void AdjustTiles(unsigned int iterations = 100) {
		bool tiles_ok[Width * Height];
		for (unsigned int k=0; k<iterations; ++k) {
			unsigned int changes = 0;
			unsigned int wrong = 0;
			for (unsigned int y=0; y<Height; ++y) {
				for (unsigned int x=0; x<Width; ++x) {
					int best_err = -1;
					if (!Cells[x + y*Width].Fixed) {
						unsigned char best_tile = 0;
						best_err = -1;
						int r = rand() % NUM_TILES;
						for (int t = 0; t < NUM_TILES; ++t) {
							int c = (r+t) % NUM_TILES; // Current tile
							int e = 0;  // Error
							uint32_t w = 0;

							if (x > 0) {
								int d = Cells[(x-1)+y*Width].TileID;
								if (Tiles[d].EdgeRight != Tiles[c].EdgeLeft)
									e += 300;
							} else {
								int d = Cells[(x+1)+y*Width].TileID;
								if (Tiles[d].EdgeLeft != Tiles[c].EdgeLeft) // Mirror
									e += 300;
							}

							if (x < Width - 1) {
								int d = Cells[(x+1)+y*Width].TileID;
								if (Tiles[c].EdgeRight != Tiles[d].EdgeLeft)
									e += 300;
							} else {
								int d = Cells[(x-1)+y*Width].TileID;
								if (Tiles[c].EdgeRight != Tiles[d].EdgeRight) // Mirror
									e += 300;
							}

							if (y > 0) {
								int d = Cells[x+(y-1)*Width].TileID;
								if (Tiles[d].EdgeDown != Tiles[c].EdgeUp)
									e += 300;
							} else {
								int d = Cells[x+(y+1)*Width].TileID;
								if (Tiles[d].EdgeUp != Tiles[c].EdgeUp) // Mirror
									e += 300;
							}

							if (y < Height - 1) {
								int d = Cells[x+(y+1)*Width].TileID;
								if (Tiles[c].EdgeDown != Tiles[d].EdgeUp)
									e += 300;
							} else {
								int d = Cells[x+(y-1)*Width].TileID;
								if (Tiles[c].EdgeDown != Tiles[d].EdgeDown) // Mirror
									e += 300;
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
			if (!changes && !wrong) return;
		}
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

				if (!u && !d && !l && !r) c = false;
				if (u && d && l && r) c = true;

				Cells[x+y*Width].Fixed = false;
				if (c & u & d & l & r) {
					Cells[x+y*Width].TileID = TILE_SOLID;
					Cells[x+y*Width].Fixed = true;
				} else if (!c & !u & !d & !l & !r) {
					Cells[x+y*Width].TileID = TILE_EMPTY;
					Cells[x+y*Width].Fixed = false;
				} else {
					Cells[x+y*Width].TileID = TILE_SOLID;
				}
			}
		}
	}

	void Random() {
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
		AdjustTiles();
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

