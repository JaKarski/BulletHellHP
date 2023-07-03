#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<string.h>

extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}

#define NORTH 'N'
#define SOUTH 'S'
#define EAST 'E'
#define WEST 'W'

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define LEVEL_WIDTH 1920
#define LEVEL_HEIGHT 1440

#define BORDER_UP_Y 480
#define BORDER_DOWN_Y -340
#define BORDER_LEFT -220
#define BORDER_RIGHT 1040

#define MAX_OF_SPELLS 100
#define NUMBER_OF_SPELLS_1 100
#define DIRECTION 4
#define SPEED_OF_SPELLS 100
#define DISTANCE_BETWEEN_SPELS 100
#define SPELLS_X -180
#define SPELLS_Y 190

#define X_OF_CHARACTER 320
#define Y_OF_CHARACTER 240
#define MOVE 10

#define X_OF_SCROLLING -80
#define Y_OF_SCROLLING 240

#define HIT_BOX_X 50
#define HIT_BOX_Y 100

#define HEALTH 105
#define HEART_X 250
#define HEART_Y 430
#define HEART_Y_EN 60
#define HEART_D_X 610
#define HEART_SIZE 40
#define NUMBER_OF_HEART 10
#define TIME_TO_REST 2
#define DAMAGE 5

#define NUMBER_OF_PHOTOS 13
#define CHARSET 0
#define TLO 1
#define ENEMY 2
#define CHARACTER 3
#define SPELL 4
#define SPELL_1 5
#define HEART 6
#define HEART_DEAD 7
#define MENU 8
#define LOST 9
#define WIN 10
#define HEART_EN 11
#define CHARACTER_RIGHT 12


struct colors {
	int czarny;
	int zielony;
	int czerwony;
	int niebieski;
	int bialy;
};

struct fpss {
	int frames=0;
	double fpsTimer=0;
	double fps = 0;
	double worldTime = 0;
	double distance = 0;
	double distance_last = 0;
	double delta=0;
	double etiSpeed=1;
	int t1=0;
	int t2=0;
	int timer_flag=0;
};

struct spells {
	double x;
	double y;
};

struct player_spell {
	char dir;
	double time;
	double x;
	double y;
	double dx;
	double dy;
};

struct player {
	double time=-11;
	double time_left=0;
	int health = HEALTH;
	char dir = WEST;
	int shoot_dir = 1;
	int score = 0;
	double x;
	double y;
};

struct moving {
	int dx = 0;
	int dy = 0;
	int quit = 0;
	int quit_menu = 0;
	int casted_spell = 0;
	int spells_iter = 0;
	player_spell inf_spells[MAX_OF_SPELLS];
	player playerA;
	double dt = 0;
	int flag = 0;
};


void DrawString(SDL_Surface *screen, int x, int y, const char *text,
                SDL_Surface *charset) {
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while(*text) {
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = x;
		d.y = y;
		SDL_BlitSurface(charset, &s, screen, &d);
		x += 8;
		text++;
		};
	};

void DrawSurface(SDL_Surface *screen, SDL_Surface *sprite, int x, int y) {
	SDL_Rect dest;
	dest.x = x - sprite->w / 2;
	dest.y = y - sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
	};

void DrawPixel(SDL_Surface *surface, int x, int y, Uint32 color) {
	int bpp = surface->format->BytesPerPixel;
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32 *)p = color;
	};

void DrawLine(SDL_Surface *screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for(int i = 0; i < l; i++) {
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
		};
	};

void DrawRectangle(SDL_Surface *screen, int x, int y, int l, int k,  Uint32 outlineColor, Uint32 fillColor) {
	int i;
	DrawLine(screen, x, y, k, 0, 1, outlineColor);
	DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(screen, x, y, l, 1, 0, outlineColor);
	DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for(i = y + 1; i < y + k - 1; i++)
		DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
	};

bool PrepareGame(int rc, SDL_Renderer *renderer, SDL_Window *window) {
	if (rc != 0) {
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		return false;
	};

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

	SDL_SetWindowTitle(window, "BULLET HELL");

	// wy³¹czenie widocznoœci kursora myszy
	SDL_ShowCursor(SDL_DISABLE);
	return true;
}

void ImageUploadFail(SDL_Surface* screen, SDL_Texture* scrtex, SDL_Window* window, SDL_Renderer* renderer) {
	SDL_FreeSurface(screen);
	SDL_DestroyTexture(scrtex);
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_Quit();
}

