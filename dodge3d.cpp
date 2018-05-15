// Kyle Overstreet
// CMPS 4490
// Game Project

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>
#include "log.h"
#include "fonts.h"
#include <string>
#include <string.h>
#include <sstream>
#include </usr/include/AL/alut.h>
#include "ppm.h"

typedef float Flt;
typedef Flt Vec[3];
typedef Flt	Matrix[4][4];
typedef int iVec[3];
//some defined macros
#define MakeVector(x, y, z, v) (v)[0]=(x),(v)[1]=(y),(v)[2]=(z)
#define VecMake(x, y, z, v) (v)[0]=(x),(v)[1]=(y),(v)[2]=(z)
#define VecNegate(a) (a)[0]=(-(a)[0]); (a)[1]=(-(a)[1]); (a)[2]=(-(a)[2]);
#define VecDot(a,b) ((a)[0]*(b)[0]+(a)[1]*(b)[1]+(a)[2]*(b)[2])
#define VecLen(a) ((a)[0]*(a)[0]+(a)[1]*(a)[1]+(a)[2]*(a)[2])
#define VecLenSq(a) sqrtf((a)[0]*(a)[0]+(a)[1]*(a)[1]+(a)[2]*(a)[2])
#define VecCopy(a,b) (b)[0]=(a)[0];(b)[1]=(a)[1];(b)[2]=(a)[2];
#define VecAdd(a,b,c) \
    (c)[0]=(a)[0]+(b)[0];\
(c)[1]=(a)[1]+(b)[1];\
(c)[2]=(a)[2]+(b)[2]
#define VecSub(a,b,c) \
    (c)[0]=(a)[0]-(b)[0]; \
(c)[1]=(a)[1]-(b)[1]; \
(c)[2]=(a)[2]-(b)[2]
#define VecS(A,a,b) (b)[0]=(A)*(a)[0]; (b)[1]=(A)*(a)[1]; (b)[2]=(A)*(a)[2]
#define VecAddS(A,a,b,c) \
    (c)[0]=(A)*(a)[0]+(b)[0]; \
(c)[1]=(A)*(a)[1]+(b)[1]; \
(c)[2]=(A)*(a)[2]+(b)[2]
#define VecCross(a,b,c) \
    (c)[0]=(a)[1]*(b)[2]-(a)[2]*(b)[1]; \
(c)[1]=(a)[2]*(b)[0]-(a)[0]*(b)[2]; \
(c)[2]=(a)[0]*(b)[1]-(a)[1]*(b)[0]
#define VecZero(v) (v)[0]=0.0;(v)[1]=0.0;v[2]=0.0
#define ABS(a) (((a)<0)?(-(a)):(a))
#define SGN(a) (((a)<0)?(-1):(1))
#define SGND(a) (((a)<0.0)?(-1.0):(1.0))
#define rnd() (float)rand() / (float)RAND_MAX
#define PI 3.14159265358979323846264338327950
#define MY_INFINITY 1000.0
const Flt DTR = 1.0 / 180.0 * PI;

using namespace std;

// Sound functions
void initialize_sounds();
void play_theme();
void stop_theme();
void continue_theme();
void play_sound(string);
void cleanup_sounds();

// Image functions
void convertpng2ppm(void);
void getImage(void);
void generateTextures(void);
unsigned char *buildAlphaData(Ppmimage);
void cleanupPPM(void);

void init();
void init_opengl();
void check_mouse(XEvent *e);
int check_keys(XEvent *e);
void physics();
void y_rotate(float, Vec, Vec);
void render();

bool start_text_timer();
bool start_notext_timer();
bool check_text_timer(bool);
bool check_notext_timer(bool);
void start_countDown_timer();
bool check_countDown_timer();
void countDown3();
void countDown2();
void countDown1();
void countDown0();
bool start_powerup_timer();
bool check_powerup_timer(bool powerup);
void restart_game();
void gamelog(string, int, string);
void view_scores();

//-----------------------------------------------------------------------------
//Setup timers
const double physicsRate = 1.0 / 30.0;
const double oobillion = 1.0 / 1e9;
struct timespec timeStart, timeCurrent;
struct timespec timePause;
double physicsCountdown=0.0;
double timeSpan=0.0;
unsigned int upause=0;
double timeDiff(struct timespec *start, struct timespec *end) {
	return (double)(end->tv_sec - start->tv_sec ) +
		(double)(end->tv_nsec - start->tv_nsec) * oobillion;
}
void timeCopy(struct timespec *dest, struct timespec *source) {
	memcpy(dest, source, sizeof(struct timespec));
}
//-----------------------------------------------------------------------------
timespec txt_start, txt_current;
timespec countdown_start, countdown_current;
timespec power_start, power_current;
timespec fps_start, fps_current;

ALuint alSource;
ALuint themeSource;
ALuint alBuffer;

Ppmimage *bgImage;
Ppmimage *logoImage;
Ppmimage *roadImage;
Ppmimage *wallImage;
Ppmimage *deadImage;
Ppmimage *spikeImage;
Ppmimage *bombImage;
Ppmimage *coinImage;
Ppmimage *heartImage;
Ppmimage *starImage;

GLuint bgTexture;
GLuint logoTexture;
GLuint roadTexture;
GLuint wallTexture;
GLuint deadTexture;
GLuint spikeTexture;
GLuint bombTexture;
GLuint coinTexture;
GLuint heartTexture;
GLuint starTexture;

unsigned char *screendata = NULL;
float fps_counter = 0;
char player_name[100];
bool blinkon = true;
bool blinkoff = false;
bool countdown_started;
bool countdown_done;
bool one;
bool two;
bool three;
bool four;

Vec spot = {0, 0, -1.0};
int keys[65536];
float spike_collision[500][2];
float bomb_collision[100][2];
float coin_collision[100][2];
float coin2_collision[10][2];
float heart_collision[50][2];
float star_collision[25][2];
float random_spikes[1000];
float random_bombs[100];
float random_coins[100];
float random_hearts[50];
float random_stars[25];

class Global {
    public:
	int xres, yres;
	int xres3, xres4;
	string difficulty;
	bool audio_on;
	bool display_attract;
	bool display_startmenu;
	bool display_entername;
	bool display_tutorial;
	bool display_difficulty;
	bool display_audiosettings;
	bool display_gameover;
	bool display_endmenu;
	bool invincible;
	int prev_menu;
	int menu_position;
	Flt aspectRatio;
	Vec cameraPosition;
	Vec dir;
	GLfloat lightPosition[4];
	int check_x, check_z;
	int check_bomb_z;
	int check_coin_z;
	int check_heart_z;
	bool moveLeft;
	bool moveRight;
	bool sprint;
	bool jumping;
	bool falling;
	int health;
	int coins;
	float acceleration;
	int num_spikes;
	int num_bombs;
	int num_coins;
	int num_coins2;
	int num_hearts;
	int num_stars;
	float angle;
	Global() {
	    xres=640;
	    yres=480;
	    difficulty = "EASY";
	    audio_on = true;
	    display_attract = true;
	    display_startmenu = false;
	    display_entername = false;
	    display_tutorial = false;
	    display_difficulty = false;
	    display_audiosettings = false;
	    display_gameover = false;
	    display_endmenu = false;
	    prev_menu = 0;
	    menu_position = 1;
	    invincible = false;
	    aspectRatio = (GLfloat)xres / (GLfloat)yres;
	    MakeVector(0.0, 1.0, 15.0, cameraPosition);
	    dir[0] = 0.0 - cameraPosition[0];
	    dir[1] = 0.0 - cameraPosition[1];
	    dir[2] = 0.0 - cameraPosition[2];
	    //light is up high, right a little, toward a little
	    MakeVector(100.0f, 240.0f, 40.0f, lightPosition);
	    lightPosition[3] = 1.0f;
	    health = 4;
	    coins = 0;
	    check_z = -1;
	    check_bomb_z = -1;
		check_coin_z = -1;
		check_heart_z = -1;
		moveLeft = true;
		angle = 0;
		acceleration = 0.1;
		num_spikes = 500;
		num_bombs = 100;
		num_coins = 100;
		num_coins2 = 10;
		num_hearts = 50;
		num_stars = 25;
	}
	void identity33(Matrix m) {
	    m[0][0] = m[1][1] = m[2][2] = 1.0f;
	    m[0][1] = m[0][2] = m[1][0] = m[1][2] = m[2][0] = m[2][1] = 0.0f;
	}
} g;

