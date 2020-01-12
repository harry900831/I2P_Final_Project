// [main.c]
// this template is provided for the 2D shooter game.

#define _CRT_SECURE_NO_DEPRECATE
#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <math.h>

// If defined, logs will be shown on console and written to file.
// If commented out, logs will not be shown nor be saved.
#define LOG_ENABLED


#define max(a, b) (a > b ? a : b)
#define min(a, b) (a < b ? a : b)
/* Constants. */
const float PI = acos(-1);
// Frame rate (frame per second)
const int FPS = 60;
// Display (screen) width.
const int SCREEN_W = 800;
// Display (screen) height.
const int SCREEN_H = 600;
// At most 4 audios can be played at a time.
const int RESERVE_SAMPLES = 4;
// Same as:
// const int SCENE_MENU = 1;
// const int SCENE_START = 2;
enum {
	SCENE_MENU = 1,
	SCENE_START = 2,
	SCENE_SETTINGS = 3,
	SCENE_END = 4
};

#define MOD 10007
int seed;
int rand_num(){
	return seed = (seed * seed % MOD + seed + 7122) % MOD;
}

/* Input states */

// The active scene id.
int active_scene;
// Keyboard state, whether the key is down or not.
bool key_state[ALLEGRO_KEY_MAX];
// Mouse state, whether the key is down or not.
// 1 is for left, 2 is for right, 3 is for middle.
bool *mouse_state;
// Mouse position.
int mouse_x, mouse_y;
// TODO: More variables to store input states such as joysticks, ...

/* Variables for allegro basic routines. */

ALLEGRO_DISPLAY* game_display;
ALLEGRO_EVENT_QUEUE* game_event_queue;
ALLEGRO_TIMER* game_update_timer;

/* Shared resources*/

ALLEGRO_FONT* font_pirulen_32;
ALLEGRO_FONT* font_pirulen_24;
// TODO: More shared resources or data that needed to be accessed
// across different scenes.

/* Menu Scene resources*/
ALLEGRO_BITMAP* main_img_background;
// #[HACKATHON 3-1]
// TODO: Declare 2 variables for storing settings images.
// Uncomment and fill in the code below.
ALLEGRO_BITMAP* img_settings;
ALLEGRO_BITMAP* img_settings2;
ALLEGRO_SAMPLE* main_bgm;
ALLEGRO_SAMPLE_ID main_bgm_id;

/* Start Scene resources*/
ALLEGRO_BITMAP* start_img_background;
ALLEGRO_BITMAP* start_img_plane;
ALLEGRO_BITMAP* start_img_enemy;
ALLEGRO_SAMPLE* start_bgm;
ALLEGRO_SAMPLE_ID start_bgm_id;
// #[HACKATHON 2-1]
// TODO: Declare a variable to store your bullet's image.
// Uncomment and fill in the code below.
ALLEGRO_BITMAP* plane_img_bullet;
ALLEGRO_BITMAP* enemy_img_bullet;
ALLEGRO_BITMAP* dead_enemy_img;

typedef struct {
	float x, y;
	float w, h;
	float vx, vy;
	bool hidden;
	ALLEGRO_BITMAP* img;
} MovableObject;
void draw_movable_object(MovableObject obj);

#define MAX_PLANE_BULLET 5
#define MAX_ENEMY_BULLET 20
#define MAX_ENEMY 5

MovableObject plane_bullets[MAX_PLANE_BULLET]; 
MovableObject enemy_bullets[MAX_ENEMY_BULLET];

typedef struct{
	float x, y;
	float w, h;
	float vx, vy;
	bool hidden;
	int hp;
	float last_shoot_timestamp;
	ALLEGRO_BITMAP* img;
} Enemy;
Enemy enemies[MAX_ENEMY];
void draw_enemy(Enemy obj);

typedef struct{
	float x, y;
	float w, h;
	float vx, vy;
	bool hidden;
	int hp;
	int score;
	float last_shoot_timestamp;
	ALLEGRO_BITMAP* img;
} Plane;
Plane plane;
void draw_plane(Plane obj);

const int PLANE_INIT_HP = 5;
const int ENEMY_INIT_HP = 3;
const float PLANE_SHOOT_COOLDOWN = 0.2f;
const float PLANE_BULLET_VELOCITY = 5.0f;
const float ENEMY_BULLET_VELOCITY = 3.0f;
const float ENEMY_VELOCITY = 2.0f;
double last_enemy_shoot_timestamp[MAX_ENEMY];

