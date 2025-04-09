/* See LICENSE file for copyright and license details. */

#include <time.h>
#define SESSION_FILE "/tmp/dwm-session"

/* appearance */
static const unsigned int borderpx  = 1;        /* border pixel of windows */
static const int startwithgaps[]    = { 5 };	/* 1 means gaps are used by default, this can be customized for each tag */
static const unsigned int gappx[]   = { 10 };   /* default gap between windows in pixels, this can be customized for each tag */
static const unsigned int snap      = 32;       /* snap pixel */
static const unsigned int systraypinning = 0;   /* 0: sloppy systray follows selected monitor, >0: pin systray to monitor X */
static const unsigned int systrayonleft = 0;    /* 0: systray in the right corner, >0: systray on left of status text */
static const unsigned int systrayspacing = 2;   /* systray spacing */
static const int systraypinningfailfirst = 1;   /* 1: if pinning fails, display systray on the first monitor, False: display systray on the last monitor*/
static const int showsystray        = 1;        /* 0 means no systray */
static const int swallowfloating    = 0;        /* 1 means swallow floating windows by default */
static const int showbar            = 1;        /* 0 means no bar */
static const int topbar             = 0;        /* 0 means bottom bar */
static char *fonts[] = {
	"BlexMono Nerd Font:size=11",
  "Noto Color Emoji:size=11",
  "Font Awesome 6 Free:size=11",
  "Font Awesome 6 Brands:size=11",
  "Material Icons:size=11",
};
static char dmenufont[] = "BlexMono Nerd Font:size=11";
static char normbgcolor[] = "#2E3440";
static char normbordercolor[] = "#181616";
static char normfgcolor[] = "#ECEFF4";
static char selfgcolor[] = "#2E3440";
static char selbordercolor[] = "#aaaaaa";
static char selbgcolor[] = "#88C0D0";

static char *colors[][3] = {
	/*               fg           bg           border   */
	[SchemeNorm] = { normfgcolor, normbgcolor, normbordercolor },
	[SchemeSel] =  { selfgcolor,  selbgcolor,  selbordercolor },
};

typedef struct {
	const char *name;
	const void *cmd;
} Sp;
const char *spcmd1[] = {"ghostty", "--class=com.sc.yazi-float", "--title=yazi_float", "--window-width=200", "--window-height=50", "-e", "yazi", NULL};
const char *spcmd2[] = {"bitwarden", NULL };
static Sp scratchpads[] = {
	/* name          cmd  */
	{"spyazi",      spcmd1},
	{"bitwarden",   spcmd2},
};

/* tagging */
static const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };
static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */

	/* class               instance  title   tags mask  isfloating  isterminal  noswallow  monitor */
	{ "Gimp",	             NULL,		 NULL,		0,				 1,			     0,          1,         -1 },
	{ "Firefox",           NULL,		 NULL,		1 << 8,		 0,			     0,          1,         -1 },
  { "ghostty",           NULL,     NULL,    0,         0,          1,          0,         -1 },
	{ "com.sc.yazi-float", NULL,     NULL,    SPTAG(0),  1,          0,          1,         -1 },
	{ "Bitwarden",         NULL,     NULL,    SPTAG(1),  1,          0,          -1,        -1 },
};

/* layout(s) */
static const float mfact     = 0.5; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 1;    /* 1 means respect size hints in tiled resizals */
static const int lockfullscreen = 1; /* 1 will force focus on the fullscreen window */

static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "[]=",      tile },    /* first entry is default */
	{ "><>",      NULL },    /* no layout function means floating behavior */
	{ "[M]",      monocle },
};

/* key definitions */
#define MODKEY Mod4Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[] = { 
  "dmenu_run", "-b",	    "-m",  dmenumon,
  "-fn",       dmenufont,   "-nb", normbgcolor,
  "-nf",       normfgcolor, "-sb", selbgcolor,
  "-sf",       selfgcolor,  NULL 
};
static const char *dPowermenu[] = { 
  "dpowermenu.sh", "-b",	  "-m",
  dmenumon,	     "-fn",	  dmenufont,
  "-nb",	     normbgcolor, "-nf",
  normfgcolor,     "-sb",	  selbgcolor,
  "-sf",	     selfgcolor,  NULL
};
static const char *dmonitor[] = { 
  "dmonitor.sh", "-b",	      "-m",
  dmenumon,	 "-fn",	      dmenufont,
  "-nb",	 normbgcolor, "-nf",
  normfgcolor,	 "-sb",	      selbgcolor,
  "-sf",	 selfgcolor,  NULL
};

static const char *termcmd[]  = { "ghostty", NULL };
static const char *kcalc[2] = { "kcalc", NULL };
static const char *volume[3][9] = {
	{ "/bin/sh", "-c", "pactl set-sink-volume @DEFAULT_SINK@ +10% && pkill -SIGRTMIN+15 dwmblocks", NULL },
	{ "/bin/sh", "-c", "pactl set-sink-volume @DEFAULT_SINK@ -10% && pkill -SIGRTMIN+15 dwmblocks", NULL },
	{ "/bin/sh", "-c", "pactl set-sink-mute @DEFAULT_SINK@ toggle && pkill -SIGRTMIN+15 dwmblocks", NULL },
};