class Object {
    private:
	Vec *vert;
	Vec *norm;
	iVec *face;
	Matrix m;
	int nverts;
	int nfaces;
	Vec pos, vel, rot;
	Vec color;
    public:
	~Object() {
	    delete [] vert;
	    delete [] face;
	    delete [] norm;
	}
	Object(int nv, int nf) {
	    vert = new Vec[nv];
	    face = new iVec[nf];
	    norm = new Vec[nf];
	    nverts = nv;
	    nfaces = nf;
	    VecZero(pos);
	    VecZero(vel);
	    VecZero(rot);
	    VecMake(.9, .9, 0, color);
	    g.identity33(m);
	}
	void setColor(float r, float g, float b) {
	    VecMake(r, g, b, color);
	}
	void setVert(Vec v, int i) {
	    VecMake(v[0], v[1], v[2], vert[i]);
	}
	void setFace(iVec f, int i) {
	    VecMake(f[0], f[2], f[1], face[i]);
	}
	void translate(Flt x, Flt y, Flt z) {
	    pos[0] += x;
	    pos[1] += y;
	    pos[2] += z;
	}
	void rotate(Flt x, Flt y, Flt z) {
	    rot[0] += x;
	    rot[1] += y;
	    rot[2] += z;
	}
	void scale(Flt scalar) {
	    for (int i = 0; i < nverts; i++) {
		VecS(scalar, vert[i], vert[i]);
	    }
	}
	void getTriangleNormal(Vec tri[3], Vec norm) {
	    Vec v0, v1;
	    VecSub(tri[1], tri[0], v0);
	    VecSub(tri[2], tri[0], v1);
	    VecCross(v0, v1, norm);
	    Flt vecNormalize(Vec vec);
	    vecNormalize(norm);
	}
	void doRotate(Vec r) {
	    if (r[0] != 0.0)
		glRotatef(r[0], 1.0f, 0.0f, 0.0f);
	    if (r[1] != 0.0)
		glRotatef(r[1], 0.0f, 1.0f, 0.0f);
	    if (r[2] != 0.0)
		glRotatef(r[2], 0.0f, 0.0f, 1.0f);
	}
	void draw() {
	    //build our own rotation matrix for rotating the shadow polys.
	    g.identity33(m);
	    Vec vr;
	    VecMake(rot[0]*DTR, rot[1]*DTR, rot[2]*DTR, vr);
	    void yy_transform(const Vec rotate, Matrix a);
	    yy_transform(vr, m);
	    //must do for each triangle face...
	    glPushMatrix();
	    for (int j=0; j<nfaces; j++) {
		//transform the vertices of the tri, to use for the shadow
		Vec tv[3], norm;
		int fa = face[j][0];
		int fb = face[j][1];
		int fc = face[j][2];
		void trans_vector(Matrix mat, const Vec in, Vec out);
		trans_vector(m, vert[fa], tv[0]);
		trans_vector(m, vert[fb], tv[1]);
		trans_vector(m, vert[fc], tv[2]);
		VecAdd(tv[0], pos, tv[0]);
		VecAdd(tv[1], pos, tv[1]);
		VecAdd(tv[2], pos, tv[2]);
		getTriangleNormal(tv, norm);
		glBegin(GL_TRIANGLES);
		glColor3fv(color);
		glNormal3fv(norm);
		glVertex3fv(tv[0]);
		glVertex3fv(tv[1]);
		glVertex3fv(tv[2]);
		glEnd();
	    }
	    glPopMatrix();
	}
} *menu_spike, *spikes[1000], *bombs[100], *coins[100], *hearts[50], *stars[25], *coins2[10];

class Input {
    public:
        int loc[2];
        int size;
        char player1[100];
        //char player2[100];
        Input () {
            loc[0] = 0;
            //loc[1] = 0;
            player1[0] = '\0';
            //player2[0] = '\0';
            size = 32;
        }
} input;

void onePlayerStart(int, char [], Input &input);
void player1Name(char [], Input &input);
void getName_player1(int, Input &input);
void assign_namep1(char[], Input &input);

struct Vect {
    float x, y, z;
};

struct Box {
    float width, height;
    Vect center;
};

struct MenuBox {
    Box box[2];
};

class X11_wrapper {
    private:
	Display *dpy;
	Window win;
	GLXContext glc;
    public:
    int init_h;
    int init_w;
	X11_wrapper() {
	    Window root;
	    GLint att[] = { GLX_RGBA,
		GLX_STENCIL_SIZE, 2,
		GLX_DEPTH_SIZE, 24,
		GLX_DOUBLEBUFFER, None };
	    Colormap cmap;
	    XSetWindowAttributes swa;
	    setup_screen_res(640, 480);
	    dpy = XOpenDisplay(NULL);
	    if (dpy == NULL) {
			printf("\n\tcannot connect to X server\n\n");
			exit(EXIT_FAILURE);
	    }
	    root = DefaultRootWindow(dpy);

	    //for fullscreen
		XWindowAttributes getWinAttr;
	    XGetWindowAttributes(dpy, root, &getWinAttr);
		int fullscreen = 0;

		int init_w = 1200;
		int init_h = 800;
		
        //int init_w = 0;
        //int init_h = 0;
        g.xres = init_w;
		g.yres = init_h;
		if (!init_w && !init_h) {
			//Go to fullscreen.
			g.xres = getWinAttr.width;
			g.yres = getWinAttr.height;
			//printf("getWinAttr: %i %i\n", w, h); fflush(stdout);
			//When window is fullscreen, there is no client window
			//so keystrokes are linked to the root window.
			XGrabKeyboard(dpy, root, False,
				GrabModeAsync, GrabModeAsync, CurrentTime);
			fullscreen = 1;
		}

	    XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
	    if (vi == NULL) {
			printf("\n\tno appropriate visual found\n\n");
			exit(EXIT_FAILURE);
	    } 
	    cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
	    swa.colormap = cmap;
	    swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
		StructureNotifyMask | SubstructureNotifyMask;

		//for fullscreen
		unsigned int winops = CWBorderPixel|CWColormap|CWEventMask;
		if (fullscreen) {
			winops |= CWOverrideRedirect;
			swa.override_redirect = True;
		}

		//true fullscreen
	    /*win = XCreateWindow(dpy, root, 0, 0, g.xres, g.yres, 0,
		    vi->depth, InputOutput, vi->visual,
		    winops | CWEventMask, &swa);*/

	    //windowed fullscreen
	   	win = XCreateWindow(dpy, root, 0, 0, g.xres, g.yres, 0,
		    vi->depth, InputOutput, vi->visual,
		    CWColormap | CWEventMask, &swa);

	    set_title();
	    glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
	    glXMakeCurrent(dpy, win, glc);
	}
	~X11_wrapper() {
	    XDestroyWindow(dpy, win);
	    XCloseDisplay(dpy);
	}
	void setup_screen_res(const int w, const int h) {
	    g.xres = init_w;
	    g.yres = init_h;
		g.xres3 = g.xres * 3;
		g.xres4 = g.xres * 4;
	    if (screendata)
			delete [] screendata;
		screendata = new unsigned char[g.yres * g.xres4];
	    //g.aspectRatio = (GLfloat)g.xres / (GLfloat)g.yres;
	}
	void check_resize(XEvent *e) {
	    //The ConfigureNotify is sent by the
	    //server if the window is resized.
	    if (e->type != ConfigureNotify)
		return;
	    XConfigureEvent xce = e->xconfigure;
	    if (xce.width != g.xres || xce.height != g.yres) {
		//Window size did change.
		reshape_window(xce.width, xce.height);
	    }
	}
	void reshape_window(int width, int height) {
	    //window has been resized.
	    setup_screen_res(width, height);
	    //
	    glViewport(0, 0, (GLint)width, (GLint)height);
	    glMatrixMode(GL_PROJECTION); glLoadIdentity();
	    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	    glOrtho(0, g.xres, 0, g.yres, -1, 1);
	    set_title();
	}
	void set_title() {
	    //Set the window title bar.
	    XMapWindow(dpy, win);
	    XStoreName(dpy, win, "Dodge 3D");
	}
	bool getXPending() {
	    return XPending(dpy);
	}
	XEvent getXNextEvent() {
	    XEvent e;
	    XNextEvent(dpy, &e);
	    return e;
	}
	void swapBuffers() {
	    glXSwapBuffers(dpy, win);
	}
} x11;

int main(int argc, char* argv[])
{
	/*if (argc > 1) {
		if (strcmp(argv[1], "-w") == 0) {
			x11.init_w = 1200;
			x11.init_h = 800;
			printf("hello\n");
		} else {
			printf("Execute game with -w option for windowed mode!\n");
			return 0;
		}
	} else {
		printf("blah");
		x11.init_w = 0;
		x11.init_h = 0;
	}*/

	initialize_sounds();
	play_theme();
	init();
    init_opengl();

	clock_gettime(CLOCK_REALTIME, &fps_start);
    blinkon = start_text_timer();

    int done = 0;
    while (!done) {
		while (x11.getXPending()) {
			XEvent e = x11.getXNextEvent();
			x11.check_resize(&e);
			check_mouse(&e);
			done = check_keys(&e);
		}
		render();
		if (!g.display_attract && !g.display_startmenu && !g.display_entername
			&& !g.display_tutorial && !g.display_audiosettings && !g.display_gameover
			&& !g.display_endmenu) {
			physics();
		}
		x11.swapBuffers();
    }
    cleanup_fonts();
    cleanupPPM();
    return 0;
}

void initialize_sounds() 
{
    alutInit(0, NULL);
    if ( alGetError() != AL_NO_ERROR ) {
		printf("error initializing sound\n");
		return;
    }
    alGetError();

    float vec[6] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f };
    alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
    alListenerfv(AL_ORIENTATION, vec);
    alListenerf(AL_GAIN, 1.0f);
}

void play_theme()
{
    alBuffer = alutCreateBufferFromFile("./sounds/off_limits.wav");

    alGenSources(1, &themeSource);
    alSourcei(themeSource, AL_BUFFER, alBuffer);

    alSourcef(themeSource, AL_GAIN, 1.0f);
    alSourcef(themeSource, AL_PITCH, 1.0f);
    alSourcei(themeSource, AL_LOOPING, AL_TRUE);
    if (alGetError() != AL_NO_ERROR) {
	printf("ERROR setting sound source\n");
	return;
    }

    alSourcePlay(themeSource);
}

void stop_theme() 
{
	alSourcePause(themeSource);
	return;
}

void continue_theme() 
{
	alSourcePlay(themeSource);
}

void play_sound(string path)
{
    const char* sound_path = path.c_str();
    alBuffer = alutCreateBufferFromFile(sound_path);

    alGenSources(1, &alSource);
    alSourcei(alSource, AL_BUFFER, alBuffer);

    alSourcef(alSource, AL_GAIN, 1.0f);
    alSourcef(alSource, AL_PITCH, 1.0f);
    alSourcei(alSource, AL_LOOPING, AL_FALSE);
    if (alGetError() != AL_NO_ERROR) {
		printf("ERROR setting sound source\n");
		return;
    }

    alSourcePlay(alSource);
}