/* Declare function prototypes. */

// Initialize allegro5 library
void allegro5_init(void);
// Initialize variables and resources.
// Allows the game to perform any initialization it needs before
// starting to run.
void game_init(void);
// Process events inside the event queue using an infinity loop.
void game_start_event_loop(void);
// Run game logic such as updating the world, checking for collision,
// switching scenes and so on.
// This is called when the game should update its logic.
void game_update(void);
// Draw to display.
// This is called when the game should draw itself.
void game_draw(void);
// Release resources.
// Free the pointers we allocated.
void game_destroy(void);
// Function to change from one scene to another.
void game_change_scene(int next_scene);
// Load resized bitmap and check if failed.
ALLEGRO_BITMAP *load_bitmap_resized(const char *filename, int w, int h);

bool pnt_in_rect(int, int, int, int, int, int);
bool object_overlap(float, float, float, float, float, float, float, float);
bool movableobject_in_map(MovableObject);

void on_key_down(int keycode);
void on_mouse_down(int btn, int x, int y);

void game_abort(const char* format, ...);

void game_log(const char* format, ...);
// Log using va_list.
void game_vlog(const char* format, va_list arg);

int main(int argc, char** argv) {
	// Set random seed for better random outcome.
	srand(time(NULL)); seed = rand() %MOD;
	allegro5_init();
	game_log("Allegro5 initialized");
	game_log("Game begin");
	// Initialize game variables.
	game_init();
	game_log("Game initialized");
	// Draw the first frame.
	game_draw();
	game_log("Game start event loop");
	// This call blocks until the game is finished.
	game_start_event_loop();
	game_log("Game end");
	game_destroy();
	return 0;
}

void allegro5_init(void) {
	if (!al_init())
		game_abort("failed to initialize allegro");

	// Initialize add-ons.
	if (!al_init_primitives_addon())
		game_abort("failed to initialize primitives add-on");
	if (!al_init_font_addon())
		game_abort("failed to initialize font add-on");
	if (!al_init_ttf_addon())
		game_abort("failed to initialize ttf add-on");
	if (!al_init_image_addon())
		game_abort("failed to initialize image add-on");
	if (!al_install_audio())
		game_abort("failed to initialize audio add-on");
	if (!al_init_acodec_addon())
		game_abort("failed to initialize audio codec add-on");
	if (!al_reserve_samples(RESERVE_SAMPLES))
		game_abort("failed to reserve samples");
	if (!al_install_keyboard())
		game_abort("failed to install keyboard");
	if (!al_install_mouse())
		game_abort("failed to install mouse");
	// TODO: Initialize other addons such as video, ...

	// Setup game display.
	game_display = al_create_display(SCREEN_W, SCREEN_H);
	if (!game_display)
		game_abort("failed to create display");
	al_set_window_title(game_display, "I2P(I)_2019 Final Project <student_id>");

	// Setup update timer.
	game_update_timer = al_create_timer(1.0f / FPS);
	if (!game_update_timer)
		game_abort("failed to create timer");

	// Setup event queue.
	game_event_queue = al_create_event_queue();
	if (!game_event_queue)
		game_abort("failed to create event queue");

	// Malloc mouse buttons state according to button counts.
	const unsigned m_buttons = al_get_mouse_num_buttons();
	game_log("There are total %u supported mouse buttons", m_buttons);
	// mouse_state[0] will not be used.
	mouse_state = malloc((m_buttons + 1) * sizeof(bool));
	memset(mouse_state, false, (m_buttons + 1) * sizeof(bool));

	// Register display, timer, keyboard, mouse events to the event queue.
	al_register_event_source(game_event_queue, al_get_display_event_source(game_display));
	al_register_event_source(game_event_queue, al_get_timer_event_source(game_update_timer));
	al_register_event_source(game_event_queue, al_get_keyboard_event_source());
	al_register_event_source(game_event_queue, al_get_mouse_event_source());
	// TODO: Register other event sources such as timer, video, ...

	// Start the timer to update and draw the game.
	al_start_timer(game_update_timer);
}