void InformationBox(colors kolory, SDL_Surface *charset, SDL_Surface* screen, SDL_Texture* scrtex, fpss fpsset , SDL_Renderer* renderer, player playerA) {
	char text[128];
	// tekst informacyjny / info text
	DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, 36, kolory.czerwony, kolory.niebieski);
	//            "template for the second project, elapsed time = %.1lf s  %.0lf frames / s"
	sprintf(text, "BULLET HELL, czas trwania = %.1lf s  %.0lf klatek / s, Punkty: %d", fpsset.worldTime, fpsset.fps, playerA.score);
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);
	//	      "Esc - exit, \030 - faster, \031 - slower"
	sprintf(text, "Esc - wyjscie, Strzalki - przemieszczanie, Spacja - strzelanie , n - nowa gra" );
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 26, text, charset);

	SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
	//		SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, scrtex, NULL, NULL);
	SDL_RenderPresent(renderer);
}

void InformationBoxScore(colors kolory, SDL_Surface* charset, SDL_Surface* screen, SDL_Texture* scrtex, fpss fpsset, SDL_Renderer* renderer, player playerA) {
	char text[128];
	// tekst informacyjny / info text
	DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, 36, kolory.czerwony, kolory.niebieski);
	//            "template for the second project, elapsed time = %.1lf s  %.0lf frames / s"
	sprintf(text, "BULLET HELL, czas trwania = %.1lf s  %.0lf klatek / s, Punkty: %d", fpsset.worldTime, fpsset.fps, playerA.score);
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);
	//	      "Esc - exit, \030 - faster, \031 - slower"
	sprintf(text, "Twoj Wynik: %d", playerA.score);
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 26, text, charset);

	SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
	//		SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, scrtex, NULL, NULL);
	SDL_RenderPresent(renderer);
}

spells ChangerOfDirection_1(fpss fpsset, spells *speling_1, int dx, int dy) {
	for (int i = 0; i < DIRECTION*NUMBER_OF_SPELLS_1; i++) {
		speling_1[i].x = SPELLS_X;
		speling_1[i].y = SPELLS_Y;
	}
	
	for (int i = 0; i < NUMBER_OF_SPELLS_1; i++) {
		speling_1[i].x = SPELLS_X + dx - DISTANCE_BETWEEN_SPELS * i + SPEED_OF_SPELLS * fpsset.worldTime;
		speling_1[i].y = SPELLS_Y + dy;
	}
	for (int i = 0; i < NUMBER_OF_SPELLS_1; i++) {
		speling_1[i + NUMBER_OF_SPELLS_1].x = SPELLS_X + dx + DISTANCE_BETWEEN_SPELS * i - SPEED_OF_SPELLS * fpsset.worldTime;
		speling_1[i + NUMBER_OF_SPELLS_1].y = SPELLS_Y + dy;
	}
	for (int i = 0; i < NUMBER_OF_SPELLS_1; i++) {
		speling_1[i + 2 * NUMBER_OF_SPELLS_1].x = SPELLS_X + dx;
		speling_1[i + 2 * NUMBER_OF_SPELLS_1].y = SPELLS_Y + dy - DISTANCE_BETWEEN_SPELS * i + SPEED_OF_SPELLS * fpsset.worldTime;
	}
	for (int i = 0; i < NUMBER_OF_SPELLS_1; i++) {
		speling_1[i + 3 * NUMBER_OF_SPELLS_1].x = SPELLS_X + dx;
		speling_1[i + 3 * NUMBER_OF_SPELLS_1].y = SPELLS_Y + dy + DISTANCE_BETWEEN_SPELS * i - SPEED_OF_SPELLS * fpsset.worldTime;
	}
	return *speling_1;
}

