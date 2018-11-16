#include <ncurses.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../driver/hawk.h"


#define HAWK_PROCFS_ENTRY "/proc/hawk"
#define GUI_AIM_UP			'w'
#define GUI_AIM_DOWN		's'
#define GUI_ROTATE_RIGHT	'd'
#define GUI_ROTATE_LEFT		'a'
#define GUI_FIRE_MISSILES	'f'
#define GUI_STOP_MOVEMENT	'q'
#define GUI_QUIT_PROGRAM	'x'



int write_to_procfile(char ch, FILE *p)
{
	int retval;
	
	fputc(ch,p);
	retval = fflush(p);
	
	if(retval < 0) {
		printw("Wrtiting to %s failed!\n", HAWK_PROCFS_ENTRY);
		refresh();
		getch();
	}
	
	return retval;
}


int main(void)
{
	int k;
	FILE *f = NULL;

	initscr();
	raw();
	noecho();
	keypad(stdscr,TRUE);
	
	
	//does /proc/hawk exist?
	struct stat pe_stat;
	if (stat (HAWK_PROCFS_ENTRY, &pe_stat) == 0) {
			f = fopen(HAWK_PROCFS_ENTRY,"a");
			
			if (f) {
					printw("[%c] rotate left\n", GUI_ROTATE_LEFT);
					printw("[%c] rotate right\n", GUI_ROTATE_RIGHT);
					printw("[%c] aim up\n", GUI_AIM_UP);
					printw("[%c] aim down\n", GUI_AIM_DOWN);
					printw("[%c] fire missiles\n", GUI_FIRE_MISSILES);
					printw("[%c] stop movement\n", GUI_STOP_MOVEMENT);
					printw("[%c] quit program\n", GUI_QUIT_PROGRAM);
					printw("\n");
					
					k = 0;
					while(k == 0)
					{
						switch(getch())
						{
							case GUI_ROTATE_LEFT:	k = write_to_procfile(CONST_ROTATE_LEFT,f); break;
							case GUI_ROTATE_RIGHT:	k = write_to_procfile(CONST_ROTATE_RIGHT,f); break;
							case GUI_AIM_UP:		k = write_to_procfile(CONST_AIM_UP,f); break;
							case GUI_AIM_DOWN:		k = write_to_procfile(CONST_AIM_DOWN,f); break;
							case GUI_FIRE_MISSILES:	k = write_to_procfile(CONST_FIRE_MISSILES,f); break;
							case GUI_STOP_MOVEMENT:	k = write_to_procfile(CONST_STOP_MOVEMENT,f); break;
							case GUI_QUIT_PROGRAM:	k = 1; break;
						}
					}
					
					fclose(f);
			} else {
				printw("fopen() failed!\n");
				refresh();
				getch();
			}
			
	} else {
		printw("/proc/hawk does not exist!");
		refresh();
		getch();
	}

	endwin();

	return 0;
}