static const char *player[3][3] = { 
  { "playerctl", "play-pause", NULL },
  { "playerctl", "next",       NULL },
  { "playerctl", "previous",   NULL }
};

static const char *brightness[2][5] = {
	{ "brightnessctl", "-q", "set", "5%-", NULL },
	{ "brightnessctl", "-q", "set", "5%+", NULL },
};
static const char *flameshot[] = { "flameshot", "gui", NULL };
static const char *browserCmd[] = { "google-chrome", NULL };

static const Key keys[] = {
	/* modifier                     key                       function        argument */
	{ MODKEY,                       XK_p,                     spawn,          {.v = dmenucmd } },
	{ MODKEY|ControlMask,           XK_p,                     spawn,          {.v = dmonitor } },
	{ MODKEY,                       XK_Return,                spawn,          {.v = termcmd } },
	{ MODKEY,                       XK_b,                     togglebar,      {0} },
	{ MODKEY,                       XK_j,                     focusstack,     {.i = +1 } },
	{ MODKEY,                       XK_k,                     focusstack,     {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_j,                     movestack,      {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_k,                     movestack,      {.i = -1 } },
	{ MODKEY,                       XK_i,                     incnmaster,     {.i = +1 } },
	{ MODKEY,                       XK_d,                     incnmaster,     {.i = -1 } },
	{ MODKEY,                       XK_h,                     setmfact,       {.f = -0.05} },
	{ MODKEY,                       XK_l,                     setmfact,       {.f = +0.05} },
	{ MODKEY,                       XK_z,                     zoom,           {0} },
	{ MODKEY|ShiftMask,             XK_c,                     killclient,     {0} },
	{ MODKEY,                       XK_t,                     setlayout,      {.v = &layouts[0]} },
	{ MODKEY,                       XK_f,                     setlayout,      {.v = &layouts[1]} },
	{ MODKEY,                       XK_m,                     setlayout,      {.v = &layouts[2]} },
	{ MODKEY,                       XK_Tab,                   setlayout,      {0} },
	{ MODKEY|ShiftMask,             XK_Tab,                   togglefloating, {0} },
	{ MODKEY,                       XK_0,                     view,           {.ui = ~0 } },
	{ MODKEY|ShiftMask,             XK_0,                     tag,            {.ui = ~0 } },
	{ MODKEY,                       XK_comma,                 focusmon,       {.i = -1 } },
	{ MODKEY,                       XK_period,                focusmon,       {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_comma,                 tagmon,         {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_period,                tagmon,         {.i = +1 } },
	{ MODKEY,                       XK_F1,                    togglescratch,  { .ui = 0 } },
	{ MODKEY,                       XK_F2,                    togglescratch,  { .ui = 1 } },
	{ MODKEY,                       XK_minus,                 setgaps,        {.i = -5 } },
	{ MODKEY,                       XK_equal,                 setgaps,        {.i = +5 } },
	{ MODKEY|ShiftMask,             XK_minus,                 setgaps,        {.i = GAP_RESET } },
	{ MODKEY|ShiftMask,             XK_equal,                 setgaps,        {.i = GAP_TOGGLE} },
	{ MODKEY,                       XK_space,                 spawn,          SHCMD("xkb-switch -n && pkill -SIGRTMIN+14 dwmblocks") },
	TAGKEYS(                        XK_1,                                     0)
	TAGKEYS(                        XK_2,                                     1)
	TAGKEYS(                        XK_3,                                     2)
	TAGKEYS(                        XK_4,                                     3)
	TAGKEYS(                        XK_5,                                     4)
	TAGKEYS(                        XK_6,                                     5)
	TAGKEYS(                        XK_7,                                     6)
	TAGKEYS(                        XK_8,                                     7)
	TAGKEYS(                        XK_9,                                     8)
	{ MODKEY|ShiftMask,             XK_r,                     quit,           {1} }, 
	{ MODKEY|ControlMask,           XK_q,                     spawn,          {.v = dPowermenu} }, 
	{ MODKEY|ShiftMask,             XK_b,                     spawn,          {.v = browserCmd} },
  { 0,                            XF86XK_AudioRaiseVolume,  spawn,          { .v = volume[0] } },
	{ 0,                            XF86XK_AudioLowerVolume,  spawn,          { .v = volume[1] } },
	{ 0,                            XF86XK_AudioMute,         spawn,          { .v = volume[2] } },
	{ 0,                            XF86XK_AudioPlay,         spawn,          { .v = player[0] } },
	{ 0,                            XF86XK_AudioNext,         spawn,          { .v = player[1] } },
	{ 0,                            XF86XK_AudioPrev,         spawn,          { .v = player[2] } },
	{ 0,                            XF86XK_Calculator,        spawn,          { .v = kcalc } },
	{ 0,                            XF86XK_MonBrightnessDown, spawn,          { .v = brightness[0] } },
	{ 0,                            XF86XK_MonBrightnessUp,   spawn,          { .v = brightness[1] } },
	{ 0,                            XK_Print,                 spawn,          { .v = flameshot } },
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static const Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
	{ ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	{ ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd } },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};