spells ChangerOfDirection_2(fpss fpsset, spells* speling_1, int dx, int dy) {
	for (int i = 0; i < DIRECTION * NUMBER_OF_SPELLS_1; i++) {
		speling_1[i].x = SPELLS_X;
		speling_1[i].y = SPELLS_Y;
	}

		for (int i = 0; i < NUMBER_OF_SPELLS_1; i++) {
			speling_1[i].x = SPELLS_X + dx - DISTANCE_BETWEEN_SPELS * i - i*sin(fpsset.distance) * SCREEN_HEIGHT / 3;
			speling_1[i].y = SPELLS_Y + dy - DISTANCE_BETWEEN_SPELS * i - i*cos(fpsset.distance) * SCREEN_HEIGHT / 3;
		}
		for (int i = 0; i < NUMBER_OF_SPELLS_1; i++) {
			speling_1[i + NUMBER_OF_SPELLS_1].x = SPELLS_X + dx + DISTANCE_BETWEEN_SPELS * i - i*sin(fpsset.distance) * SCREEN_HEIGHT / 3;
			speling_1[i + NUMBER_OF_SPELLS_1].y = SPELLS_Y + dy - DISTANCE_BETWEEN_SPELS * i - i*cos(fpsset.distance) * SCREEN_HEIGHT / 3;
		}
		for (int i = 0; i < NUMBER_OF_SPELLS_1; i++) {
			speling_1[i + 2 * NUMBER_OF_SPELLS_1].x = SPELLS_X + dx - DISTANCE_BETWEEN_SPELS * i - i*sin(fpsset.distance) * SCREEN_HEIGHT / 3;
			speling_1[i + 2 * NUMBER_OF_SPELLS_1].y = SPELLS_Y + dy + DISTANCE_BETWEEN_SPELS * i - i*cos(fpsset.distance) * SCREEN_HEIGHT / 3;
		}
		for (int i = 0; i < NUMBER_OF_SPELLS_1; i++) {
			speling_1[i + 3 * NUMBER_OF_SPELLS_1].x = SPELLS_X + dx + DISTANCE_BETWEEN_SPELS * i - i*sin(fpsset.distance) * SCREEN_HEIGHT / 3;
			speling_1[i + 3 * NUMBER_OF_SPELLS_1].y = SPELLS_Y + dy + DISTANCE_BETWEEN_SPELS * i - i*cos(fpsset.distance) * SCREEN_HEIGHT / 3;
		}
	return *speling_1;
}

spells ChangerOfDirection_3(fpss fpsset, spells* speling_1, int dx, int dy) {
	
	
	for (int i = 0; i < DIRECTION * NUMBER_OF_SPELLS_1; i++) {
		speling_1[i].x = SPELLS_X;
		speling_1[i].y = SPELLS_Y;
	}

	for (int i = 0; i < NUMBER_OF_SPELLS_1; i++) {
		speling_1[i].x = SPELLS_X + dx - DISTANCE_BETWEEN_SPELS * i + SPEED_OF_SPELLS * fpsset.worldTime + sin(fpsset.distance) * SCREEN_HEIGHT / 3;
		speling_1[i].y = SPELLS_Y + dy + cos(fpsset.distance) * SCREEN_HEIGHT / 3;
	}
	for (int i = 0; i < NUMBER_OF_SPELLS_1; i++) {
		speling_1[i + NUMBER_OF_SPELLS_1].x = SPELLS_X + dx + DISTANCE_BETWEEN_SPELS * i - SPEED_OF_SPELLS * fpsset.worldTime + sin(fpsset.distance) * SCREEN_HEIGHT / 3;
		speling_1[i + NUMBER_OF_SPELLS_1].y = SPELLS_Y + dy + cos(fpsset.distance) * SCREEN_HEIGHT / 3;
	}
	for (int i = 0; i < NUMBER_OF_SPELLS_1; i++) {
		speling_1[i + 2 * NUMBER_OF_SPELLS_1].x = SPELLS_X + dx +sin(fpsset.distance) * SCREEN_HEIGHT / 3;
		speling_1[i + 2 * NUMBER_OF_SPELLS_1].y = SPELLS_Y + dy - DISTANCE_BETWEEN_SPELS * i + SPEED_OF_SPELLS * fpsset.worldTime + cos(fpsset.distance) * SCREEN_HEIGHT / 3;
	}
	for (int i = 0; i < NUMBER_OF_SPELLS_1; i++) {
		speling_1[i + 3 * NUMBER_OF_SPELLS_1].x = SPELLS_X + dx + sin(fpsset.distance) * SCREEN_HEIGHT / 3;
		speling_1[i + 3 * NUMBER_OF_SPELLS_1].y = SPELLS_Y + dy + DISTANCE_BETWEEN_SPELS * i - SPEED_OF_SPELLS * fpsset.worldTime + cos(fpsset.distance) * SCREEN_HEIGHT / 3;
	}
	return *speling_1;
}

player_spell ChangerOfDirection_SPELL(player_spell *inf_spells, int dx, int dy, fpss fpsset) {
	for (int i = 0; i < MAX_OF_SPELLS; i++) {
		if (inf_spells[i].dir == EAST) {
			inf_spells[i].x = X_OF_CHARACTER + SPEED_OF_SPELLS * (fpsset.worldTime - inf_spells[i].time) - inf_spells[i].dx + dx;
			inf_spells[i].y = Y_OF_CHARACTER - inf_spells[i].dy + dy;
		}
		else if (inf_spells[i].dir == WEST) {
			inf_spells[i].x = X_OF_CHARACTER - SPEED_OF_SPELLS * (fpsset.worldTime - inf_spells[i].time) - inf_spells[i].dx + dx;
			inf_spells[i].y = Y_OF_CHARACTER - inf_spells[i].dy + dy;
		}
		else if (inf_spells[i].dir == NORTH){
			inf_spells[i].x = X_OF_CHARACTER - inf_spells[i].dx + dx;
			inf_spells[i].y = Y_OF_CHARACTER - SPEED_OF_SPELLS * (fpsset.worldTime - inf_spells[i].time) - inf_spells[i].dy + dy;
		}
		else if (inf_spells[i].dir == SOUTH) {
			inf_spells[i].x = X_OF_CHARACTER - inf_spells[i].dx + dx;
			inf_spells[i].y = Y_OF_CHARACTER + SPEED_OF_SPELLS * (fpsset.worldTime - inf_spells[i].time) - inf_spells[i].dy + dy;
		}
	}
	return *inf_spells;
}