void cleanup_sounds()
{
    ALCcontext *Context = alcGetCurrentContext();
    ALCdevice *Device = alcGetContextsDevice(Context);
    alcMakeContextCurrent(NULL);
    alcDestroyContext(Context);
    alcCloseDevice(Device);
}

void convertpng2ppm(void) {
	system("convert ./images/bg.jpg ./images/bg.ppm");
    system("convert ./images/dodge3d.png ./images/dodge3d.ppm");
    system("convert ./images/road.jpg ./images/road.ppm");
    system("convert ./images/walltexture.jpg ./images/walltexture.ppm");
    system("convert ./images/dead2.png ./images/dead.ppm");
    system("convert ./images/Spike.png ./images/spike.ppm");
    system("convert ./images/bomb.png ./images/bomb.ppm");
    system("convert ./images/coin.png ./images/coin.ppm");
    system("convert ./images/heart.png ./images/heart.ppm");
    system("convert ./images/Star.png ./images/star.ppm");
}

void getImage(void) {
	bgImage = ppm6GetImage("./images/bg.ppm");
    logoImage = ppm6GetImage("./images/dodge3d.ppm");
    roadImage = ppm6GetImage("./images/road.ppm");
    wallImage = ppm6GetImage("./images/walltexture.ppm");
    deadImage = ppm6GetImage("./images/dead.ppm");
    spikeImage = ppm6GetImage("./images/spike.ppm");
    bombImage = ppm6GetImage("./images/bomb.ppm");
    coinImage = ppm6GetImage("./images/coin.ppm");
    heartImage = ppm6GetImage("./images/heart.ppm");
    starImage = ppm6GetImage("./images/star.ppm");
}

void generateTextures(void) {
	glGenTextures(1, &bgTexture);
    glGenTextures(1, &logoTexture);
    glGenTextures(1, &roadTexture);
    glGenTextures(1, &wallTexture);
    glGenTextures(1, &deadTexture);
    glGenTextures(1, &spikeTexture);
    glGenTextures(1, &bombTexture);
    glGenTextures(1, &coinTexture);
    glGenTextures(1, &heartTexture);
    glGenTextures(1, &starTexture);
}


unsigned char *buildAlphaData(Ppmimage *img)
{
	// Add 4th component to RGB stream...
	int i;
	int a,b,c;
	unsigned char *newdata, *ptr;
	unsigned char *data = (unsigned char *)img->data;
	newdata = (unsigned char *)malloc(img->width * img->height * 4);
	ptr = newdata;
	for (i=0; i<img->width * img->height * 3; i+=3) {
		a = *(data+0);
		b = *(data+1);
		c = *(data+2);
		*(ptr+0) = a;
		*(ptr+1) = b;
		*(ptr+2) = c;
		*(ptr+3) = (a|b|c);
		ptr += 4;
		data += 3;
	}
	return newdata;
}

void cleanupPPM(void) {
	remove("./images.bg.ppm");
    remove("./images/dodge3d.ppm");
    remove("./images/road.ppm");
    remove("./images/walltexture.ppm");
    remove("./images/dead.ppm");
    remove("./images/spike.ppm");
    remove("./images/bomb.ppm");
    remove("./images/coin.ppm");
    remove("./images/heart.ppm");
    remove("./images/star.ppm");

}

void y_rotate(float rads, Vec in, Vec out)
{
	Vec dummy = {0, 0, 0};
	dummy[0] = in[0]*cos(rads) - in[2]*sin(rads);
	dummy[1] = in[1];
	dummy[2] = in[0]*sin(rads) + in[2]*cos(rads);
	out[0] = dummy[0];
	out[1] = dummy[1];
	out[2] = dummy[2];
}

void init()
{
	// Load the images file into a ppm structure.
	convertpng2ppm();

	// Load images into each individual Ppmimage structure
	getImage();

	// Create opengl texture elements
	generateTextures();

	int w = bgImage->width;
	int h = bgImage->height;	
	glBindTexture(GL_TEXTURE_2D, bgTexture);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	unsigned char *silhouetteData = buildAlphaData(bgImage);	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, silhouetteData);

	// Dodge logo
	w = logoImage->width;
	h = logoImage->height;	
	glBindTexture(GL_TEXTURE_2D, logoTexture);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	silhouetteData = buildAlphaData(logoImage);	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, silhouetteData);

	// Road texture
	w = roadImage->width;
	h = roadImage->height;	
	glBindTexture(GL_TEXTURE_2D, roadTexture);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	silhouetteData = buildAlphaData(roadImage);	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, silhouetteData);

	// Wall texture
	w = wallImage->width;
	h = wallImage->height;	
	glBindTexture(GL_TEXTURE_2D, wallTexture);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	silhouetteData = buildAlphaData(wallImage);	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, silhouetteData);

	// Game Over (Bloody)
	w = deadImage->width;
	h = deadImage->height;	
	glBindTexture(GL_TEXTURE_2D, deadTexture);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	silhouetteData = buildAlphaData(deadImage);	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, silhouetteData);
	free(silhouetteData);

	// 2D Spike Image
	w = spikeImage->width;
	h = spikeImage->height;	
	glBindTexture(GL_TEXTURE_2D, spikeTexture);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	silhouetteData = buildAlphaData(spikeImage);	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, silhouetteData);
	free(silhouetteData);

	// 2D Bomb Image
	w = bombImage->width;
	h = bombImage->height;	
	glBindTexture(GL_TEXTURE_2D, bombTexture);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	silhouetteData = buildAlphaData(bombImage);	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, silhouetteData);
	free(silhouetteData);

	// 2D Coin Image
	w = coinImage->width;
	h = coinImage->height;	
	glBindTexture(GL_TEXTURE_2D, coinTexture);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	silhouetteData = buildAlphaData(coinImage);	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, silhouetteData);
	free(silhouetteData);

	// 2D Heart Image
	w = heartImage->width;
	h = heartImage->height;	
	glBindTexture(GL_TEXTURE_2D, heartTexture);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	silhouetteData = buildAlphaData(heartImage);	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, silhouetteData);
	free(silhouetteData);

	// 2D Star Image
	w = starImage->width;
	h = starImage->height;	
	glBindTexture(GL_TEXTURE_2D, starTexture);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	silhouetteData = buildAlphaData(starImage);	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, silhouetteData);
	free(silhouetteData);


    Object *buildModel(const char *mname);

    // draw spikes and save their positions
    for (int i=0; i<g.num_spikes; i++) {
		spikes[i] = buildModel("images/spike.obj");
		spikes[i]->scale(0.25);
		//random_spikes[i] = rand()%11 - 5.0;
		random_spikes[i] =  (rand() / float(RAND_MAX) * 10) - 5;
		spike_collision[i][0] = random_spikes[i];
		spike_collision[i][1] = -i*2;
		spikes[i]->translate(random_spikes[i], 0, -i*2);
		spikes[i]->setColor(1.0, 1.0, 1.0);
		//printf("%f, ", random_spikes[i]);
    }
    
    // draw bombs and save their positions
    for (int i=0; i<g.num_bombs; i++) {
		bombs[i] = buildModel("images/bomb.obj");
		bombs[i]->scale(0.2);
		//random_bombs[i] = rand()%11 - 5.0;
		random_bombs[i] =  (rand() / float(RAND_MAX) * 10) - 5;
		bomb_collision[i][0] = random_bombs[i];
		bomb_collision[i][1] = -i*10;
		bombs[i]->translate(random_bombs[i], 0.6, -i*10);
		bombs[i]->setColor(0.0, 0.0, 0.0);
    }

    // draw coins and save their positions
    for (int i=0; i<g.num_coins; i++) {
		coins[i] = buildModel("images/coin.obj");
		coins[i]->scale(0.2);
		//int r = rand()%11 - 5.0;
		//random_coins[i] = r;
		random_coins[i] =  (rand() / float(RAND_MAX) * 10) - 5;
		coin_collision[i][0] = random_coins[i];
		coin_collision[i][1] = -i*10;
		coins[i]->translate(random_coins[i], 0.5, -i*10);
		coins[i]->setColor(0.68, 0.53, 0.06);
    }

    // draw extra coins
    for (int i=0; i<g.num_coins2; i++) {
    	coins2[i] = buildModel("images/coin.obj");
    	coins2[i]->scale(0.2);
    	coin2_collision[i][0] = 0;
    	coin2_collision[i][1] = -1000 - i*5;
    	coins2[i]->translate(0, 0.5, -1000 - i*5);
    	coins2[i]->setColor(0.68, 0.53, 0.06);
    }

    // draw hearts and save their positions
	for (int i=0; i<g.num_hearts; i++) {
		hearts[i] = buildModel("images/heart.obj");
		hearts[i]->scale(0.1);
		//random_hearts[i] = rand()%11 - 5.0;
		random_hearts[i] =  (rand() / float(RAND_MAX) * 10) - 5;
		heart_collision[i][0] = random_hearts[i];
		heart_collision[i][1] = -i*20;
		hearts[i]->translate(random_hearts[i], 0.7, -i*20);
		hearts[i]->setColor(1, 0, 0);
    }

    // draw stars and save their positions
    for (int i =0; i<g.num_stars; i++) {
    	stars[i] = buildModel("images/star.obj");
    	stars[i]->scale(0.1);
    	//random_stars[i] = rand()%11 - 5.0;
    	random_stars[i] =  (rand() / float(RAND_MAX) * 10) - 5;
    	star_collision[i][0] = random_stars[i];
    	star_collision[i][1] = -i*40;
    	stars[i]->translate(random_stars[i], 0.7, -i*40);
    	stars[i]->setColor(0.9, 0.9, 0.1);
    }
}