void game_init(void) {
	/* Shared resources*/
	font_pirulen_32 = al_load_font("pirulen.ttf", 32, 0);
	if (!font_pirulen_32)
		game_abort("failed to load font: pirulen.ttf with size 32");

	font_pirulen_24 = al_load_font("pirulen.ttf", 24, 0);
	if (!font_pirulen_24)
		game_abort("failed to load font: pirulen.ttf with size 24");

	/* Menu Scene resources*/
	main_img_background = load_bitmap_resized("menu-bg.jpg", SCREEN_W, SCREEN_H);

	main_bgm = al_load_sample("S31-Night Prowler.ogg");
	if (!main_bgm)
		game_abort("failed to load audio: S31-Night Prowler.ogg");

	// #[HACKATHON 3-4]
	// TODO: Load settings images.
	// Don't forget to check their return values.
	// Uncomment and fill in the code below.
	img_settings = al_load_bitmap("settings.png");
	if (!img_settings)
		game_abort("failed to load image: settings.png");
	img_settings2 = al_load_bitmap("settings2.png");
	if (!img_settings2)
		game_abort("failed to load image: settings2.png");

	/* Start Scene resources*/
	start_img_background = load_bitmap_resized("game-bg.jpg", SCREEN_W, SCREEN_H);

	start_img_plane = load_bitmap_resized("plane.png", 60, 60);
	if (!start_img_plane)
		game_abort("failed to load image: plane.png");

	start_img_enemy = load_bitmap_resized("enemy.png", 68, 36);
	if (!start_img_enemy)
		game_abort("failed to load image: enemy.png");

	start_bgm = al_load_sample("mythica.ogg");
	if (!start_bgm)
		game_abort("failed to load audio: mythica.ogg");

	plane_img_bullet = load_bitmap_resized("bullet.png", 15, 30);
	if (!plane_img_bullet)
		game_abort("failed to load image: bullet.png");

	enemy_img_bullet = load_bitmap_resized("purple_bullet.png", 20, 32);
	if (!enemy_img_bullet)
		game_abort("failed to load image: purple_bullet.png");

	dead_enemy_img = load_bitmap_resized("enemy_dead.png", 400, 200);
	if (!dead_enemy_img)
		game_abort("failed to load image: enemy_dead.png");

	// Change to first scene.
	game_change_scene(SCENE_MENU);
}

void game_start_event_loop(void) {
	bool done = false;
	ALLEGRO_EVENT event;
	int redraws = 0;
	while (!done) {
		al_wait_for_event(game_event_queue, &event);
		if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
			game_log("Window close button clicked");
			done = true;
		} else if (event.type == ALLEGRO_EVENT_TIMER) {
			// Event for redrawing the display.
			if (event.timer.source == game_update_timer)
				// The redraw timer has ticked.
				redraws++;
		} else if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
			// Event for keyboard key down.
			game_log("Key with keycode %d down", event.keyboard.keycode);
			key_state[event.keyboard.keycode] = true;
			on_key_down(event.keyboard.keycode);
		} else if (event.type == ALLEGRO_EVENT_KEY_UP) {
			// Event for keyboard key up.
			game_log("Key with keycode %d up", event.keyboard.keycode);
			key_state[event.keyboard.keycode] = false;
		} else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
			// Event for mouse key down.
			game_log("Mouse button %d down at (%d, %d)", event.mouse.button, event.mouse.x, event.mouse.y);
			mouse_state[event.mouse.button] = true;
			on_mouse_down(event.mouse.button, event.mouse.x, event.mouse.y);
		} else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
			// Event for mouse key up.
			game_log("Mouse button %d up at (%d, %d)", event.mouse.button, event.mouse.x, event.mouse.y);
			mouse_state[event.mouse.button] = false;
		} else if (event.type == ALLEGRO_EVENT_MOUSE_AXES) {
			if (event.mouse.dx != 0 || event.mouse.dy != 0) {
				// Event for mouse move.
				//game_log("Mouse move to (%d, %d)", event.mouse.x, event.mouse.y);
				mouse_x = event.mouse.x;
				mouse_y = event.mouse.y;
			} else if (event.mouse.dz != 0) {
				// Event for mouse scroll.
				game_log("Mouse scroll at (%d, %d) with delta %d", event.mouse.x, event.mouse.y, event.mouse.dz);
			}
		}
		// TODO: Process more events and call callbacks by adding more
		// entries inside Scene.

		// Redraw
		if (redraws > 0 && al_is_event_queue_empty(game_event_queue)) {
			// if (redraws > 1)
			// 	game_log("%d frame(s) dropped", redraws - 1);
			// Update and draw the next frame.
			game_update();
			game_draw();
			redraws = 0;
		}
	}
}