SDL_Surface UploadFiles(SDL_Surface **photos) {
	photos[CHARSET] = SDL_LoadBMP("./photo/cs8x8.bmp");
	if (photos[CHARSET] == NULL) {
		return **photos;
	};
	SDL_SetColorKey(photos[CHARSET], true, 0x000000);

	photos[TLO] = SDL_LoadBMP("./photo/tlo.bmp");
	if (photos[TLO] == NULL) {
		return **photos;
	};

	photos[ENEMY] = SDL_LoadBMP("./photo/enemy.bmp");
	if (photos[ENEMY] == NULL) {
		return **photos;
	};
	SDL_SetColorKey(photos[ENEMY], SDL_TRUE, SDL_MapRGB(photos[ENEMY]->format, 0XFF, 0x00, 0xA2));

	photos[CHARACTER] = SDL_LoadBMP("./photo/character.bmp");
	if (photos[CHARACTER] == NULL) {
		return **photos;
	};
	SDL_SetColorKey(photos[CHARACTER], SDL_TRUE, SDL_MapRGB(photos[CHARACTER]->format, 0XFF, 0x00, 0xA2));
	photos[CHARACTER_RIGHT] = SDL_LoadBMP("./photo/character_right.bmp");
	if (photos[CHARACTER_RIGHT] == NULL) {
		return **photos;
	};
	SDL_SetColorKey(photos[CHARACTER_RIGHT], SDL_TRUE, SDL_MapRGB(photos[CHARACTER_RIGHT]->format, 0XFF, 0x00, 0xA2));

	photos[SPELL] = SDL_LoadBMP("./photo/spell.bmp");
	if (photos[SPELL] == NULL) {
		return **photos;
	};
	SDL_SetColorKey(photos[SPELL], SDL_TRUE, SDL_MapRGB(photos[SPELL]->format, 0XFF, 0x00, 0xA2));

	photos[SPELL_1] = SDL_LoadBMP("./photo/spell_1.bmp");
	if (photos[SPELL_1] == NULL) {
		return **photos;
	};
	SDL_SetColorKey(photos[SPELL_1], SDL_TRUE, SDL_MapRGB(photos[SPELL_1]->format, 0XFF, 0x00, 0xA2));
	photos[HEART] = SDL_LoadBMP("./photo/heart.bmp");
	if (photos[HEART] == NULL) {
		return **photos;
	};
	SDL_SetColorKey(photos[HEART], SDL_TRUE, SDL_MapRGB(photos[HEART]->format, 0XFF, 0x00, 0xA2));
	photos[HEART_DEAD] = SDL_LoadBMP("./photo/heart_dead.bmp");
	if (photos[HEART_DEAD] == NULL) {
		return **photos;
	};
	SDL_SetColorKey(photos[HEART_DEAD], SDL_TRUE, SDL_MapRGB(photos[HEART_DEAD]->format, 0XFF, 0x00, 0xA2));
	photos[HEART_EN] = SDL_LoadBMP("./photo/heart_en.bmp");
	if (photos[7] == NULL) {
		return **photos;
	};
	SDL_SetColorKey(photos[HEART_EN], SDL_TRUE, SDL_MapRGB(photos[HEART_EN]->format, 0XFF, 0x00, 0xA2));
	photos[MENU] = SDL_LoadBMP("./photo/menu.bmp");
	if (photos[MENU] == NULL) {
		return **photos;
	};
	photos[LOST] = SDL_LoadBMP("./photo/lost.bmp");
	if (photos[LOST] == NULL) {
		return **photos;
	};
	photos[WIN] = SDL_LoadBMP("./photo/won.bmp");
	if (photos[WIN] == NULL) {
		return **photos;
	};
	return **photos;
}

