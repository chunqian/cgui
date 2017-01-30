The examples may preferably be evalutated in the order indicated by the
number that prefixes thier names as listed below (the first ones explains a
more "simple" usage, the last ones a more "advanced" usage).

00hello.c
   A program that creates a window, with a few buttons in. I just show how
   to use cgui.

01hellox.c
   Similar to the previous program. The purpose is to show an more
   conveniant manner of processing events. This approach will be used in all
   other examples.

02objtyp.c
   A program that performs absolutely nothing, but exposes the most common
   types of objects in cgui.

03spinbu.c
   The code is identical to "objtypes.c" except that a spin-button has been
   added.

04status.c
   The code is identical to "objtypes.c" except that a status-field has been
   added.

05txtbox.c
   The code is similar to "objtypes.c". Some "text-objects" are added.

06edit.c
   Shows some details about advanced use of edit-boxes. The code is a
   modification of the code in "status.c".

07slider.c
   Exposes some views of the slider object.

08wintyp.c
   The program show the effect of the different window-modes in cgui.

09menus.c
   Some meaningless code, but is shows how to create menus, and how they look.

10lists.c
   Shows simple usage of lists

11contai.c
   Shows the usage of containers for grouping of objects

12tabwin.c
   Shows a simple usage of tab-windows.

13direct.c
   Shows some advanced topics about grouping, sizing, and positioning objects
   by use of the `direction commands' and containers in conjunction.

14resize.c
   Must be linked with: ldnames.c, skv.c
   This code shows how to apply the re-sizing facility to objects, both
   simple objects and nodes like lists.
   The program views the content from an address book (skv-format).

   15filebr.c
   Shows a simple usage of a file selector and a file manager. To use a
   file browser from CGUI in your application, you must link with libaldat.a
   which you will find in allegro/lib/'platform' where 'platform' is the
   platform you use (djgpp, mingw).
   The calls are set up to allow for browsing in data files. You can open
   this file and read its contents in the text-box or you can as well find a
   text-file int e.g. language.dat.

16filere.c
   Shows extended usage of the file browsers: How to use "file registration",
   which is a function to extend the default behaviour of the browsers (for
   icons, names, pre-viewing, call-backs etc).

17progr.c
   Shows how to use the progress bar object/window.

18icons.c
   Shows how to decorate variaous type of objects with icons in CGUI.

19deskt.c
   Shows how to use the bare desktop to put icons on.

20anim.c
   Shows how to do animations on objects, examplified with a push-button.

21canvas.c
   Shows an example of a possible use of the canvas object (the games show
   another suggestion).

22drag.c
   Shows how to use the drag-and-drop facility in CGUI. List-boxes and
   edit-boxes are involved in the example.

23ttip.c
   Shows how to use tool tips and the view modification facility of CGUI.

24browse.c
   Shows how to use the browsing object in CGUI. The browsing object can
   be attached to any object, but has probably only meaning if you attach
   it to a canvas object.

25viewer.c
   A simple text viewer made by Rafal. Pass a file name on the command line or
   a file browser will give you the opportunity to select a file.

26joytst.c
   A joy stick testing program. Ported from Allegro's example by Rafal.

27trees.c
   This program shows how to make a tree viewer out of a normal list-box.
   
28colors.c
   This program shows how to create custom colors in CGUI.

------------------------
The game*.c examples: Not a real game, just a bouncing ball, but anyway it
shows something that is moving. These examples shows how cgui can be used
together with any game.

game1.c
   An ordinary game (not built on top of CGUI) which only uses a "requester"
   from the CGUI-lib.

game2.c
   The other way around: The same game is now built on top of CGUI. The
   board is put directly on the "desktop" of CGUI.

game3.c
   Now using a window (instead of the desktop) for the game.

game4.c
   Now running an arbitrary number of games, each in its own window.

game5.c
   As "game4.c" but now using the built in dialogue for selecting screen mode.

game6.c
   As "game5.c" but now allowing the user to resize the canvas (the game
   "board").