void game_update(void) {
	if (active_scene == SCENE_START) {
		plane.vx = plane.vy = 0;
		if (key_state[ALLEGRO_KEY_UP] || key_state[ALLEGRO_KEY_W])
			plane.vy -= 1;
		if (key_state[ALLEGRO_KEY_DOWN] || key_state[ALLEGRO_KEY_S])
			plane.vy += 1;
		if (key_state[ALLEGRO_KEY_LEFT] || key_state[ALLEGRO_KEY_A])
			plane.vx -= 1;
		if (key_state[ALLEGRO_KEY_RIGHT] || key_state[ALLEGRO_KEY_D])
			plane.vx += 1;
		// 0.71 is (1/sqrt(2))
		plane.y += plane.vy * 4 * (plane.vx ? 0.71f : 1);
		plane.x += plane.vx * 4 * (plane.vy ? 0.71f : 1);

		if (plane.x - plane.w / 2 < 0)
			plane.x = plane.w / 2;
		else if (plane.x + plane.w / 2 > SCREEN_W)
			plane.x = SCREEN_W - plane.w / 2;
		if (plane.y - plane.h / 2 < 0)
			plane.y = plane.h / 2;
		else if (plane.y + plane.h / 2 > SCREEN_H)
			plane.y = SCREEN_H - plane.h / 2;

		int i, j;

		//enemy shoot
		float now;
		for(i = 0; i < MAX_ENEMY; i++){
			if(enemies[i].hidden) continue;
			now = al_get_time();
			if(now - last_enemy_shoot_timestamp[i] >= enemies[i].last_shoot_timestamp){
				for (j = 0; j < MAX_ENEMY_BULLET; j++) {
			    	if (enemy_bullets[j].hidden)
			   			break;
			    } 
			    if (j < MAX_ENEMY_BULLET) {
			    	enemy_bullets[j].hidden = false;
			    	enemy_bullets[j].x = enemies[i].x;
			    	enemy_bullets[j].y = enemies[i].y;
			    	float radius = atan2(1.0 * plane.y - enemies[i].y, 1.0 * plane.x - enemies[i].x);
			    	enemy_bullets[j].vx = ENEMY_BULLET_VELOCITY * cos(radius);
			    	enemy_bullets[j].vy = ENEMY_BULLET_VELOCITY * sin(radius);
			    	last_enemy_shoot_timestamp[i] = now;
			    }
			}
		}

		// process enemy bullet
		for (i = 0; i < MAX_ENEMY_BULLET; i++){
			if (enemy_bullets[i].hidden) continue;
			enemy_bullets[i].x += enemy_bullets[i].vx;
			enemy_bullets[i].y += enemy_bullets[i].vy;
			//game_log("%f %f", enemy_bullets[i].vx, enemy_bullets[i].vy);
			if(!movableobject_in_map(enemy_bullets[i])){
				enemy_bullets[i].hidden = true;
			}
			if(object_overlap(plane.x, plane.y, plane.w, plane.h, enemy_bullets[i].x, enemy_bullets[i].y, enemy_bullets[i].w, enemy_bullets[i].h)){
				enemy_bullets[i].hidden = true;
				plane.hp--;
			}
		}

		// process plane bullet 
		for (i = 0; i < MAX_PLANE_BULLET; i++) {
			if (plane_bullets[i].hidden) continue;
			plane_bullets[i].x += plane_bullets[i].vx;
			plane_bullets[i].y += plane_bullets[i].vy;
			if (!movableobject_in_map(plane_bullets[i])){
				plane_bullets[i].hidden = true;
				continue;
			}
			for(j = 0; j < MAX_ENEMY; j++){
				if(enemies[j].hidden)continue;
				if(object_overlap(plane_bullets[i].x, plane_bullets[i].y, plane_bullets[i].w, plane_bullets[i].h, enemies[j].x, enemies[j].y, enemies[j].w, enemies[j].h)){
					enemies[j].hp--;
					plane_bullets[i].hidden = true;
					if(enemies[j].hp == 0){
						plane.score += 100;
						enemies[j].hidden = true;	
					}
				}
			}
		}

		if(plane.hp <= 0) game_change_scene(SCENE_END);
	}
}