void Menu(SDL_Surface* screen, SDL_Surface **photos, colors kolory, SDL_Texture* scrtex, fpss fpsset, SDL_Renderer* renderer, int win_flag, player playerA) {
	SDL_FillRect(screen, NULL, kolory.bialy);
	if (win_flag == 0) {
		DrawSurface(screen, photos[MENU], X_OF_CHARACTER, Y_OF_CHARACTER);
		InformationBoxScore(kolory, photos[CHARSET], screen, scrtex, fpsset, renderer, playerA);
	}
	else if (win_flag == -1) {
		DrawSurface(screen, photos[LOST], X_OF_CHARACTER, Y_OF_CHARACTER);
		InformationBoxScore(kolory, photos[CHARSET], screen, scrtex, fpsset, renderer, playerA);
	}
	else if (win_flag == 1) {
		DrawSurface(screen, photos[WIN], X_OF_CHARACTER, Y_OF_CHARACTER);
		InformationBoxScore(kolory, photos[CHARSET], screen, scrtex, fpsset, renderer, playerA);
	}
}

void Round_1(SDL_Surface* screen, SDL_Surface** photos, int dx, int dy, player_spell *inf_spells, colors kolory, int spells_iter, player playerA, player enemy, fpss fpsset, int casted_spell, spells *speling_1) {
	SDL_FillRect(screen, NULL, kolory.bialy);
	DrawSurface(screen, photos[TLO], X_OF_SCROLLING + dx, Y_OF_SCROLLING + dy);
	if (playerA.dir == NORTH || playerA.dir == WEST) {
		DrawSurface(screen, photos[CHARACTER], X_OF_CHARACTER, Y_OF_CHARACTER);
	}
	else {
		DrawSurface(screen, photos[CHARACTER_RIGHT], X_OF_CHARACTER, Y_OF_CHARACTER);
	}
	DrawSurface(screen, photos[ENEMY], X_OF_SCROLLING + dx, Y_OF_SCROLLING + dy);
	for (int i = 0; i < NUMBER_OF_HEART; i++) {
		DrawSurface(screen, photos[HEART_DEAD], HEART_X + HEART_SIZE * i, HEART_Y);
	}
	for (int i = 0; i < playerA.health / NUMBER_OF_HEART; i++) {
		DrawSurface(screen, photos[HEART], HEART_X + HEART_SIZE * i, HEART_Y);
	}
	for (int i = 0; i < enemy.health / NUMBER_OF_HEART; i++) {
		DrawSurface(screen, photos[HEART_EN], HEART_SIZE + HEART_SIZE * i, HEART_Y_EN);
	}

	for (int i = 0; i < spells_iter; i++) {
		DrawSurface(screen, photos[SPELL], speling_1[i].x, speling_1[i].y);
		DrawSurface(screen, photos[SPELL], speling_1[i + NUMBER_OF_SPELLS_1].x, speling_1[i + NUMBER_OF_SPELLS_1].y);
		DrawSurface(screen, photos[SPELL], speling_1[i + 2 * NUMBER_OF_SPELLS_1].x, speling_1[i + 2 * NUMBER_OF_SPELLS_1].y);
		DrawSurface(screen, photos[SPELL], speling_1[i + 3 * NUMBER_OF_SPELLS_1].x, speling_1[i + 3 * NUMBER_OF_SPELLS_1].y);
	}
	ChangerOfDirection_SPELL(inf_spells, dx, dy, fpsset);
	for (int i = 0; i < casted_spell; i++) {
		DrawSurface(screen, photos[SPELL_1], inf_spells[i].x, inf_spells[i].y);
	}
}

void Round_2(SDL_Surface* screen, SDL_Surface** photos, int dx, int dy, player_spell* inf_spells, colors kolory, int spells_iter, player playerA, player enemy, fpss fpsset, int casted_spell, spells* speling_1) {
	SDL_FillRect(screen, NULL, kolory.bialy);
	DrawSurface(screen, photos[TLO], X_OF_SCROLLING + dx, Y_OF_SCROLLING + dy);
	if (playerA.dir == NORTH || playerA.dir == WEST) {
		DrawSurface(screen, photos[CHARACTER], X_OF_CHARACTER, Y_OF_CHARACTER);
	}
	else {
		DrawSurface(screen, photos[CHARACTER_RIGHT], X_OF_CHARACTER, Y_OF_CHARACTER);
	}
	DrawSurface(screen, photos[ENEMY], X_OF_SCROLLING + dx, Y_OF_SCROLLING + dy);
	for (int i = 0; i < NUMBER_OF_HEART; i++) {
		DrawSurface(screen, photos[HEART_DEAD], HEART_X + HEART_SIZE * i, HEART_Y);
	}
	for (int i = 0; i < playerA.health / NUMBER_OF_HEART; i++) {
		DrawSurface(screen, photos[HEART], HEART_X + HEART_SIZE * i, HEART_Y);
	}
	for (int i = 0; i < enemy.health / NUMBER_OF_HEART; i++) {
		DrawSurface(screen, photos[HEART_EN], HEART_SIZE + HEART_SIZE * i, HEART_Y_EN);
	}

	for (int i = 0; i < spells_iter; i++) {
		DrawSurface(screen, photos[SPELL], speling_1[i].x, speling_1[i].y);
		DrawSurface(screen, photos[SPELL], speling_1[i + NUMBER_OF_SPELLS_1].x, speling_1[i + NUMBER_OF_SPELLS_1].y);
		DrawSurface(screen, photos[SPELL], speling_1[i + 2 * NUMBER_OF_SPELLS_1].x, speling_1[i + 2 * NUMBER_OF_SPELLS_1].y);
		DrawSurface(screen, photos[SPELL], speling_1[i + 3 * NUMBER_OF_SPELLS_1].x, speling_1[i + 3 * NUMBER_OF_SPELLS_1].y);
	}
	ChangerOfDirection_SPELL(inf_spells, dx, dy, fpsset);
	for (int i = 0; i < casted_spell; i++) {
		DrawSurface(screen, photos[SPELL_1], inf_spells[i].x, inf_spells[i].y);
	}
}

