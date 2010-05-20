#ifndef HM_SIMULATION_INTERFACE
#define HM_SIMULATION_INTERFACE

#ifdef __cplusplus
extern "C" {

typedef float vec_t;
typedef vec_t* vec3_t;
#endif

// These types are needed to interface C++ classes with C code.
typedef void* context_t;

// Hivemind interface
context_t create_hivemind_context();
void hivemind_world_update(context_t hm_context);

// ------------------------------------------------------------------------------------------
// Player Management
// ------------------------------------------------------------------------------------------
typedef unsigned int backend_t; 
typedef void* interface_t;
typedef void* controller_t;

void bot_connect(char* name);
void bot_config(char *appearance, char *gender);
void bot_disconnect();

vec_t *bot_origin();
vec_t *bot_orientation();
unsigned int bot_entity_id();
int bot_health();
int bot_frags();

void bot_move(vec3_t orientation, vec3_t velocity);

#if 0
// ------------------------------------------------------------------------------------------
// Bot Actions
// ------------------------------------------------------------------------------------------
void command_move( bot_t, vec3_t direction );
void command_turn( bot_t, vec3_t orientation );
void command_jump( bot_t );
void command_duck( bot_t );


// ------------------------------------------------------------------------------------------
// Physics Interface
// ------------------------------------------------------------------------------------------
float physics_walk( bot_t, float angle, float steps );
float physics_line( bot_t b, vec_t* s, vec_t* d );
float physics_balance( bot_t );
void* physics_getplatform( bot_t );
int physics_iswater( bot_t );
int physics_isladder( bot_t );
int physics_isair( bot_t );
int physics_collide( bot_t, vec_t* normal);
vec_t* physics_position( bot_t );
vec_t* physics_direction( bot_t );



// ------------------------------------------------------------------------------------------
// Entity Access
// ------------------------------------------------------------------------------------------
typedef unsigned short entity_t;

void entity_reset( bot_t );
void* entity_ptr( bot_t, entity_t );
void* entity_allocate( entity_t, char* );
void entity_deallocate( void *, char* );
entity_t entity_next( bot_t );
vec_t* entity_position( bot_t b, entity_t );
void entity_init();


entity_t entity_typefromstring( char* str );
char* entity_stringfromtype( entity_t type );


void entity_valuable_value(void* ent, int v);
void entity_weapon_ammo(void* ent, void* ammo);
void entity_door_height(void* ent, float h);
void entity_platform_distances(void* ent, float t, float b);
void entity_projectile_heading(void* ent, vec_t* h);


// ------------------------------------------------------------------------------------------
// Event Handling
// ------------------------------------------------------------------------------------------
typedef void* event_t;

event_t event_message(char* name);
event_t event_valuable(char* name, int value);
event_t event_pain(void* owner_entity_ptr, vec_t* pos, int damage);
event_t event_situated(char* name, void* owner_entity_ptr, vec_t* pos);

void bot_visual_event(unsigned int body, event_t event);
void bot_personal_event(unsigned int body, event_t event);


// ------------------------------------------------------------------------------------------
// Communication Interface
// ------------------------------------------------------------------------------------------
void command_say( bot_t, const char* text, int team );
void command_wave( bot_t, int i );

int personal_health(bot_t);
int personal_armor(bot_t);
void inventory_reset(bot_t);
void* inventory_next(bot_t);


// ------------------------------------------------------------------------------------------
// Game Interface
// ------------------------------------------------------------------------------------------
int game_bots();
int game_full();
char* game_name();


// ------------------------------------------------------------------------------------------
// Weapon Handling
// ------------------------------------------------------------------------------------------
int weapon_fire( bot_t );
int weapon_select( bot_t, const char* weapon );
char* weapon_get( bot_t b );


// ------------------------------------------------------------------------------------------
// FEAR Controller Interface
// ------------------------------------------------------------------------------------------
controller_t controller_create();
void controller_destroy( controller_t );
void controller_run( controller_t );
void controller_process( controller_t, backend_t );
void controller_request( controller_t ctrl, char* name );
void controller_kick( controller_t ctrl, backend_t );


// ------------------------------------------------------------------------------------------
// Debug Tools
// ------------------------------------------------------------------------------------------

#define COLOUR_BLUE		0xf3f3f1f1
#define COLOUR_RED		0xf2f2f0f0
#define COLOUR_GREEN	0xd0d1d2d3
#define COLOUR_YELLOW	0xdcdddedf
#define COLOUR_ORANGE	0xe0e1e2e3

#define GLOW_RED		0x00040000
#define GLOW_BLUE		0x00080000


void debug_line( vec3_t, vec3_t, unsigned int col );
void debug_print( char *text );
void debug_glow( bot_t, unsigned int col );
#endif

#ifdef __cplusplus
} // "C"
#endif

#endif