void game_draw(void) {
	if (active_scene == SCENE_MENU) {
		al_draw_bitmap(main_img_background, 0, 0, 0);
		al_draw_text(font_pirulen_32, al_map_rgb(255, 255, 255), SCREEN_W / 2, 30, ALLEGRO_ALIGN_CENTER, "Space Shooter");
		al_draw_text(font_pirulen_24, al_map_rgb(255, 255, 255), 20, SCREEN_H - 50, 0, "Press enter key to start");
		
		if (pnt_in_rect(mouse_x, mouse_y, SCREEN_W - 48, 10, 38, 38))
			al_draw_bitmap(img_settings, SCREEN_W - 48, 10, 0);
		else
			al_draw_bitmap(img_settings2, SCREEN_W - 48, 10, 0);

		al_draw_bitmap(start_img_plane, round(400 - al_get_bitmap_width(start_img_plane) / 2), round(500 - al_get_bitmap_height(start_img_plane) / 2), 0);
		al_draw_bitmap(dead_enemy_img, round(400 - al_get_bitmap_width(dead_enemy_img) / 2), round(250 - al_get_bitmap_height(dead_enemy_img) / 2), 0);
		
	} else if (active_scene == SCENE_START) {
		int i;
		al_draw_bitmap(start_img_background, 0, 0, 0);
		draw_plane(plane);
		for (i = 0; i < MAX_PLANE_BULLET; i++)
			draw_movable_object(plane_bullets[i]);
		for (i = 0; i < MAX_ENEMY; i++)
			draw_enemy(enemies[i]);
		for (i = 0; i < MAX_ENEMY_BULLET; i++)
			draw_movable_object(enemy_bullets[i]);
		al_draw_textf(font_pirulen_24, al_map_rgb(100, 50, 255), 20, SCREEN_H - 50, 0, "SCORE: %d", plane.score);
		
		al_draw_filled_rectangle(SCREEN_W/2 - 100, SCREEN_H - 40, SCREEN_W/2 + 100, SCREEN_H - 30, al_map_rgb(100, 100, 100));
		al_draw_filled_rectangle(SCREEN_W/2 - 100, SCREEN_H - 40, SCREEN_W/2 - 100 + (200 * plane.hp / PLANE_INIT_HP), SCREEN_H - 30, al_map_rgb(255, 0, 0));
		al_draw_rectangle(SCREEN_W/2 - 100, SCREEN_H - 40, SCREEN_W/2 + 100, SCREEN_H - 30, al_map_rgb(0, 0, 0), 2);
	}
	else if (active_scene == SCENE_SETTINGS) {
		al_clear_to_color(al_map_rgb(0, 0, 0));
	} else if(active_scene == SCENE_END){
		al_clear_to_color(al_map_rgb(0, 0, 0));
		al_draw_text(font_pirulen_32, al_map_rgb(255, 255, 255), SCREEN_W / 2, SCREEN_H / 2 - 70, ALLEGRO_ALIGN_CENTER, "THE END");
		al_draw_textf(font_pirulen_32, al_map_rgb(255, 255, 255), SCREEN_W / 2, SCREEN_H / 2 - 20, ALLEGRO_ALIGN_CENTER, "YOUR SCORE: %d", plane.score);
		al_draw_text(font_pirulen_32, al_map_rgb(255, 255, 255), SCREEN_W / 2, SCREEN_H / 2 + 30, ALLEGRO_ALIGN_CENTER, "PRESS R TO THE MENU");
	}
	al_flip_display();
}

void game_destroy(void) {
	// Destroy everything you have created.
	// Free the memories allocated by malloc or allegro functions.
	// Destroy shared resources.
	al_destroy_font(font_pirulen_32);
	al_destroy_font(font_pirulen_24);

	/* Menu Scene resources*/
	al_destroy_bitmap(main_img_background);
	al_destroy_sample(main_bgm);
	// #[HACKATHON 3-6]
	// TODO: Destroy the 2 settings images.
	// Uncomment and fill in the code below.
	al_destroy_bitmap(img_settings);
	al_destroy_bitmap(img_settings2);

	/* Start Scene resources*/
	al_destroy_bitmap(start_img_background);
	al_destroy_bitmap(start_img_plane);
	al_destroy_bitmap(start_img_enemy);
	al_destroy_sample(start_bgm);
	// #[HACKATHON 2-10]
	// TODO: Destroy your bullet image.
	// Uncomment and fill in the code below.
	al_destroy_bitmap(plane_img_bullet);
	al_destroy_bitmap(enemy_img_bullet);

	al_destroy_bitmap(dead_enemy_img);

	al_destroy_timer(game_update_timer);
	al_destroy_event_queue(game_event_queue);
	al_destroy_display(game_display);
	free(mouse_state);
}