void Round_3(SDL_Surface* screen, SDL_Surface** photos, int dx, int dy, player_spell* inf_spells, colors kolory, int spells_iter, player playerA, player enemy, fpss fpsset, int casted_spell, spells* speling_1) {
	SDL_FillRect(screen, NULL, kolory.bialy);
	DrawSurface(screen, photos[TLO], X_OF_SCROLLING + dx, Y_OF_SCROLLING + dy);
	if (playerA.dir == NORTH || playerA.dir == WEST) {
		DrawSurface(screen, photos[CHARACTER], X_OF_CHARACTER, Y_OF_CHARACTER);
	}
	else {
		DrawSurface(screen, photos[CHARACTER_RIGHT], X_OF_CHARACTER, Y_OF_CHARACTER);
	}
	DrawSurface(screen, photos[ENEMY], enemy.x, enemy.y);
	for (int i = 0; i < NUMBER_OF_HEART; i++) {
		DrawSurface(screen, photos[HEART_DEAD], HEART_X + HEART_SIZE * i, HEART_Y);
	}
	for (int i = 0; i < playerA.health / NUMBER_OF_HEART; i++) {
		DrawSurface(screen, photos[HEART], HEART_X + HEART_SIZE * i, HEART_Y);
	}
	for (int i = 0; i < enemy.health / NUMBER_OF_HEART; i++) {
		DrawSurface(screen, photos[HEART_EN], HEART_SIZE + HEART_SIZE * i, HEART_Y_EN);
	}

	for (int i = 0; i < spells_iter; i++) {
		DrawSurface(screen, photos[SPELL], speling_1[i].x, speling_1[i].y);
		DrawSurface(screen, photos[SPELL], speling_1[i + NUMBER_OF_SPELLS_1].x, speling_1[i + NUMBER_OF_SPELLS_1].y);
		DrawSurface(screen, photos[SPELL], speling_1[i + 2 * NUMBER_OF_SPELLS_1].x, speling_1[i + 2 * NUMBER_OF_SPELLS_1].y);
		DrawSurface(screen, photos[SPELL], speling_1[i + 3 * NUMBER_OF_SPELLS_1].x, speling_1[i + 3 * NUMBER_OF_SPELLS_1].y);
	}
	ChangerOfDirection_SPELL(inf_spells, dx, dy, fpsset);
	for (int i = 0; i < casted_spell; i++) {
		DrawSurface(screen, photos[SPELL_1], inf_spells[i].x, inf_spells[i].y);
	}
}

player CoordinatesBossStatic(player enemyA, moving shift, fpss fpsset) {
	enemyA.x = X_OF_SCROLLING + shift.dx;
	enemyA.y = Y_OF_SCROLLING + shift.dy;
	return enemyA;
}

player CoordinatesBoss(player enemyA, moving shift, fpss fpsset) {
	enemyA.x = X_OF_SCROLLING + shift.dx + sin(fpsset.distance) * SCREEN_HEIGHT / 3;
	enemyA.y = Y_OF_SCROLLING + shift.dy + cos(fpsset.distance) * SCREEN_HEIGHT / 3;
	return enemyA;
}

fpss TimeCreate(fpss fpsset, moving shift) {
	fpsset.delta = (fpsset.t2 - fpsset.t1) * 0.001;
	fpsset.t1 = fpsset.t2;
	fpsset.worldTime += (fpsset.delta - shift.dt);
	fpsset.distance += fpsset.etiSpeed * fpsset.delta;
	return fpsset;
}