void init_opengl()
{
    //OpenGL initialization
    //glClearColor(0.0f, 0.4f, 0.5f, 0.0f);
    glClearColor(0.2, 0.6, 1.0, 0.0);
    glClearDepth(1.0);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, g.aspectRatio, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
    //Enable this so material colors are the same as vert colors.
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);
    //Turn on a light
    glLightfv(GL_LIGHT0, GL_POSITION, g.lightPosition);
    glEnable(GL_LIGHT0);
    //Do this to allow fonts
    glEnable(GL_TEXTURE_2D);

    initialize_fonts();
    //init_textures();
}

Object *buildModel(const char *mname)
{
    char line[200];
    Vec *vert=NULL;  //vertices in list
    iVec *face=NULL; //3 indicies per face
    int nv=0, nf=0;
    //printf("void buildModel(%s)...\n",mname);
    //Model exported from Blender. Assume an obj file.
    FILE *fpi = fopen(mname,"r");
    if (!fpi) {
	printf("ERROR: file **%s** not found.\n", mname);
	return NULL;
    }
    //================================================
    //count all vertices
    fseek(fpi, 0, SEEK_SET);
    while (fgets(line, 100, fpi) != NULL) {
	if (line[0] == 'v' && line[1] == ' ')
	    nv++;
    }
    vert = new Vec[nv];
    if (!vert) {
		printf("ERROR: out of mem (vert)\n");
		exit(EXIT_FAILURE);
		}
		//printf("n verts: %i\n", nv);
		//count all faces
		int iface[4];
		fseek(fpi, 0, SEEK_SET);
		while (fgets(line, 100, fpi) != NULL) {
		if (line[0] == 'f' && line[1] == ' ') {
			sscanf(line+1,"%i %i %i", &iface[0], &iface[1], &iface[2]);
			nf++;
		}
    }
    face = new iVec[nf];
    if (!face) {
		printf("ERROR: out of mem (face)\n");
		exit(EXIT_FAILURE);
		}
		//printf("n faces: %i\n", nf);
		//first pass, read all vertices
		nv=0;
		fseek(fpi, 0, SEEK_SET);
		while (fgets(line, 100, fpi) != NULL) {
		if (line[0] == 'v' && line[1] == ' ') {
			sscanf(line+1,"%f %f %f",&vert[nv][0],&vert[nv][1],&vert[nv][2]);
			nv++;
		}
    }
    //second pass, read all faces
    int comment=0;
    nf=0;
    fseek(fpi, 0, SEEK_SET);
    while (fgets(line, 100, fpi) != NULL) {
		if (line[0] == '/' && line[1] == '*') {
			comment=1;
		}
		if (line[0] == '*' && line[1] == '/') {
			comment=0;
			continue;
		}
		if (comment)
			continue;
		if (line[0] == 'f' && line[1] == ' ') {
			sscanf(line+1,"%i %i %i", &iface[0], &iface[1], &iface[2]);
			face[nf][0] = iface[1]-1;
			face[nf][1] = iface[0]-1;
			face[nf][2] = iface[2]-1;
			nf++;
		}
    }
    fclose(fpi);
    //printf("nverts: %i   nfaces: %i\n", nv, nf);
    Object *o = new Object(nv, nf);
    for (int i=0; i<nv; i++) {
		o->setVert(vert[i], i);
    }
    //opengl default for front facing is counter-clockwise.
    //now build the triangles...
    for (int i=0; i<nf; i++) {
		o->setFace(face[i], i);
    }
    delete [] vert;
    delete [] face;
    //printf("returning.\n"); fflush(stdout);
    return o;
}

Flt vecNormalize(Vec vec) {
    Flt len = vec[0]*vec[0] + vec[1]*vec[1] + vec[2]*vec[2];
    if (len == 0.0) {
	MakeVector(0.0,0.0,1.0,vec);
	return 1.0;
    }
    len = sqrt(len);
    Flt tlen = 1.0 / len;
    vec[0] *= tlen;
    vec[1] *= tlen;
    vec[2] *= tlen;
    return len;
}

void check_mouse(XEvent *e)
{
    //Did the mouse move?
    //Was a mouse button clicked?
    static int savex = 0;
    static int savey = 0;
    //
    if (e->type == ButtonRelease) {
		return;
    }
    if (e->type == ButtonPress) {
		if (e->xbutton.button==1) {
			//Left button is down
		}
		if (e->xbutton.button==3) {
			//Right button is down
		}
    }
    if (savex != e->xbutton.x || savey != e->xbutton.y) {
		//Mouse moved
		savex = e->xbutton.x;
		savey = e->xbutton.y;
    }
}

int check_keys(XEvent *e)
{
    //Was there input from the keyboard?
    if (e->type == KeyPress) {
		int key = (XLookupKeysym(&e->xkey, 0) & 0x0000ffff);
		if (g.display_attract) {
			switch(key) {
				case XK_Return:
					g.display_attract = false;
					g.display_startmenu = true;
					key = 0;
					spot[0] = 0;
					spot[1] = 0;
					spot[2] = -1.0;
					g.moveLeft = false;
					g.moveRight = false;
					break;
			}
		}
		if (g.display_startmenu || g.display_endmenu) {
			switch(key) {
				case XK_Down:
					if (g.menu_position != 5) {
						g.menu_position++;
						if (g.audio_on) {
							play_sound("./sounds/menu.wav");
						}
					}
					break;
				case XK_Up:
					if (g.menu_position != 1) {
						g.menu_position--;
						if (g.audio_on) {
							play_sound("./sounds/menu.wav");
						}
					}
					break;
				case XK_Return:
					if (g.menu_position == 1) {
						if (g.display_startmenu) {
							g.display_startmenu = false;
						} else {
							g.display_endmenu = false;
							//restart_game();
						}
						g.display_entername = true;
					} else if (g.menu_position == 2) {
						if (g.display_startmenu) {
							g.prev_menu = 0;
							g.display_startmenu = false;
						} else {
							g.prev_menu = 1;
							g.display_endmenu = false;
						}
						g.display_tutorial = true;
						key = 0;
					} else if (g.menu_position == 3) {
						if (g.display_startmenu) {
							g.prev_menu = 0;
							g.display_startmenu = false;
						} else {
							g.prev_menu = 1;
							g.display_endmenu = false;
						}
						g.display_difficulty = true;
						g.menu_position = 1;
						key = 0;
					} else if (g.menu_position == 4) {
						if (g.display_startmenu) {
							g.prev_menu = 0;
							g.display_startmenu = false;
						} else {
							// prev menu was "play again"
							g.prev_menu = 1;
							g.display_endmenu = false;
						}
						//g.display_startmenu = false;
						g.display_audiosettings = true;
						g.menu_position = 1;
						key = 0;
					} else if (g.menu_position == 5) {
						view_scores();
					}
					key = 0;
					break;
				case XK_Escape:
					return 1;
			}
		}
		if (g.display_tutorial) {
			if (key == XK_Left) {
				if (g.prev_menu == 0) {
					g.display_tutorial = false;
					g.display_startmenu = true;
				} else {
					g.display_tutorial = false;
					g.display_endmenu = true;
				}
			}
			key = 0;
		}
		if (g.display_entername) {
			onePlayerStart(key, player_name, input);
			if (key == XK_Return) {
				if (g.prev_menu == 1) {
					restart_game();
				}
		        g.display_entername = false;
		        start_countDown_timer();
				countdown_started = true;
				countdown_done = false;
			}
		}
		if (g.display_difficulty) {
			switch(key) {
				case XK_Down:
					if (g.menu_position != 3) {
						g.menu_position++;
						if (g.audio_on) {
							play_sound("./sounds/menu.wav");
						}
					}
					break;
				case XK_Up:
					if (g.menu_position != 1) {
						g.menu_position--;
						if (g.audio_on) {
							play_sound("./sounds/menu.wav");
						}
					}
					break;
				case XK_Return:
					if (g.menu_position == 1) {
						g.difficulty = "EASY";
					} else if (g.menu_position == 2) {
						g.difficulty = "MEDIUM";
					} else {
						g.difficulty = "HARD";
					}
					break;
				case XK_Left:
					g.display_difficulty = false;
					if (g.prev_menu == 0) {
						g.display_startmenu = true;
					} else {
						g.display_endmenu = true;
					}
					g.menu_position = 3;
					break;
			}
		}
		if (g.display_audiosettings) {
			switch(key) {
				case XK_Down:
					if (g.menu_position != 2) {
						g.menu_position++;
						if (g.audio_on) {
							play_sound("./sounds/menu.wav");
						}
					}
					break;
				case XK_Up:
					if (g.menu_position != 1) {
						g.menu_position--;
						if (g.audio_on) {
							play_sound("./sounds/menu.wav");
						}
					}
					break;
				case XK_Return:
					if (g.menu_position == 1 && !g.audio_on) {
						g.audio_on = true;
						continue_theme();
					} else if (g.menu_position == 2) {
						g.audio_on = false;
						stop_theme();
					}
					break;
				case XK_Left:
					g.display_audiosettings = false;
					if (g.prev_menu == 0) {
						g.display_startmenu = true;
					} else {
						g.display_endmenu = true;
					}
					g.menu_position = 4;
					break;
			}
		} if (g.display_gameover) {
			switch(key) {
				case XK_Return:
					g.display_gameover = false;
					g.display_endmenu = true;
					break;
				case XK_Escape:
					return 1;
			}
		} else {
			switch(key) {
				case XK_1:
					break;
				case XK_w:
					g.sprint = true;
					break;
				case XK_d:
					g.moveRight = true;
					break;
				case XK_a:
					g.moveLeft = true;
					break;
				case XK_space:
					if (!g.falling) {
						g.jumping = true;
					}
					break;
				case XK_Escape:
					return 1;
			}
		}
    }
    
    if (e->type == KeyRelease) {
		int key = (XLookupKeysym(&e->xkey, 0) & 0x0000ffff);
		switch(key) {
			case XK_1:
				break;
			case XK_w:
				g.sprint = false;
				break;
			case XK_d:
				g.moveRight = false;
				break;
			case XK_a:
				g.moveLeft = false;
				break;
		}
	}
    return 0;
}