void game_change_scene(int next_scene) {
	game_log("Change scene from %d to %d", active_scene, next_scene);
	// TODO: Destroy resources initialized when creating scene.
	if (active_scene == SCENE_MENU) {
		al_stop_sample(&main_bgm_id);
		game_log("stop audio (bgm)");
	} else if (active_scene == SCENE_START) {
		al_stop_sample(&start_bgm_id);
		game_log("stop audio (bgm)");
	}

	active_scene = next_scene;
	// TODO: Allocate resources before entering scene.
	if (active_scene == SCENE_MENU) {
		if (!al_play_sample(main_bgm, 1, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, &main_bgm_id))
			game_abort("failed to play audio (bgm)");
	} else if (active_scene == SCENE_START) {
		int i;
		plane.img = start_img_plane;
		plane.x = 400;
		plane.y = 500;
		plane.w = al_get_bitmap_width(plane.img);
        plane.h = al_get_bitmap_height(plane.img);
        plane.score = 0;
        plane.hp = PLANE_INIT_HP;
        plane.last_shoot_timestamp = al_get_time();
		for (i = 0; i < MAX_ENEMY; i++) {
			last_enemy_shoot_timestamp[i] = al_get_time();
			enemies[i].last_shoot_timestamp = rand_num()%5 + 1;
			enemies[i].img = start_img_enemy;
			enemies[i].w = al_get_bitmap_width(start_img_enemy);
			enemies[i].h = al_get_bitmap_height(start_img_enemy);
			enemies[i].x = enemies[i].w / 2 + (float)rand() / RAND_MAX * (SCREEN_W - enemies[i].w);
			enemies[i].y = rand_num() % 200 +50;
			enemies[i].hp = ENEMY_INIT_HP;
		}
		for (i = 0; i < MAX_PLANE_BULLET; i++) {
			plane_bullets[i].w = al_get_bitmap_width(plane_img_bullet);
			plane_bullets[i].h = al_get_bitmap_height(plane_img_bullet);
			plane_bullets[i].img = plane_img_bullet;
			plane_bullets[i].vx = 0;
			plane_bullets[i].vy = -3;
			plane_bullets[i].hidden = true;
		}
		for (i = 0; i < MAX_ENEMY_BULLET; i++) {
			enemy_bullets[i].w = al_get_bitmap_width(enemy_img_bullet);
			enemy_bullets[i].h = al_get_bitmap_height(enemy_img_bullet);
			enemy_bullets[i].img = enemy_img_bullet;
			enemy_bullets[i].vx = 0;
			enemy_bullets[i].vy = 3;
			enemy_bullets[i].hidden = true;
		}
		if (!al_play_sample(start_bgm, 1, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, &start_bgm_id))
			game_abort("failed to play audio (bgm)");
	}
}

void on_key_down(int keycode) {
	if (active_scene == SCENE_MENU) {
		if (keycode == ALLEGRO_KEY_ENTER)
			game_change_scene(SCENE_START);
	} else if(active_scene == SCENE_END){
		if(keycode == ALLEGRO_KEY_R){
			game_change_scene(SCENE_MENU);	
		}
	}
	
}

