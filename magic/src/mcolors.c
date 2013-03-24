/*
  mcolors.c
	
  Calligraphy Magic Tool Plugin
  Tux Paint - A simple drawing program for children.

  Copyright (c) 2002-2008 by Bill Kendrick and others; see AUTHORS.txt
  bill@newbreedsoftware.com
  http://www.tuxpaint.org/

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
  (See COPYING.txt)
*/

#include <stdio.h>
#include <string.h>  
#include <libintl.h> 

#include "tp_magic_api.h"  
#include "SDL_image.h" 
#include "SDL_mixer.h"  


enum {
  TOOL_ONE,  
  NUM_TOOLS  
};

const char * snd_filenames[NUM_TOOLS] = {
  "one.wav",
};

const char * icon_filenames[NUM_TOOLS] = {
  "mcolors.png",
};

const char * names[NUM_TOOLS] = {
  gettext_noop("Magic Colors"),
};

const char * descs[NUM_TOOLS] = {
  gettext_noop("Draw your picture and touch it with a bit of magic"),
};


Mix_Chunk * snd_effect[NUM_TOOLS];

Uint8 mcolors_r, mcolors_g, mcolors_b;

void mcolors_drag(magic_api * api, int which, SDL_Surface * canvas,
	          SDL_Surface * snapshot, int ox, int oy, int x, int y,
		  SDL_Rect * update_rect);

void mcolors_line_callback(void * ptr, int which,
                           SDL_Surface * canvas, SDL_Surface * snapshot,
                           int x, int y);

Uint32 mcolors_api_version(void)
{
  return(TP_MAGIC_API_VERSION);
}

int mcolors_init(magic_api * api)
{
  int i;
  char fname[1024];

  for (i = 0; i < NUM_TOOLS; i++)
  {
    snprintf(fname, sizeof(fname),
             "%s/sounds/magic/%s",
	     api->data_directory, snd_filenames[i]);

    printf("Trying to load %s sound file\n", fname);


    snd_effect[i] = Mix_LoadWAV(fname);
  }

  return(1);
}

int mcolors_get_tool_count(magic_api * api)
{
  return(NUM_TOOLS);
}

SDL_Surface * mcolors_get_icon(magic_api * api, int which)
{
  char fname[1024];

  snprintf(fname, sizeof(fname), "%s/images/magic/%s.png",
	     api->data_directory, icon_filenames[which]);



  return(IMG_Load(fname));
}

char * mcolors_get_name(magic_api * api, int which)
{
  const char * our_name_english;
  const char * our_name_localized;

  our_name_english = names[which];

  our_name_localized = gettext(our_name_english);

  return(strdup(our_name_localized));
}

char * mcolors_get_description(magic_api * api, int which, int mode)
{
  const char * our_desc_english;
  const char * our_desc_localized;

  our_desc_english = descs[which];

  our_desc_localized = gettext(our_desc_english);

  return(strdup(our_desc_localized));
}

int mcolors_requires_colors(magic_api * api, int which)
{
  return 1;
}

int mcolors_modes(magic_api * api, int which)
{
  return MODE_FULLSCREEN;
}

void mcolors_shutdown(magic_api * api)
{
  int i;
  for (i = 0; i < NUM_TOOLS; i++)
    Mix_FreeChunk(snd_effect[i]);
}


void do_mcolors(void * ptr, SDL_Surface * canvas, SDL_Surface * snapshot,int x, int y)
{
  magic_api * api = (magic_api *) ptr;
  Uint8 r = 0,g = 0,b = 0;
  float h = 0, s = 0, v = 0;
  float h_col = 0, s_col = 0,v_col = 0;
  Uint32 pixel = 0;
  
  api->rgbtohsv(mcolors_r,mcolors_g,mcolors_b,&h,&s,&v);
  h_col = h;
  s_col = s;
  for(int y=0;y<canvas->h;y++)
		for(int x=0;x<canvas->w;x++)
		{		
			pixel = api->getpixel(canvas,x,y);
			SDL_GetRGB(pixel,canvas->format,&r,&g,&b); // for v parameter
			api->rgbtohsv(r,g,b,&h,&s,&v);
			if (r==255 && g==255 && b==255) continue;
			h = h_col;
			s = s_col;
			v_col = v;
			if (mcolors_r==0 && mcolors_g==0 &&mcolors_b==0){
				v += 50;
			 }
			else if (mcolors_r==128 && mcolors_g==128 && mcolors_b==128){
				 v += 15;
			 }
			else if (mcolors_r==192 && mcolors_g==192 && mcolors_b==192){
				 v -= 15;
			 }
			else if (mcolors_r==255 && mcolors_g==255 && mcolors_b==255){
				 v -= 50;
			 }
			else{
				h = h_col;
			}
			api->hsvtorgb(h,s,v,&r,&g,&b);
			pixel = SDL_MapRGB(canvas->format,r,g,b);
			api->putpixel(canvas,x,y,pixel);
		}
 }

void mcolors_click(magic_api * api, int which, int mode,
	           SDL_Surface * canvas, SDL_Surface * snapshot,
	           int x, int y, SDL_Rect * update_rect)
{
  mcolors_drag(api, which, canvas, snapshot, x, y, x, y, update_rect);
}

void mcolors_drag(magic_api * api, int which, SDL_Surface * canvas,
	          SDL_Surface * snapshot, int ox, int oy, int x, int y,
		  SDL_Rect * update_rect)
{
 do_mcolors(api,canvas,snapshot,x,y);
 update_rect->x = 0;
 update_rect->y = 0;
 update_rect->w = canvas->w;
 update_rect->h = canvas->h;

 api->playsound(snd_effect[which],
                 (x * 255) / canvas->w, 
                 	         255);
}

void mcolors_release(magic_api * api, int which,
	           SDL_Surface * canvas, SDL_Surface * snapshot,
	           int x, int y, SDL_Rect * update_rect)
{
}

void mcolors_set_color(magic_api * api, Uint8 r, Uint8 g, Uint8 b)
{
  mcolors_r = r;
  mcolors_g = g;
  mcolors_b = b;
}

void mcolors_switchin(magic_api * api, int which, int mode, SDL_Surface * canvas)
{
}

void mcolors_switchout(magic_api * api, int which, int mode, SDL_Surface * canvas)
{
}