void identity33(Matrix m)
{
    m[0][0] = m[1][1] = m[2][2] = 1.0f;
    m[0][1] = m[0][2] = m[1][0] = m[1][2] = m[2][0] = m[2][1] = 0.0f;
}

void yy_transform(const Vec rotate, Matrix a)
{
    //This function applies a rotation to a matrix.
    //Call this function first, then call trans_vector() to apply the
    //rotations to an object or vertex.

    if (rotate[0] != 0.0f) {
		Flt ct = cos(rotate[0]), st = sin(rotate[0]);
		Flt t10 = ct*a[1][0] - st*a[2][0];
		Flt t11 = ct*a[1][1] - st*a[2][1];
		Flt t12 = ct*a[1][2] - st*a[2][2];
		Flt t20 = st*a[1][0] + ct*a[2][0];
		Flt t21 = st*a[1][1] + ct*a[2][1];
		Flt t22 = st*a[1][2] + ct*a[2][2];
		a[1][0] = t10;
		a[1][1] = t11;
		a[1][2] = t12;
		a[2][0] = t20;
		a[2][1] = t21;
		a[2][2] = t22;
		return;
    }
    if (rotate[1] != 0.0f) {
		Flt ct = cos(rotate[1]), st = sin(rotate[1]);
		Flt t00 = ct*a[0][0] - st*a[2][0];
		Flt t01 = ct*a[0][1] - st*a[2][1];
		Flt t02 = ct*a[0][2] - st*a[2][2];
		Flt t20 = st*a[0][0] + ct*a[2][0];
		Flt t21 = st*a[0][1] + ct*a[2][1];
		Flt t22 = st*a[0][2] + ct*a[2][2];
		a[0][0] = t00;
		a[0][1] = t01;
		a[0][2] = t02;
		a[2][0] = t20;
		a[2][1] = t21;
		a[2][2] = t22;
		return;
    }
    if (rotate[2] != 0.0f) {
		Flt ct = cos(rotate[2]), st = sin(rotate[2]);
		Flt t00 = ct*a[0][0] - st*a[1][0];
		Flt t01 = ct*a[0][1] - st*a[1][1];
		Flt t02 = ct*a[0][2] - st*a[1][2];
		Flt t10 = st*a[0][0] + ct*a[1][0];
		Flt t11 = st*a[0][1] + ct*a[1][1];
		Flt t12 = st*a[0][2] + ct*a[1][2];
		a[0][0] = t00;
		a[0][1] = t01;
		a[0][2] = t02;
		a[1][0] = t10;
		a[1][1] = t11;
		a[1][2] = t12;
		return;
    }
}

void trans_vector(Matrix mat, const Vec in, Vec out)
{
    Flt f0 = mat[0][0] * in[0] + mat[1][0] * in[1] + mat[2][0] * in[2];
    Flt f1 = mat[0][1] * in[0] + mat[1][1] * in[1] + mat[2][1] * in[2];
    Flt f2 = mat[0][2] * in[0] + mat[1][2] * in[1] + mat[2][2] * in[2];
    out[0] = f0;
    out[1] = f1;
    out[2] = f2;
}

void physics()
{
	// Sprint
	if (g.sprint) {
    	g.cameraPosition[0] += spot[0]*0.09;
    	g.cameraPosition[2] += spot[2]*0.09;
	}
	
	// Jump
	if (g.jumping) {
		//g.cameraPosition[1] += 0.06;
		g.cameraPosition[1] += g.acceleration;
		if (g.acceleration > 0.0) {
			g.acceleration -= 0.005;
		}

		if (g.cameraPosition[1] >= 2.0) {
			g.jumping = false;
			g.falling = true;
			g.acceleration = 0.0;
		}
	}
	
	if (g.falling) {
		//g.cameraPosition[1] -= 0.06;
		g.cameraPosition[1] -= g.acceleration;
		g.acceleration += 0.01;
		if (g.cameraPosition[1] <= 1.0) {
			g.falling = false;
			g.acceleration = 0.1;
		}
	}
	
	// Turn left
	if (g.moveLeft && g.angle > -PI/4.0) {
		Vec newspot;
		y_rotate(-0.018, spot, newspot);
		spot[0] = newspot[0];
		spot[1] = newspot[1];
		spot[2] = newspot[2];
		g.angle -= 0.018;
	}
	
	// Turn right
	if (g.moveRight && g.angle < PI/4.0) {
		Vec newspot;
		y_rotate(0.018, spot, newspot);
		spot[0] = newspot[0];
		spot[1] = newspot[1];
		spot[2] = newspot[2];
		g.angle += 0.018;
	}
	
    // Rotate coins
    for (int i=0; i<g.num_coins; i++) {
		coins[i]->rotate(0,1,0);
    }

    // Rotate extra coins
    for (int i=0; i<g.num_coins2; i++) {
    	coins2[i]->rotate(0,1,0);
    }

	// Rotate bombs
	for (int i=0; i<g.num_bombs; i++) {
		bombs[i]->rotate(0,1,0);
	}

    // Rotate hearts
    for (int i=0; i<g.num_hearts; i++) {
		hearts[i]->rotate(0,1,0);
	}

	// Rotate stars
	for (int i=0; i<g.num_stars; i++) {
		stars[i]->rotate(1,0,0);
	}

    int xdist, zdist;
    int x_bomb_dist, z_bomb_dist;
    int x_coin_dist, z_coin_dist;
    int x_coin2_dist, z_coin2_dist;
    int x_heart_dist, z_heart_dist;
    int x_star_dist, z_star_dist;
    
	if (countdown_started) {
        countdown_done = check_countDown_timer();
	}

	if (countdown_done) {
		// move player
		if (g.difficulty == "EASY") {
	    	g.cameraPosition[0] += spot[0]*0.16;
	    	g.cameraPosition[2] += spot[2]*0.16;
	    } else if (g.difficulty == "MEDIUM") {
	    	g.cameraPosition[0] += spot[0]*0.24;
	    	g.cameraPosition[2] += spot[2]*0.24;
	    } else {
	    	g.cameraPosition[0] += spot[0]*0.32;
	    	g.cameraPosition[2] += spot[2]*0.32;
	    }
    }
    
    // Check for wall collision
    if (g.cameraPosition[0] < -4.8) {
		g.cameraPosition[0] = -4.8;
		Vec newspot;
		y_rotate(0.01, spot, newspot);
		spot[0] = newspot[0];
		spot[1] = newspot[1];
		spot[2] = newspot[2];
		g.angle += 0.01;
	}
	
	if (g.cameraPosition[0] > 4.8) {
		g.cameraPosition[0] = 4.8;
		Vec newspot;
		y_rotate(-0.01, spot, newspot);
		spot[0] = newspot[0];
		spot[1] = newspot[1];
		spot[2] = newspot[2];
		g.angle -= 0.01;
	}

	// Check for collision with spikes
    for (int i = 0; i < g.num_spikes; i++) {
		xdist = fabs(g.cameraPosition[0] - spike_collision[i][0]);
		zdist = fabs(g.cameraPosition[2] - spike_collision[i][1]);
		if (xdist <= 0.06 && zdist <= 0.06 && (!g.jumping && !g.falling)) {
			if (g.health > 0 && !g.invincible) {
				g.health--;
				if (g.audio_on) {
					play_sound("./sounds/ouch.wav");
				}
				if (g.health == 0) {
					gamelog(player_name, g.coins, g.difficulty);
					g.display_gameover = true;
				}
			}
			for (int j = i; j < g.num_spikes - 1; j++) {
				spike_collision[j][0] = spike_collision[j+1][0];
				spike_collision[j][1] = spike_collision[j+1][1];
				spikes[j] = spikes[j+1];
			}
			g.num_spikes--;
		}
    }
    
    // Check for collision with bombs
	for (int i = 0; i < g.num_bombs; i++) {
		x_bomb_dist = fabs(g.cameraPosition[0] - bomb_collision[i][0]);
		z_bomb_dist = fabs(g.cameraPosition[2] - bomb_collision[i][1]);
		if (x_bomb_dist <= 0.06 && z_bomb_dist <= 0.06) {
			if (g.health > 0 && !g.invincible) {
				g.health--;
				if (g.audio_on) {
					play_sound("./sounds/explosion.wav");
				}
				if (g.health == 0) {
					gamelog(player_name, g.coins, g.difficulty);
					g.display_gameover = true;
				}
			}
			for (int j = i; j < g.num_bombs - 1; j++) {
				bomb_collision[j][0] = bomb_collision[j+1][0];
				bomb_collision[j][1] = bomb_collision[j+1][1];
				bombs[j] = bombs[j+1];
			}
			g.num_bombs--;
		}
    }
    
    // Check for collision with coins
    for (int i = 0; i < g.num_coins; i++) {
		x_coin_dist = fabs(g.cameraPosition[0] - coin_collision[i][0]);
		z_coin_dist = fabs(g.cameraPosition[2] - coin_collision[i][1]);
		if (x_coin_dist <= 0.1 && z_coin_dist <= 0.1) {
			g.coins++;
			if (g.audio_on) {
				play_sound("./sounds/chaching.wav");
			}
			for (int j = i; j < g.num_coins - 1; j++) {
				coin_collision[j][0] = coin_collision[j+1][0];
				coin_collision[j][1] = coin_collision[j+1][1];
				coins[j] = coins[j+1];
			}
			g.num_coins--;
		}
    }

    // Check for collision with extra coins
    for (int i = 0; i < g.num_coins2; i++) {
		x_coin2_dist = fabs(g.cameraPosition[0] - coin2_collision[i][0]);
		z_coin2_dist = fabs(g.cameraPosition[2] - coin2_collision[i][1]);
		if (x_coin2_dist <= 0.1 && z_coin2_dist <= 0.1) {
			g.coins++;
			if (g.audio_on) {
				play_sound("./sounds/chaching.wav");
			}
			for (int j = i; j < g.num_coins2 - 1; j++) {
				coin2_collision[j][0] = coin2_collision[j+1][0];
				coin2_collision[j][1] = coin2_collision[j+1][1];
				coins2[j] = coins2[j+1];
			}
			g.num_coins2--;
		}
    }

    // Check for collision with hearts
    for (int i = 0; i < g.num_hearts; i++) {
		x_heart_dist = fabs(g.cameraPosition[0] - heart_collision[i][0]);
		z_heart_dist = fabs(g.cameraPosition[2] - heart_collision[i][1]);
		if (x_heart_dist <= 0.1 && z_heart_dist <= 0.1) {
			if (g.health != 4) {
				g.health++;
			}
			if (g.audio_on) {
				play_sound("./sounds/health-pickup.wav");
			}
			for (int j = i; j < g.num_hearts - 1; j++) {
				heart_collision[j][0] = heart_collision[j+1][0];
				heart_collision[j][1] = heart_collision[j+1][1];
				hearts[j] = hearts[j+1];
			}
			g.num_hearts--;
		}
    }

    // Check for collision with stars
    for (int i = 0; i < g.num_stars; i++) {
		x_star_dist = fabs(g.cameraPosition[0] - star_collision[i][0]);
		z_star_dist = fabs(g.cameraPosition[2] - star_collision[i][1]);
		if (x_star_dist <= 0.1 && z_star_dist <= 0.1) {
			g.invincible = start_powerup_timer();
			if (g.audio_on) {
				play_sound("./sounds/powerup01.wav");
			}
			for (int j = i; j < g.num_stars - 1; j++) {
				star_collision[j][0] = star_collision[j+1][0];
				star_collision[j][1] = star_collision[j+1][1];
				stars[j] = stars[j+1];
			}
			g.num_stars--;
		}
    }

    if (g.cameraPosition[2] <= -1050) {
    	g.display_gameover = true;
    	gamelog(player_name, g.coins, g.difficulty);
    }
}