fpss FpsCreate(fpss fpsset) {
	fpsset.fpsTimer += fpsset.delta;
	if (fpsset.fpsTimer > 0.5) {
		fpsset.fps = fpsset.frames * 2;
		fpsset.frames = 0;
		fpsset.fpsTimer -= 0.5;
	};
	return fpsset;
}

moving Move_Keyboard_Calls(moving shift, SDL_Event event, player playerA, fpss fpsset) {
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_ESCAPE) {
				shift.quit = 1;
				return shift;
			}
			else if (event.key.keysym.sym == SDLK_UP) {
				shift.playerA.dir = NORTH;
				if (shift.dy <= BORDER_UP_Y)
					shift.dy += MOVE;
				return shift;
			}
			else if (event.key.keysym.sym == SDLK_DOWN) {
				shift.playerA.dir = SOUTH;
				if (shift.dy > BORDER_DOWN_Y)
					shift.dy -= MOVE;
				return shift;
			}

			else if (event.key.keysym.sym == SDLK_RIGHT) {
				shift.playerA.dir = EAST;
				if (shift.dx > BORDER_LEFT)
					shift.dx -= MOVE;
				return shift;
			}
			else if (event.key.keysym.sym == SDLK_LEFT) {
				shift.playerA.dir = WEST;
				if (shift.dx < BORDER_RIGHT)
					shift.dx += MOVE;
				return shift;
			}
			else if (event.key.keysym.sym == SDLK_SPACE) {
				if (shift.casted_spell < MAX_OF_SPELLS) {
					shift.inf_spells[shift.casted_spell].time = fpsset.worldTime;
					shift.inf_spells[shift.casted_spell].dx = shift.dx;
					shift.inf_spells[shift.casted_spell].dy = shift.dy;
					shift.inf_spells[shift.casted_spell].dir = playerA.dir;
					shift.casted_spell++;

				}
				return shift;
			}
			else if (event.key.keysym.sym == SDLK_n) {
				shift.dx = 0;
				shift.dy = 0;
				shift.spells_iter = 0;
				shift.casted_spell = 0;
				shift.flag = 1;
				playerA.health = HEALTH;
			}
			break;
		case SDL_KEYUP:
			break;
		case SDL_QUIT:
			shift.quit = 1;
			break;
		};
	};
	return shift;
}

moving NewGameShift(moving shift) {
	shift.quit = 0;
	shift.quit_menu = 1;
	shift.dx = 0;
	shift.dy = 0;
	shift.spells_iter = 0;
	shift.casted_spell = 0;
	return shift;
}

player NewGamePlayer(player playerA) {
	playerA.health = HEALTH;
	playerA.time = -11;
	playerA.time_left = 0;
	return playerA;
}

moving Escape(moving shift) {
	shift.quit_menu = 1;
	shift.quit = 1;
	return shift;
}

player ShootPlayer(player user, fpss fpsset, spells *speling_1) {
	user.time_left = fpsset.worldTime - user.time;
	if (user.time_left > TIME_TO_REST) {
		for (int i = 0; i < DIRECTION * NUMBER_OF_SPELLS_1; i++) {
			if (speling_1[i].x <= X_OF_CHARACTER + HIT_BOX_X && speling_1[i].x >= X_OF_CHARACTER - HIT_BOX_X && speling_1[i].y >= Y_OF_CHARACTER - HIT_BOX_Y && speling_1[i].y <= Y_OF_CHARACTER + HIT_BOX_Y) {
				user.health -= DAMAGE;
				user.time = fpsset.worldTime;
				if (user.health <= 0) {
					user.health = 0;
				}
				printf("PLAYER HEALTH: % d\n", user.health);
			}
		}
	}
	return user;
}

player ShootEnemy(player user, fpss fpsset, moving shift, player enemy) {
	user.time_left = fpsset.worldTime - user.time;
	if (user.time_left > TIME_TO_REST) {
		for (int i = 0; i < shift.casted_spell; i++) {
			if (enemy.x>= shift.inf_spells[i].x - HIT_BOX_X && enemy.x <= shift.inf_spells[i].x + HIT_BOX_X && enemy.y >= shift.inf_spells[i].y - HIT_BOX_Y && enemy.y <= shift.inf_spells[i].y + HIT_BOX_Y) {
				user.health -= DAMAGE;
				user.time = fpsset.worldTime;
				if (user.health <= 0) {
					user.health = 0;
				}
				printf("BOSS HEALTH: % d\n", user.health);
			}
		}
	}
	return user;
}

// main
#ifdef __cplusplus
extern "C"
#endif