void on_mouse_down(int btn, int x, int y) {
	// #[HACKATHON 3-8]
	// TODO: When settings clicked, switch to settings scene.
	// Uncomment and fill in the code below.
	if (active_scene == SCENE_MENU) {
		if (btn == 1) {
			if (pnt_in_rect(x, y, SCREEN_W - 48, 10, 38, 38))
				game_change_scene(SCENE_SETTINGS);
		}
	} else if(active_scene == SCENE_START){
		int i;
		double now = al_get_time();
		if (btn == 1 && now - plane.last_shoot_timestamp >= PLANE_SHOOT_COOLDOWN) {
		    for (i = 0; i < MAX_PLANE_BULLET; i++) {
		        if (plane_bullets[i].hidden)
		            break;
		    } 
		    if (i < MAX_PLANE_BULLET) {
		    	plane.last_shoot_timestamp = now;
		        float radius = atan2(1.0 * mouse_y - plane.y, 1.0 * mouse_x - plane.x);
		        plane_bullets[i].hidden = false;
		        plane_bullets[i].x = plane.x;
		        plane_bullets[i].y = plane.y;
		        plane_bullets[i].vx = PLANE_BULLET_VELOCITY * cos(radius);
		        plane_bullets[i].vy = PLANE_BULLET_VELOCITY * sin(radius);
		    }
		}

	}
}

void draw_movable_object(MovableObject obj) {
	if (obj.hidden)
		return;
	al_draw_rotated_bitmap(obj.img, obj.w/2, obj.h/2, obj.x, obj.y, atan2(obj.vy, obj.vx) + PI/2, 0);
}
void draw_enemy(Enemy obj) {
	if (obj.hidden)
		return;
	al_draw_bitmap(obj.img, round(obj.x - obj.w / 2), round(obj.y - obj.h / 2), 0);
}
void draw_plane(Plane obj) {
	if (obj.hidden)
		return;
	float radius = atan2(1.0 * mouse_y - obj.y, 1.0 * mouse_x - obj.x) + PI/2;
	al_draw_rotated_bitmap(obj.img, obj.w/2, obj.h/2, obj.x , obj.y, radius, 0);
}
bool pnt_in_rect(int px, int py, int x, int y, int w, int h){
	return px >= x && px <= x + w && py >= y && py <= y + h;
}
bool object_overlap(float x1, float y1, float w1, float h1, float x2, float y2, float w2, float h2){
	float l1 = x1 - w1/2, r1 = x1 + w1/2, u1 = y1 - h1/2, d1 = y1 + h1/2;
	float l2 = x2 - w2/2, r2 = x2 + w2/2, u2 = y2 - h2/2, d2 = y2 + h2/2;
	if(max(l1, l2) < min(r1, r2) && max(u1, u2) < min(d1, d2)) return 1;
	else return 0;
}
bool movableobject_in_map(MovableObject obj){
	return obj.x >= 0 && obj.x < SCREEN_W && obj.y >= 0 && obj.y < SCREEN_H;
}
ALLEGRO_BITMAP *load_bitmap_resized(const char *filename, int w, int h) {
	ALLEGRO_BITMAP* loaded_bmp = al_load_bitmap(filename);
	if (!loaded_bmp)
		game_abort("failed to load image: %s", filename);
	ALLEGRO_BITMAP *resized_bmp = al_create_bitmap(w, h);
	ALLEGRO_BITMAP *prev_target = al_get_target_bitmap();

	if (!resized_bmp)
		game_abort("failed to create bitmap when creating resized image: %s", filename);
	al_set_target_bitmap(resized_bmp);
	al_draw_scaled_bitmap(loaded_bmp, 0, 0,
		al_get_bitmap_width(loaded_bmp),
		al_get_bitmap_height(loaded_bmp),
		0, 0, w, h, 0);
	al_set_target_bitmap(prev_target);
	al_destroy_bitmap(loaded_bmp);

	game_log("resized image: %s", filename);

	return resized_bmp;
}

void game_abort(const char* format, ...) {
	va_list arg;
	va_start(arg, format);
	game_vlog(format, arg);
	va_end(arg);
	fprintf(stderr, "error occured, exiting after 2 secs");
	// Wait 2 secs before exiting.
	al_rest(2);
	// Force exit program.
	exit(1);
}

void game_log(const char* format, ...) {
#ifdef LOG_ENABLED
	va_list arg;
	va_start(arg, format);
	game_vlog(format, arg);
	va_end(arg);
#endif
}

void game_vlog(const char* format, va_list arg) {
#ifdef LOG_ENABLED
	static bool clear_file = true;
	vprintf(format, arg);
	printf("\n");
	// Write log to file for later debugging.
	FILE* pFile = fopen("log.txt", clear_file ? "w" : "a");
	if (pFile) {
		vfprintf(pFile, format, arg);
		fprintf(pFile, "\n");
		fclose(pFile);
	}
	clear_file = false;
#endif
}


