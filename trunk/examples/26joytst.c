/* Example to show how to emulate the mouse using a joystick or keyboard. */

#include <allegro.h>
#include <string.h>
#include "cgui.h"

int use_kbd = 0;

static struct mouse_emu {
   /* Area limit. */
   int x1, y1, x2, y2;

   /* Current pos. */
   int x, y, z;

} mouse_emu;

static void check_range(void)
{
   if (mouse_emu.x > mouse_emu.x2)
      mouse_emu.x = mouse_emu.x2;
   if (mouse_emu.x < mouse_emu.x1)
      mouse_emu.x = mouse_emu.x1;
   if (mouse_emu.y < mouse_emu.y1)
      mouse_emu.y = mouse_emu.y1;
   if (mouse_emu.y > mouse_emu.y2)
      mouse_emu.y = mouse_emu.y2;
}

static void process_joystick_input(int *px, int *py, int *pz, int *buttons)
{
   poll_joystick();

   mouse_emu.x += joy[0].stick[0].axis[0].pos/40;
   mouse_emu.y += joy[0].stick[0].axis[1].pos/40;
   mouse_emu.z += joy[0].stick[0].axis[2].pos/40;
   check_range();
   *px = mouse_emu.x;
   *py = mouse_emu.y;
   *pz = mouse_emu.z;
   *buttons = joy[0].button[0].b | (joy[0].button[1].b << 1) | (joy[0].button[2].b << 2);
}

static void process_kbd_input(int *px, int *py, int *pz, int *buttons)
{
   if (key[KEY_A])
      mouse_emu.x -= 2;
   if (key[KEY_W])
      mouse_emu.y -= 2;
   if (key[KEY_D])
      mouse_emu.x += 2;
   if (key[KEY_S])
      mouse_emu.y += 2;
   check_range();
   *px = mouse_emu.x;
   *py = mouse_emu.y;
   *pz = 0;
   *buttons = key[KEY_F1] | (key[KEY_F2] << 1) | (key[KEY_F3] << 2);
}

static void force_pos(int x, int y)
{
   mouse_emu.x = x;
   mouse_emu.y = y;
   check_range();
}

static void set_range(int x, int y, int w, int h)
{
   mouse_emu.x1 = x;
   mouse_emu.y1 = y;
   mouse_emu.x2 = x + w - 1;
   mouse_emu.y2 = y + h - 1;
}

static int init_joystick(void)
{
   const char *msg;

   Req("Message","Please center the joystick and press enter. If you have digital joystick/pad just press enter|OK");

   if (install_joystick(JOY_TYPE_AUTODETECT) != 0) {
      Req("Error","Error initialising joystick module!| OK ");
      return 0;
   }

   if (!num_joysticks) {
      if (Req("Error","joystick not found! alternative: press 'use keyboard' and use A W S D keys to move cursor.|Exit|Use keyboard")==0) {
         return 0;
      } else {
         use_kbd = 1;
      }
   }

   if (use_kbd) {
      CguiSetMouseInput(process_kbd_input, force_pos, set_range);
   } else {
      while (joy[0].flags & JOYFLAG_CALIBRATE) {
         msg = calibrate_joystick_name(0);
         Request("PRESS ENTER", 400, 0, "%s|Press_enter key", msg);
         if (calibrate_joystick(0) != 0) {
            Req("Error","Error calibrating joystick!| OK ");
            return 0;
         }
      }
      CguiSetMouseInput(process_joystick_input, force_pos, set_range);
   }
   mouse_emu.x = SCREEN_W / 2;
   mouse_emu.y = SCREEN_H / 2;
   mouse_emu.x1 = 0;
   mouse_emu.y1 = 0;
   mouse_emu.x2 = SCREEN_W - 1;
   mouse_emu.y2 = SCREEN_H - 1;
   return 1;
}

void quit(void *data)
{
   (void)data;
   StopProcessEvents();
}

int main(void)
{
   int radiosel = 1;
   InitCgui(640,480,16);

   if (init_joystick()) {
      MkDialogue(ADAPTIVE,"JoyTest",0);
      AddTextBox(DOWNLEFT, "The mouse can no longer be used to move the cursor", 200, 0, 0);
      MkRadioContainer(DOWNLEFT, &radiosel, R_HORIZONTAL);
      AddRadioButton("radiosel=0");
      AddRadioButton("radiosel=1");
      AddRadioButton("radiosel=2");
      EndRadioContainer();
      if (use_kbd) {
         AddTextBox(DOWNLEFT, "No joystick detected. Use the keyboard to move the cursor._"
               " A - left_ D - right_ W - up_ S - down_ F1 - left mouse button",
               200, 0, TB_LINEFEED_);
      } else {
         AddTextBox(DOWNLEFT, "Joystick detected. Use the stick to move the cursor and the left button as mouse button.", 200, 0, 0);
      }
      AddButton(DOWNLEFT,"E~xit", quit, NULL);
      DisplayWin();
      ProcessEvents();
   }
   return 0;
}
END_OF_MAIN()