void render()
{
	g.invincible = check_powerup_timer(g.invincible);
	//unsigned int yellow = 0x00011aa11;
	unsigned int black = 0x000000;

    Rect r;
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    //3D mode
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    gluPerspective(45.0f, g.aspectRatio, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	Vec spot2;
    spot2[0] = g.cameraPosition[0] + spot[0];
    spot2[1] = g.cameraPosition[1] + spot[1];
    spot2[2] = g.cameraPosition[2] + spot[2];
    gluLookAt(g.cameraPosition[0], g.cameraPosition[1], g.cameraPosition[2],
	    spot2[0], spot2[1], spot2[2], 0,1,0);
    glLightfv(GL_LIGHT0, GL_POSITION, g.lightPosition);  

    float furthest_d = g.cameraPosition[2] - 80;
    float behind_pos = g.cameraPosition[2] + 10;

    // Draw spikes
    for (int i=0; i<g.num_spikes; i++) {
		if (spike_collision[i][1] > furthest_d && spike_collision[i][1] < behind_pos) {
			spikes[i]->draw();
		}
    }

    // Draw bombs
    for (int i=0; i<g.num_bombs; i++) {
		if (bomb_collision[i][1] > furthest_d && bomb_collision[i][1] < behind_pos) {
			bombs[i]->draw();
		}
    }

    // Draw coins
    for (int i=0; i<g.num_coins; i++) {
    	if (coin_collision[i][1] > furthest_d && coin_collision[i][1] < behind_pos) {
			coins[i]->draw();
		}
    }

    // Draw extra coins
    for (int i=0; i<g.num_coins2; i++) {
    	if (coin2_collision[i][1] > furthest_d && coin2_collision[i][1] < behind_pos) {
    		coins2[i]->draw();
    	}
    }

    // Draw hearts
    for (int i=0; i<g.num_hearts; i++) {
		if (heart_collision[i][1] > furthest_d && heart_collision[i][1] < behind_pos) {
			hearts[i]->draw();
		}
    }

    // Draw stars
    for (int i=0; i<g.num_stars; i++) {
    	if (star_collision[i][1] > furthest_d && star_collision[i][1] < behind_pos) {
    		stars[i]->draw();
    	}
    }


	glDisable(GL_LIGHTING);

	// SKY TEXTURE
	glPushMatrix();
	glColor3f(1.0, 1.0, 1.0);
	glTranslatef(0.0f, -50.0, g.cameraPosition[2] - 75.0f);
	glBindTexture(GL_TEXTURE_2D, bgTexture);
	glBegin(GL_QUADS);
		glTexCoord2f(1.0, 10.0); glVertex2i(-150.0, -150.0);
		glTexCoord2f(1.0, 1.0); glVertex2i(-150.0, 150.0);
		glTexCoord2f(10.0, 1.0); glVertex2i(150.0, 150.0);
		glTexCoord2f(10.0, 10.0); glVertex2i(150.0, -150.0);
	glEnd();
	glPopMatrix();


	// ROAD TEXTURE
	glPushMatrix();
    glColor3f(1.0f, 1.0f, 1.0f);
    glTranslatef(0.0f, -1.0, 1.0f);
    glBindTexture(GL_TEXTURE_2D, roadTexture);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 1.0);	glVertex3f(5.0, 1.0, -1000.0);
    glTexCoord2f(0.0, 0.0); glVertex3f(-5.0, 1.0, -1000.0);
    glTexCoord2f(100.0, 0.0); glVertex3f(-5.0, 1.0, 20.0);
    glTexCoord2f(100.0, 1.0);	glVertex3f(5.0, 1.0, 20.0);
    glEnd();
    glPopMatrix();


    // LEFT WALL TEXTURE
	glColor3f(1.0f, 1.0f, 1.0f);
    glPushMatrix();
    glTranslatef(-6.0, 2.0, -5.0);
    glBindTexture(GL_TEXTURE_2D, wallTexture);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 2.0);	glVertex3f(1.0, 0.0, -1000.0);
    glTexCoord2f(0.0f, 0.0f);	glVertex3f(1.0, -5.0, -1000.0);
    glTexCoord2f(204.0, 0.0f);	glVertex3f(1.0, -5.0, 20.0);
    glTexCoord2f(204.0, 2.0);	glVertex3f(1.0, 0.0, 20.0);
    glEnd();
    glPopMatrix();


    // RIGHT WALL TEXTURE
	glColor3f(1.0f, 1.0f, 1.0f);
    glPushMatrix();
    glTranslatef(4.0, 2.0, -5.0);
    glBindTexture(GL_TEXTURE_2D, wallTexture);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 2.0); glVertex3f(1.0, 0.0, -1000.0);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0, -5.0, -1000.0);
    glTexCoord2f(204.0, 0.0f); glVertex3f(1.0, -5.0, 20.0);
    glTexCoord2f(204.0, 2.0); glVertex3f(1.0, 0.0, 20.0);
    glEnd();
    glPopMatrix();

    // Switch to 2D mode
    glViewport(0, 0, g.xres, g.yres);
    glMatrixMode(GL_MODELVIEW);   glLoadIdentity();
    glMatrixMode (GL_PROJECTION); glLoadIdentity();
    gluOrtho2D(0, g.xres, 0, g.yres);
    glPushAttrib(GL_ENABLE_BIT);
    glDisable(GL_LIGHTING);
    
    // Display Dodge3D logo
    if (g.display_attract || g.display_startmenu || g.display_tutorial ||
    	g.display_audiosettings || g.display_endmenu) {
	    glColor3f(1.0f, 1.0f, 1.0f);
	    glPushMatrix();
	    glTranslatef(g.xres/2, g.yres - g.yres/10, 0);
	    glBindTexture(GL_TEXTURE_2D, logoTexture);
	    glEnable(GL_ALPHA_TEST);
	    glAlphaFunc(GL_GREATER, 0.0f);
	    glColor4ub(255,255,255,255);
	    glBegin(GL_QUADS);
	    glTexCoord2f(0.0f, 1.0f); glVertex2i(-140,-42);
	    glTexCoord2f(0.0f, 0.0f); glVertex2i(-140, 42);
	    glTexCoord2f(1.0f, 0.0f); glVertex2i( 140, 42);
	    glTexCoord2f(1.0f, 1.0f); glVertex2i( 140,-36);
	    glEnd();
	    glPopMatrix();
	}

	// Attraction Mode
	if (g.display_attract) {
		blinkon = check_text_timer(blinkon);
	    blinkoff = check_notext_timer(blinkoff);
	    if (blinkon){
	    	Rect i;
		    i.bot = g.yres - g.yres/5;
		    i.left = g.xres/2;
		    i.center = g.yres/2;
		    ggprint16(&i, 20, black, "Press 'Enter' for menu");
	    }
		if (g.moveLeft && g.angle > -PI/10.0) {
			Vec newspot;
			y_rotate(-0.01, spot, newspot);
			spot[0] = newspot[0];
			spot[1] = newspot[1];
			spot[2] = newspot[2];
			g.angle -= 0.01;
		} else {
			g.moveLeft = false;
			g.moveRight = true;
		}
		if (g.moveRight && g.angle < PI/10.0) {
			Vec newspot;
			y_rotate(0.01, spot, newspot);
			spot[0] = newspot[0];
			spot[1] = newspot[1];
			spot[2] = newspot[2];
			g.angle += 0.01;
		} else {
			g.moveRight = false;
			g.moveLeft = true;
		}

	// Start Menu
    } else if (g.display_startmenu) {
    	r.center = 0;
		r.bot = g.yres - (g.yres/5);
		r.left = g.xres/2 - 50;
		ggprint13(&r, 16, black, "START");
		ggprint13(&r, 16, black, "TUTORIAL");
		ggprint13(&r, 16, black, "DIFFICULTY");
		ggprint13(&r, 16, black, "AUDIO SETTINGS");
		ggprint13(&r, 16, black, "VIEW SCORES");

		r.bot = g.yres - g.yres/5 - 16*(g.menu_position-1);
		r.left = g.xres/2 - 75;
		ggprint13(&r, 16, black, ">>");

	// Input Name
	} else if (g.display_entername) {
		player1Name(player_name, input);

	// Tutorial
	} else if (g.display_tutorial) {
		r.center = 0;
		r.bot = g.yres - (g.yres/5);
		r.left = g.xres/2 - 150;
		ggprint13(&r, 16, black, "Key controls:");
		ggprint12(&r, 16, black, "A - turn left");
		ggprint12(&r, 16, black, "D - turn right");
		ggprint12(&r, 16, black, "W - sprint");
		ggprint12(&r, 16, black, "Spacebar - jump");

		r.bot = g.yres - (g.yres/5);
		r.left = g.xres/2 + 50;
		ggprint13(&r, 16, black, "Items:");
		ggprint12(&r, 16, black, "Spike - hurts player (-25%% hp)");
		ggprint12(&r, 16, black, "Bomb - hurts player (-25%% hp)");
		ggprint12(&r, 16, black, "Coin - increases score");
		ggprint12(&r, 16, black, "Heart - replenishes health (+25%% hp)");
		ggprint12(&r, 16, black, "Star - invincibility for 3 seconds");

		// Display 2D Spike
		glColor3f(1.0f, 1.0f, 1.0f);
	    glPushMatrix();
	    glTranslatef(g.xres/2 + 30, g.yres - g.yres/5 - 5, 0);
	    glBindTexture(GL_TEXTURE_2D, spikeTexture);
	    glEnable(GL_ALPHA_TEST);
	    glAlphaFunc(GL_GREATER, 0.0f);
	    glColor4ub(255,255,255,255);
	    glBegin(GL_QUADS);
	    glTexCoord2f(0.0f, 1.0f); glVertex2i(-8,-8);
	    glTexCoord2f(0.0f, 0.0f); glVertex2i(-8, 8);
	    glTexCoord2f(1.0f, 0.0f); glVertex2i( 8, 8);
	    glTexCoord2f(1.0f, 1.0f); glVertex2i( 8,-8);
	    glEnd();
	    glPopMatrix();

	    // Display 2D Bomb
		glColor3f(1.0f, 1.0f, 1.0f);
	    glPushMatrix();
	    glTranslatef(g.xres/2 + 30, g.yres - g.yres/5 - 21, 0);
	    glBindTexture(GL_TEXTURE_2D, bombTexture);
	    glEnable(GL_ALPHA_TEST);
	    glAlphaFunc(GL_GREATER, 0.0f);
	    glColor4ub(255,255,255,255);
	    glBegin(GL_QUADS);
	    glTexCoord2f(0.0f, 1.0f); glVertex2i(-8,-8);
	    glTexCoord2f(0.0f, 0.0f); glVertex2i(-8, 8);
	    glTexCoord2f(1.0f, 0.0f); glVertex2i( 8, 8);
	    glTexCoord2f(1.0f, 1.0f); glVertex2i( 8,-8);
	    glEnd();
	    glPopMatrix();

	    // Display 2D Coin
		glColor3f(1.0f, 1.0f, 1.0f);
	    glPushMatrix();
	    glTranslatef(g.xres/2 + 30, g.yres - g.yres/5 - 37, 0);
	    glBindTexture(GL_TEXTURE_2D, coinTexture);
	    glEnable(GL_ALPHA_TEST);
	    glAlphaFunc(GL_GREATER, 0.0f);
	    glColor4ub(255,255,255,255);
	    glBegin(GL_QUADS);
	    glTexCoord2f(0.0f, 1.0f); glVertex2i(-8,-8);
	    glTexCoord2f(0.0f, 0.0f); glVertex2i(-8, 8);
	    glTexCoord2f(1.0f, 0.0f); glVertex2i( 8, 8);
	    glTexCoord2f(1.0f, 1.0f); glVertex2i( 8,-8);
	    glEnd();
	    glPopMatrix();

	    // Display 2D Heart
		glColor3f(1.0f, 1.0f, 1.0f);
	    glPushMatrix();
	    glTranslatef(g.xres/2 + 30, g.yres - g.yres/5 - 53, 0);
	    glBindTexture(GL_TEXTURE_2D, heartTexture);
	    glEnable(GL_ALPHA_TEST);
	    glAlphaFunc(GL_GREATER, 0.0f);
	    glColor4ub(255,255,255,255);
	    glBegin(GL_QUADS);
	    glTexCoord2f(0.0f, 1.0f); glVertex2i(-8,-8);
	    glTexCoord2f(0.0f, 0.0f); glVertex2i(-8, 8);
	    glTexCoord2f(1.0f, 0.0f); glVertex2i( 8, 8);
	    glTexCoord2f(1.0f, 1.0f); glVertex2i( 8,-8);
	    glEnd();
	    glPopMatrix();

	    // Display 2D Star
		glColor3f(1.0f, 1.0f, 1.0f);
	    glPushMatrix();
	    glTranslatef(g.xres/2 + 30, g.yres - g.yres/5 - 69, 0);
	    glBindTexture(GL_TEXTURE_2D, starTexture);
	    glEnable(GL_ALPHA_TEST);
	    glAlphaFunc(GL_GREATER, 0.0f);
	    glColor4ub(255,255,255,255);
	    glBegin(GL_QUADS);
	    glTexCoord2f(0.0f, 1.0f); glVertex2i(-8,-8);
	    glTexCoord2f(0.0f, 0.0f); glVertex2i(-8, 8);
	    glTexCoord2f(1.0f, 0.0f); glVertex2i( 8, 8);
	    glTexCoord2f(1.0f, 1.0f); glVertex2i( 8,-8);
	    glEnd();
	    glPopMatrix();

	} else if (g.display_difficulty) {
		r.center = 0;
		r.bot = g.yres - (g.yres/5);
		r.left = g.xres/2 - 50;
		ggprint13(&r, 16, black, "EASY");
		ggprint13(&r, 16, black, "MEDIUM");
		ggprint13(&r, 16, black, "HARD");

		r.bot = g.yres - (g.yres/5) - 55;
		ggprint13(&r, 16, black, "Difficulty is currently: %s", g.difficulty.c_str());

		r.bot = g.yres - g.yres/5 - 16*(g.menu_position-1);
		r.left = g.xres/2 - 75;
		ggprint13(&r, 16, black, ">>");

	// Audio Settings
    } else if (g.display_audiosettings) {
    	r.center = 0;
		r.bot = g.yres - (g.yres/5);
		r.left = g.xres/2 - 50;
		ggprint13(&r, 16, black, "AUDIO ON");
		ggprint13(&r, 16, black, "AUDIO OFF");

		r.bot = g.yres - (g.yres/5) - 55;
		if (g.audio_on) {
			ggprint13(&r, 16, black, "Audio is currently: ON");
		} else {
			ggprint13(&r, 16, black, "Audio is currently: OFF");
		}

		r.bot = g.yres - g.yres/5 - 16*(g.menu_position-1);
		r.left = g.xres/2 - 75;
		ggprint13(&r, 16, black, ">>");

	// Game Over Display
	} else if (g.display_gameover) {
	    glColor3f(1.0f, 1.0f, 1.0f);
	    glPushMatrix();
	    glTranslatef(g.xres/2, g.yres - (g.yres/10), 0);
	    glBindTexture(GL_TEXTURE_2D, deadTexture);
	    glEnable(GL_ALPHA_TEST);
	    glAlphaFunc(GL_GREATER, 0.0f);
	    glColor4ub(255,255,255,255);
	    glBegin(GL_QUADS);
	    glTexCoord2f(0.0f, 1.0f); glVertex2i(-120,-36);
	    glTexCoord2f(0.0f, 0.0f); glVertex2i(-120, 36);
	    glTexCoord2f(1.0f, 0.0f); glVertex2i( 120, 36);
	    glTexCoord2f(1.0f, 1.0f); glVertex2i( 120,-36);
	    glEnd();
	    glPopMatrix();
		r.bot = g.yres - g.yres/5;
		r.left = g.xres/2 - 60;
		r.center = 0;
		ggprint16(&r, 16, black, "Your score: %d", g.coins);
		blinkon = check_text_timer(blinkon);
	    blinkoff = check_notext_timer(blinkoff);
	    if (blinkon){
	    	Rect i;
		    i.bot = g.yres - g.yres/4;
		    i.left = g.xres/2;
		    i.center = g.yres/2;
		    ggprint16(&i, 20, black, "Press 'Enter' for menu");
	    }

	// Game Over Menu
	} else if (g.display_endmenu) {
		g.prev_menu = 1;
		r.center = 0;
		r.bot = g.yres - g.yres/5;
		r.left = g.xres/2 - 50;
		ggprint13(&r, 16, black, "PLAY AGAIN");
		ggprint13(&r, 16, black, "TUTORIAL");
		ggprint13(&r, 16, black, "DIFFICULTY");
		ggprint13(&r, 16, black, "AUDIO SETTINGS");
		ggprint13(&r, 16, black, "VIEW SCORES");
		r.bot = g.yres - g.yres/5 - 16*(g.menu_position-1);
		r.left = g.xres/2 - 75;
		ggprint13(&r, 16, black, ">>");

	// Game HUD
	} else {
		// Display player name
		r.bot = g.yres - (g.yres/5) + 33;
		r.left = g.xres/2 - 75;
		r.center = 0;
		ggprint16(&r, 16, 0x000000, "%s", player_name);

		// Display player health
		int w = 40;
		int h = 30;
		for (int i = 0; i < g.health; i++) { 
			glPushMatrix();
			glTranslatef((g.xres/2 - 75) + 40*i, g.yres-(g.yres/5), 0);
			glBegin(GL_QUADS);
			if (!g.invincible) {
				glColor3f(0.6, 0, 0);
			} else {
				glColor3f(1.0, 1.0, 0.2);
			}
			glVertex2f(0, 0);
			glVertex2f(0, h);
			glVertex2f(w, h);
			glVertex2f(w, 0);       
			glEnd();
			glPopMatrix();
		}

		for (int i = 4; i > g.health; i--) { 
			glPushMatrix();
			glTranslatef((g.xres/2 - 75) + 40*(i-1), g.yres-(g.yres/5), 0);
			glBegin(GL_QUADS);
			glColor3f(0, 0, 0);
			glVertex2f(0, 0);
			glVertex2f(0, h);
			glVertex2f(w, h);
			glVertex2f(w, 0);       
			glEnd();
			glPopMatrix();
		}

		// Display coin count
		r.bot = g.yres - (g.yres/5) - 25;
		r.left = g.xres/2 - 75;
		r.center = 0;
		ggprint16(&r, 17, 0x00776600, "Coins: %d", g.coins);
    }

	// Show game mode
    r.bot = g.yres - 25;
    r.left = g.xres - 75;
    r.center = 0;
    ggprint13(&r, 17, 0x000000, "%s", g.difficulty.c_str());

    // Show frames per second
	fps_counter++;
	clock_gettime(CLOCK_REALTIME, &fps_current);
	double diff = timeDiff(&fps_start, &fps_current);
    float fps = fps_counter / diff;
    if (diff >= 1) {
    	fps_counter = 0;
    	clock_gettime(CLOCK_REALTIME, &fps_start);
    }

    /*r.bot = g.yres - 42;
    r.left = g.xres - 75;
    r.center = 0;*/
    ggprint13(&r, 17, 0x00998822, "fps: %.2f", fps);

    glPopAttrib();
}