int main(int argc, char **argv) {
	int rc, win_flag;
	SDL_Event event;
	SDL_Surface *screen;
	SDL_Surface *photos[NUMBER_OF_PHOTOS];
	SDL_Texture *scrtex;
	SDL_Window *window;
	SDL_Renderer *renderer;
	spells speling_1[DIRECTION*NUMBER_OF_SPELLS_1];
	moving shift;
	player playerA;
	player enemyA;
	fpss fpsset;
	enemyA.shoot_dir = 4;
	win_flag = 0;
	int score = 0;
	int lvl = 1;

	if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
	}
	rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer);
	if (PrepareGame(rc, renderer, window) == false) {
		return 1;
	}
	screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
	colors kolory = { SDL_MapRGB(screen->format, 0x00, 0x00, 0x00),
			  SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00),
			  SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00),
			  SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC),
			  SDL_MapRGB(screen->format, 0xFF, 0xFF, 0xFF) 
	};
	UploadFiles(photos);
	while (!shift.quit_menu) {
		score += playerA.score;
		while (!shift.quit_menu) {
			Menu(screen, photos, kolory, scrtex, fpsset, renderer, win_flag, playerA);
			while (SDL_PollEvent(&event)) {
				switch (event.type) {
				case SDL_KEYDOWN:
					if (event.key.keysym.sym == SDLK_ESCAPE) {
						shift = Escape(shift);
					}
					else if (event.key.keysym.sym == SDLK_n) {
						shift=NewGameShift(shift);
						fpsset.worldTime = shift.dt;
						playerA=NewGamePlayer(playerA);
						enemyA=NewGamePlayer(enemyA);
						win_flag = 0;
					}
					break;
				case SDL_KEYUP:
					break;
				case SDL_QUIT:
					shift = Escape(shift);
					break;
				}
			}
		}
		fpsset.t1 = SDL_GetTicks();
		while (!shift.quit) {
			shift.quit_menu = 0;
			if (shift.spells_iter < NUMBER_OF_SPELLS_1)
				shift.spells_iter = round(fpsset.worldTime);
			fpsset.t2 = SDL_GetTicks();
			fpsset=TimeCreate(fpsset, shift);
			if (lvl == 1) {
				enemyA=CoordinatesBossStatic(enemyA, shift, fpsset);
				ChangerOfDirection_1(fpsset, speling_1, shift.dx, shift.dy);																					  //stworzenie uk³adu kuli
				Round_1(screen, photos, shift.dx, shift.dy, shift.inf_spells, kolory, shift.spells_iter, playerA, enemyA, fpsset, shift.casted_spell, speling_1); //drukowanie kul
			}
			else if (lvl == 2) {
				enemyA=CoordinatesBossStatic(enemyA, shift, fpsset);
				ChangerOfDirection_2(fpsset, speling_1, shift.dx, shift.dy);
				Round_2(screen, photos, shift.dx, shift.dy, shift.inf_spells, kolory, shift.spells_iter, playerA, enemyA, fpsset, shift.casted_spell, speling_1);
			}
			else if (lvl == 3) {
				enemyA=CoordinatesBoss(enemyA, shift, fpsset);
				ChangerOfDirection_3(fpsset, speling_1, shift.dx, shift.dy);
				Round_3(screen, photos, shift.dx, shift.dy, shift.inf_spells, kolory, shift.spells_iter, playerA, enemyA, fpsset, shift.casted_spell, speling_1);
			}
			fpsset=FpsCreate(fpsset);
			InformationBox(kolory, photos[CHARSET], screen, scrtex, fpsset, renderer, playerA);
			shift=Move_Keyboard_Calls(shift, event, playerA, fpsset);																						 //obs³uga zdarzen z klawiatury
			if (shift.flag == 1) {
				fpsset.worldTime = shift.dt;
				playerA.health = HEALTH;
				shift.flag = 0;
			}
			playerA.dir = shift.playerA.dir;
			fpsset.frames++;
			playerA.time_left = fpsset.worldTime - playerA.time;
			playerA = ShootPlayer(playerA, fpsset, speling_1);																								//sprawdzenie czy kolizja kul z graczem
			enemyA = ShootEnemy(enemyA, fpsset, shift, enemyA);
			if (playerA.health <= 0) {
				shift.quit = 1;
				win_flag = -1;
			}
			else if (enemyA.health <= 0) {
				shift.quit = 1;
				win_flag = 1;
				if (lvl == 3) {
					lvl = 0;
				}
				lvl++;
			}
			playerA.score = playerA.health - enemyA.health + score;
		}
	}
	for (int i = 0; i < NUMBER_OF_PHOTOS; i++) {
		SDL_FreeSurface(photos[i]);
	}
	ImageUploadFail(screen, scrtex, window, renderer);
	return 0;
};
