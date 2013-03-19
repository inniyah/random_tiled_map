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

#include "tileset.h"

#include <sys/types.h>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <climits>
#include <iostream>

#define VERY_HIGH INT_MAX
#define VERY_LOW INT_MIN

struct MapCell {
	signed int Elevation;
	unsigned char TileID;
	TileSet::TileRuntime * TileRuntimeData;
	struct {
		bool FixedTile : 1;
		bool Ignore : 1;
		bool GrowUp : 1;
		bool GrowDown : 1;
	};
};

struct MapLayer {
	TileSet * Tiles;
	signed int Elevation;
};

struct Map {
	Map(unsigned int w, unsigned int h, signed int min_elev, signed int max_elev) :
	Width(w), Height(h), Layers(NULL), MaxElevation(max_elev), MinElevation(min_elev) {
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

	bool AdjustTiles(unsigned int iterations = 250) {
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
					if (!Cells[x + y*Width].FixedTile && !Cells[x + y*Width].Ignore) {
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
							if (rand() % 100 <= 5) {
								if (Cells[x+y*Width].Elevation >= CurrentLayer->Elevation) {
									Cells[x+y*Width].TileID = Tiles->SolidTile();
								} else {
									Cells[x+y*Width].TileID = Tiles->EmptyTile();
								}
							}
						} else {
							tiles_ok[x+y*Width] = true;
						}
					} // if (!Cells[x + y*Width].FixedTile && !Cells[x + y*Width].Ignore)
				} // for (unsigned int xi=0; xi<Width; ++xi)
			} // for (unsigned int yi=0; yi<Height; ++yi)
			printf("Iter=%d, Changes= %d, Wrong=%d\n", k, changes, wrong);
			if (wrong && !changes) {
				for (unsigned int y=0; y<Height; ++y) {
					for (unsigned int x=0; x<Width; ++x) {
						if (!tiles_ok[x+y*Width] && !Cells[x+y*Width].FixedTile && !Cells[x+y*Width].Ignore) {
							if (Cells[x+y*Width].Elevation >= CurrentLayer->Elevation) {
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
				if (!Cells[x + y*Width].FixedTile && !Cells[x + y*Width].Ignore) {
	
					unsigned int xm = x > 0        ? x - 1 : x;
					unsigned int xp = x < Width-1  ? x + 1 : x;
					unsigned int ym = y > 0        ? y - 1 : y;
					unsigned int yp = y < Height-1 ? y + 1 : y;

					bool c =  (Cells[ x  + y  *Width].Elevation >= CurrentLayer->Elevation);
					bool l =  (Cells[ xm + y  *Width].Elevation >= CurrentLayer->Elevation);
					bool r =  (Cells[ xp + y  *Width].Elevation >= CurrentLayer->Elevation);
					bool u =  (Cells[ x  + yp *Width].Elevation >= CurrentLayer->Elevation);
					bool d =  (Cells[ x  + ym *Width].Elevation >= CurrentLayer->Elevation);
					bool ul = (Cells[ xm + yp *Width].Elevation >= CurrentLayer->Elevation);
					bool ur = (Cells[ xp + yp *Width].Elevation >= CurrentLayer->Elevation);
					bool dl = (Cells[ xm + ym *Width].Elevation >= CurrentLayer->Elevation);
					bool dr = (Cells[ xp + ym *Width].Elevation >= CurrentLayer->Elevation);

					if (!u && !d && !l && !r) c = false;
					if (u && d && l && r) c = true;

					Cells[x+y*Width].TileID = Tiles->SolidTile();
					//Cells[x+y*Width].FixedTile = false;
					//Cells[x+y*Width].Ignore = false;

					if (c & u & d & l & r) {
						Cells[x+y*Width].TileID = Tiles->SolidTile();
						//if (ul & ur & dl & dr) {
						//	Cells[x+y*Width].FixedTile = true;
						//}
					} else if (!c & !u & !d & !l & !r) {
						Cells[x+y*Width].TileID = Tiles->EmptyTile();
						//if (!ul & !ur & !dl & !dr) {
						//	Cells[x+y*Width].FixedTile = true;
						//}
					} else {
						uint32_t env =
							(ul?0x100:0)+( u?0x080:0)+(ur?0x040:0)+
							( l?0x020:0)+( c?0x010:0)+( r?0x008:0)+
							(dl?0x004:0)+( d?0x002:0)+(dr?0x001:0);
						Cells[x+y*Width].TileID = Tiles->InitialTileGuess(env);
					}
				} // if (!Cells[x + y*Width].FixedTile && !Cells[x + y*Width].Ignore)
			} // for (unsigned int x=0; x<Width; ++x)
		} // for (unsigned int y=0; y<Height; ++y)
	}

	void Random() {
		memset(Cells, 0, Height*Width*sizeof(MapCell));

		for (unsigned int y=0; y<Height; ++y) {
			for (unsigned int x=0; x<Width; ++x) {
				Cells[x+y*Width].Elevation = MinElevation + (rand() % (MaxElevation - MinElevation));
				Cells[x+y*Width].FixedTile = false;
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

	void AddTiles()
	{
		// Central layer
		CurrentLayer = StartingLayer;
		TileSet * tiles = CurrentLayer->Tiles;
		unsigned int empty_tile = tiles->EmptyTile();
		unsigned int solid_tile = tiles->SolidTile();

		for(unsigned int y = 0; y < Height; ++y) {
			for(unsigned int x = 0; x < Width; ++x) {
				Cells[x+y*Width].FixedTile = false;
				Cells[x+y*Width].Ignore = false;
				Cells[x+y*Width].TileRuntimeData = &tiles->GetTileRuntimeData(empty_tile);
			}
		}

		for (unsigned int tries = 0 ; tries < 2; ++tries) {
			SetupInitialTiles();
			if (AdjustTiles()) break;
		}

		for(unsigned int y = 0; y < Height; ++y) {
			for(unsigned int x = 0; x < Width; ++x) {
				unsigned int tile_id = Cells[x+y*Width].TileID;
				Cells[x+y*Width].TileRuntimeData = &tiles->GetTileRuntimeData(tile_id);
				if (tile_id == solid_tile) Cells[x+y*Width].GrowUp = true;
				else if (tile_id == empty_tile) Cells[x+y*Width].GrowDown = true;
			}
		}

		// Upper layer
		CurrentLayer = StartingLayer + 1;
		tiles = CurrentLayer->Tiles;
		if (tiles != NULL) {
			empty_tile = tiles->EmptyTile();
			solid_tile = tiles->SolidTile();

			for(unsigned int y = 0; y < Height; ++y) {
				for(unsigned int x = 0; x < Width; ++x) {
					if (!Cells[x+y*Width].FixedTile) {
						Cells[x+y*Width].TileID = empty_tile;
						if (Cells[x+y*Width].GrowUp) {
							Cells[x+y*Width].Ignore = false;
						} else {
							Cells[x+y*Width].Ignore = true;
						}
					}
				}
			}

			for (unsigned int tries = 0 ; tries < 2; ++tries) {
				SetupInitialTiles();
				if (AdjustTiles()) break;
			}

			for(unsigned int y = 0; y < Height; ++y) {
				for(unsigned int x = 0; x < Width; ++x) {
					if (Cells[x+y*Width].GrowUp) {
						unsigned int tile_id = Cells[x+y*Width].TileID;
						Cells[x+y*Width].TileRuntimeData = &tiles->GetTileRuntimeData(tile_id);
						if (tile_id == solid_tile) Cells[x+y*Width].GrowUp = true;
						else if (tile_id == empty_tile) Cells[x+y*Width].GrowUp = false;
					}
				}
			}

		}

		// Lower layer
		CurrentLayer = StartingLayer - 1;
		tiles = CurrentLayer->Tiles;
		if (tiles != NULL) {
			empty_tile = tiles->EmptyTile();
			solid_tile = tiles->SolidTile();

			for(unsigned int y = 0; y < Height; ++y) {
				for(unsigned int x = 0; x < Width; ++x) {
					if (!Cells[x+y*Width].FixedTile) {
						Cells[x+y*Width].TileID = solid_tile;
						if (Cells[x+y*Width].GrowDown) {
							Cells[x+y*Width].Ignore = false;
						} else {
							Cells[x+y*Width].Ignore = true;
						}
					}
				}
			}

			for (unsigned int tries = 0 ; tries < 2; ++tries) {
				SetupInitialTiles();
				if (AdjustTiles()) break;
			}

			for(unsigned int y = 0; y < Height; ++y) {
				for(unsigned int x = 0; x < Width; ++x) {
					if (Cells[x+y*Width].GrowDown) {
						unsigned int tile_id = Cells[x+y*Width].TileID;
						Cells[x+y*Width].TileRuntimeData = &tiles->GetTileRuntimeData(tile_id);
						if (tile_id == solid_tile) Cells[x+y*Width].GrowUp = true;
						else if (tile_id == empty_tile) Cells[x+y*Width].GrowUp = false;
					}
				}
			}

		}

	}

	inline void SetLayers(MapLayer layers[]) {
		Layers = layers;
		StartingLayer = &Layers[0];
	}

	inline void SetStartingLayer(int index) {
		StartingLayer = &Layers[index];
	}

	inline MapLayer * GetCurrentLayer() {
		return CurrentLayer;
	}

	unsigned int Width;
	unsigned int Height;
	MapLayer * Layers;
	MapLayer * StartingLayer;
	MapLayer * CurrentLayer;
	MapCell *Cells;
	signed int MaxElevation;
	signed int MinElevation;
};

int main()
{
	srand((unsigned)time(0));

	TileSet tiles1;
	if (!tiles1.LoadTileImages("tiles/1"))
		return EXIT_FAILURE;

	TileSet tiles2;
	if (!tiles2.LoadTileImages("tiles/2"))
		return EXIT_FAILURE;

	TileSet tiles3;
	if (!tiles3.LoadTileImages("tiles/3"))
		return EXIT_FAILURE;

	MapLayer layers[] = { { NULL , VERY_LOW }, { &tiles1 , -4 }, { &tiles2 , 0 }, { &tiles3 , 8 }, { NULL , VERY_HIGH } };

	Map map(32, 24, -100, 100);
	map.SetLayers(layers);

	map.Random();
	map.SetStartingLayer(2);
	map.AddTiles();

	// Create the main rendering window
	sf::RenderWindow app(sf::VideoMode(1024, 768, 32), "SFML TileMap");

	// Get a reference to the input manager associated to our window
	//const sf::Input& input = app.GetInput();

	signed int OffsetX = 0;
	signed int OffsetY = 0;

	// Start game loop
	while (app.IsOpened()) {
		sf::Event event;
		while (app.GetEvent(event))
		{ // http://www.sfml-dev.org/tutorials/1.6/window-events.php

			if (event.Type == sf::Event::Closed) { // Exit when the window is closed
				app.Close();
			}

			if (event.Type == sf::Event::Resized) {
				//std::cout << "new width: " << event.Size.Width << std::endl;
				//std::cout << "new height: " << event.Size.Height << std::endl;
			}

			if (event.Type == sf::Event::LostFocus) {
				//std::cout << "lost focus" << std::endl;
			}

			if (event.Type == sf::Event::GainedFocus) {
				//std::cout << "gained focus" << std::endl;
			}

			if (event.Type == sf::Event::TextEntered) {
				if (event.Text.Unicode < 128) {
					//std::cout << "ASCII character typed: " << static_cast<char>(event.Text.Unicode) << std::endl;
				}
			}

			if (event.Type == sf::Event::KeyPressed) {
				if (event.Key.Code == sf::Key::Escape) {
					//std::cout << "the escape key was pressed" << std::endl;
					//std::cout << "control:" << event.Key.Control << std::endl;
					//std::cout << "alt:" << event.Key.Alt << std::endl;
					//std::cout << "shift:" << event.Key.Shift << std::endl;
					////std::cout << "system:" << event.Key.System << std::endl;
				}
			}

			if (event.Type == sf::Event::MouseWheelMoved) {
				//std::cout << "wheel movement: " << event.MouseWheel.Delta << std::endl;
				////std::cout << "mouse x: " << event.MouseWheel.X << std::endl;
				////std::cout << "mouse y: " << event.MouseWheel.Y << std::endl;
			}

			if (event.Type == sf::Event::MouseButtonPressed) {
				if (event.MouseButton.Button == sf::Mouse::Right) {
					//std::cout << "the right button was pressed" << std::endl;
					//std::cout << "mouse x: " << event.MouseButton.X << std::endl;
					//std::cout << "mouse y: " << event.MouseButton.Y << std::endl;
				}
			}

			if (event.Type == sf::Event::MouseMoved) {
				//std::cout << "new mouse x: " << event.MouseMove.X << std::endl;
				//std::cout << "new mouse y: " << event.MouseMove.Y << std::endl;
			}

			if (event.Type == sf::Event::MouseEntered) {
				//std::cout << "the mouse cursor has entered the window" << std::endl;
			}

			if (event.Type == sf::Event::MouseLeft) {
				//std::cout << "the mouse cursor has left the window" << std::endl;
			}

			if (event.Type == sf::Event::JoyMoved) {
				if (event.JoyMove.Axis == sf::Joy::AxisX) {
					//std::cout << "X axis moved!" << std::endl;
					//std::cout << "joystick id: " << event.JoyMove.JoystickId << std::endl;
					//std::cout << "new position: " << event.JoyMove.Position << std::endl;
				}
			}

		}
/*
		bool LeftKeyDown  = input.IsKeyDown(sf::Key::Left);
		bool RightKeyDown = input.IsKeyDown(sf::Key::Right);
		bool UpKeyDown    = input.IsKeyDown(sf::Key::Up);
		bool DownKeyDown  = input.IsKeyDown(sf::Key::Down);

		if (LeftKeyDown) OffsetX -= 8;
		if (RightKeyDown) OffsetX += 8;
		if (UpKeyDown) OffsetY -= 8;
		if (DownKeyDown) OffsetY += 8;
*/
		// Clear screen
		app.Clear();

		for (unsigned int y=0; y<map.getHeight(); ++y) {
			for (unsigned int x=0; x<map.getWidth(); ++x) {
				// Get the tile's image and sprite
				sf::Sprite & sprite = map.Cells[x+y*map.getWidth()].TileRuntimeData->Sprite;
				const sf::Image & image = *sprite.GetImage();
				// Get the width and height of the image
				int width = image.GetWidth();
				int height = image.GetHeight();
				// Adjust the offset by using the width
				sprite.SetPosition(OffsetX + x * width, OffsetY + y * height);
				// Draw the tile
				app.Draw(sprite);
			}
		}

		// Display window contents on screen
		app.Display();

//		sf::Sleep(1.0f / 60.0f);
	}

	return EXIT_SUCCESS;
}