void onePlayerStart(int key, char player_name[], Input &input)
{
    getName_player1(key, input);
    assign_namep1(player_name, input);
    if (keys[XK_Return] && player_name[0] != '\0') {
        g.display_entername = false;
        start_countDown_timer();
		countdown_started = true;
		countdown_done = false;
    }
} 

void player1Name(char player_name[], Input &input)
{
    unsigned int black = 0x000000;

    Rect p;

    p.bot = g.yres - g.yres/8;
    p.left = g.xres/2 - 60;
    p.center = 0;
    ggprint16(&p, 20, black, "Input your name:");

    p.bot = g.yres - g.yres/8 - 30;
    p.left = g.xres/2 - 60;
    p.center = 0;
    ggprint16(&p, 20, black, "%s", player_name);

    p.bot = g.yres - g.yres/8 - 60;
    p.left = g.xres/2 - 60;
    p.center = 0;
    ggprint16(&p, 20, black, "Press 'Enter' to Play!");
  
  	// go back to previous menu
    if (keys[XK_Left]) {
        if (g.prev_menu == 0) {
            g.display_entername = false;
            g.display_startmenu = true;
        } else {
            if (player_name == '\0') {
                return;
            } else {
                g.display_entername = false;
                g.display_endmenu = true;
            }
        }
    }
}

void getName_player1(int key, Input &input)
{
    if (key >= XK_a && key <= XK_z) {
        char k[2];
        k[0] = key;
        k[1] = '\0';
        strcat(input.player1, k);
        return;
    }
    if (key == XK_BackSpace) {
        int slen = strlen(input.player1);
        if (slen > 0)
            input.player1[slen - 1] = '\0';
        return;
    }
}

void assign_namep1(char player_name[], Input &input)
{
    for (int i = 0; i < 100; i++)
        player_name[i] = input.player1[i];
}


bool start_text_timer() 
{
    clock_gettime(CLOCK_REALTIME, &txt_start);
    return true;
}

bool check_text_timer(bool txt) 
{
    if (txt == false) 
		return false;
    clock_gettime(CLOCK_REALTIME, &txt_current);
    int timediff = txt_current.tv_sec - txt_start.tv_sec;
    if (timediff < 1) 
		return true;
    else {
		blinkoff = start_notext_timer(); 
		return false;	
    }
}

bool start_notext_timer() {
    clock_gettime(CLOCK_REALTIME, &txt_start);
    return true;
}
bool check_notext_timer(bool txt) {
    if (txt == false) 
		return false;
    clock_gettime(CLOCK_REALTIME, &txt_current);
    int timediff = txt_current.tv_sec - txt_start.tv_sec;
    if (timediff < 1) 
		return true;
    else {
		blinkon = start_text_timer();
	return false;
    }	
}

void start_countDown_timer() 
{
    clock_gettime(CLOCK_REALTIME, &countdown_start);
    one = true;
    two = true;
    three = true;
    four = true;
}

bool check_countDown_timer() 
{    
    clock_gettime(CLOCK_REALTIME, &countdown_current);
    int timediff = countdown_current.tv_sec - countdown_start.tv_sec;
    if (timediff < 1) {
		if (three) {
			if (g.audio_on) {
		    	play_sound("./sounds/three.wav");
		    }
		    three = false;
		}
		countDown3();
		return false;
    }
    
    if (timediff < 2 && timediff >= 1) {
		if (two) {
			if (g.audio_on) {
		    	play_sound("./sounds/two.wav");
		    }
		    two = false;
		}
		countDown2();
		return false;
    }

    if (timediff < 3 && timediff >= 2) {
		if (one) {
			if (g.audio_on) {
		    	play_sound("./sounds/one.wav");
		    }
		    one = false;
		}
		countDown1();
		return false;
    }

    if (timediff < 4 && timediff >= 3) {
		if (four) {
			if (g.audio_on) {
		    	play_sound("./sounds/start.wav");
		    }
		    four = false;
		}
		countDown0();
		return true;
    }
    
    return true;
}

void countDown3()
{
    //unsigned int yellow = 0x00dddd00;
    unsigned int black = 0x000000;
    Rect i;

    i.bot = g.yres - (g.yres/3);
    i.left = g.xres/2 - 10;
    i.center = 0;
    ggprint16(&i, 20, black, "  3");
}

void countDown2()
{
    //unsigned int yellow = 0x00dddd00;
    unsigned int black = 0x000000;
    Rect i;

    i.bot = g.yres - (g.yres/3);
    i.left = g.xres/2 - 10;
    i.center = 0;
    ggprint16(&i, 20, black, "  2");
}

void countDown1()
{
    //unsigned int yellow = 0x00dddd00;
    unsigned int black = 0x000000;
    Rect i;

    i.bot = g.yres - (g.yres/3);
    i.left = g.xres/2 - 10;
    i.center = 0;
    ggprint16(&i, 20, black, "  1");
}

void countDown0()
{
    //unsigned int yellow = 0x00dddd00;
    unsigned int black = 0x000000;
    Rect i;

    i.bot = g.yres - (g.yres/3);
    i.left = g.xres/2 - 23;
    i.center = 0;
    ggprint16(&i, 20, black, "START!!");
}

bool start_powerup_timer() {
    clock_gettime(CLOCK_REALTIME, &power_start);
    return true;
}
bool check_powerup_timer(bool powerup) {
    if (powerup == false) {
		return false;
    }
    clock_gettime(CLOCK_REALTIME, &power_current);
    int timediff = power_current.tv_sec - power_start.tv_sec;
    if (timediff < 3) {	
		return true;
    } else {
		return false;	
    }
}

void restart_game()
{
	g.health = 4;
	g.coins = 0;
	g.angle = 0;
	MakeVector(0.0, 1.0, 15.0, g.cameraPosition);
	g.dir[0] = 0.0 - g.cameraPosition[0];
	g.dir[1] = 0.0 - g.cameraPosition[1];
	g.dir[2] = 0.0 - g.cameraPosition[2];
	spot[0] = 0;
	spot[1] = 0;
	spot[2] = -1;
	init();
}

void gamelog(string player, int score, string diff)
{
    stringstream ss;
    ss << score;
    string score_str = ss.str();

    string command =
	"curl http://cs.csubak.edu/\\~koverstreet/4490/dodge3d/update_scores.php";
    
    if (player != "") {
		command += "\\?name=" + player;
	} else {
		command += "\\?name=unknown";
	}
    command += "\\&score=" + score_str;
    command += "\\&diff=" + diff;

    system(command.c_str());
}

void view_scores()
{
    system("firefox cs.csubak.edu/~koverstreet/4490/dodge3d/scores.html");
}