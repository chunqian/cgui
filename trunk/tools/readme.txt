  `mktext' is a program with the purpose to give a tool so that you can
   keep a program free from language dependent textstrings.

   Very brief about simple usage: A source-file (`.t') containing rows, each
   of which consist of pairs which defines the text. Each pair consist of:
   - a C-identifier
   - a quoted string
   Each source file must first of all start with a language definition,
   followed by a section definition. Then the actual text specifications
   follows.
   Example (suppose the below is contained in a file `foo.t'):
<language=en> /* specifies the name of the language to `en' */
[foo]         /* specifies the name of the section to `foo' */
/* three text-definitions: */
LABEL_NAME       "Name:"
LABEL_POINTS     "Points:"
LABEL_SPEED      "Speed:"
LABEL_BEGINNER   "Beginner"

   The file will be read and a C header-file (`foo.ht') will be generated.
   Also a data-file (plain text) containing the contents of the strings will
   be generated. The name of that file will be what you typed on the command
   line.

   Your C-file, say foo.c, must load a certain section from that data-file
   (in this case it will have only one section, "foo"). The function that
   loads the text is `LoadTexts' se CGUI-docs for details. It will detetect
   the current language and if this is english, the specified strings will
   be loaded.
   Example code for loading:
   const char *const*my_texts;
   int size;
   . . .
   my_texts = LoadTexts("foo.txt", "foo", &size);

   To know where in `my_texts' a cerain string is you must first
   include foo.ht, then use the macro to index the string-array like:

#include "foo.ht"
. . .
AddCheck(TOPLEFT, my_texts[LABEL_BEGINNER], &beginner);


   You don't need to know what the generated files looks like, but in case
   you are interested; for the example above the current implementation
   generates something like:

   foo.ht:
#ifndef FOO_HT
#define FOO_HT
#define LABEL_NAME 0
#define LABEL_POINTS 1
#define LABEL_SPEED 2
#define LABEL_BEGINNER 3
#endif

   foo.txt:
<en>
[foo
"Name:
"Points:
"Speed:
"Beginner

   If the the file `allegro.cfg' is not present or if `allegro.cfg' don't
   contain any language specification, then `LoadTexts' will try to load
   section "en".
   If the the file `allegro.cfg' is present, the item `lang = xx' in the
   section `cgui' will be selected. If not present the item `language = xx'
   in the `global section' will be selected by `LoadTexts'.

   If the language requested is not present in the data-file,
   then `LoadTexts' will try to load text from the language part `en'.
   If `LoadTexts' fails to load the language `en' and there exists some
   other language(s) in the textfile, the first one will be loaded.

   About the "specification language" that mktext understands:
   - The source-file may contain C- and C++ style comments.
   - The string may be partitioned over multiple rows (using the same
   notation as in C):
NAME "The name "
     "of a "
     "person:"
   - The string may contain quotation marks (") if
     indicated by back-slash (like in C-strings).
   - The following are reserved words: start,end,language
   - mktext will check that any pair of corresponding strings from different
     languages contains identical conversions codes. If you want the '%' to be
     written and the string shall later be passed to some *printf function
     then you must type '%' twice in the string (according to the C spec.).
     This can be done also in strings processed by `mktext'. The problem is if
     you plan to pass the string as it is to some other function like `textout'
     and you want a % to be written. `mktext' will believe than the '%' is the
     beginning of a conversion code and will complain about it. To avoid this
     you must type the code "%&" (the '&'-character will not be transfered to
     the output file).
   - You may create "string arrays". The syntax for that is a start
     indiciation, a sequence of sub-elements and finally an end indication.
     Example:

LEVEL                     "Level:"
<start=LEVEL_ALT>
LEVEL_BEGINNER            "Beginner"
LEVEL_MEDIUM              "Medium"
LEVEL_ADVANCED            "Advanced"
<end=NR_LEVEL_ALT>

   This is specially useful if you use the drop-down-box function
   `AddDropDownS' in Cgui.
   Example:

const char *const*txt;
int size;
txt = LoadTexts("foo.txt", "foo", &size);
. . .
AddDropDownS(TOPLEFT, 0, txt[LEVEL], &level, txt + LEVEL_ALT, NR_LEVEL_ALT);

   Possibly you may somewhere in your code have a switch statement that refers
   to the user selection (i.e. `level') and the code may look like:

switch (level) {
case LEVEL_BEGINNER:
   . . .
   break;
case LEVEL_MEDIUM:
   . . .
   break;
case LEVEL_ADVANCED:
   . . .
   break;
}

   The if `level' is the variable which value was modified by the drop-down
   box or a radio group, its value will be in the range 0..2, which is also
   the numbers assigned to "elements" in the string array (always starting
   at 0).

   If there is no need for the names of the "elements" in the string-array,
   you can optioally use a single underscore (_) to indicate "no macro
   generation".

   A source file may contain several languages and/or sections. However, it
   is assumed that you rather preferer to have one section and one language
   per file. Typically you may want N source-files for each C-file that uses
   some texts (where N is the number of languages supported).
   Probably you want to have only one file containing the texts, in
   your final program distribution.
   Therfore `mktext' has the capability to merge all texts into one single
   destination file.

   Simple calling of `mktext' (the textfile is put into current directory and
   the headerfile(s) will be put in the same directory as the source(s)):

   mktext destfile source [source2 ...]

   Maintaining a large program in a structured manner, also includes proper
   handling of the various texts. You will probably want to put all texts
   used in a certain C-file into a specific text-source-file, like `foo.c'
   has its texts in `foo.t' and includes `foo.ht'. The compiled texts you
   will have in `myprog.txt'.

   In a large program that uses texts extensively, you are also interested in
   avoiding to re-compile all (or most of) the C-files only because you made
   changes for one text (resulting in a new `.ht' file).

   To fulfill all this, and to make it possible to set up dependencies in
   a proper way in makefiles, the generation can be made in two separate
   passes, the first generating the header-file (optionally excluded) and
   an intermediate text-file. The second pass generates the final
   text-file (similar to the link-pass of a program build).

   During the second pass the macro-identifiers from the different languages
   will be checked for equality (using the `en'-langugae as reference). Also
   existing conversions specifications (like %d, %s etc) will be
   checked as well as the `Requester' codes ('|').

   Calling the mktext program for "compiling":

   mktext -c [-pXXXX] [-iYYYY] [-nh] source [source2 ...]

   where
   -XXXX is the path of the header file(s) (by default it will be put into
   the same directory as the source-file(s)).
   -YYYY is the path of the intermediate file(s) (by default it will be put
   into the same directory as the source-file(s)).
   -nh means "create no header file". Any two headerfiles originating from
   different languages will of course be identical (provided that the sources
   are correct) so you only need to have them generated for one of the
   languages.
   The suffix of the intermediate files is `.it'

   calling the mktext program for "linking":

   mktext -l destfile intermediate-file [intermediate-file2 ...]

   Advanced topics:

   * About labels and "hot-keys":
   If you have assigned hotkeys manually in a couple of strings, and want them
   to be checked for uniqueness: Just put them within a block (a pair of {}).
   Blocks may also be nested in the source. This is necessary if you use
   tab-windows (because you want the hotkeys outside the tab-part to be
   unique when combined with any selection of tab). Tab-windows may be nested.
   In a future there may also be automatic generation of hot-keys.
   Hot-keys within a Requester-text will be checked against each other
   (each button-label within the text must have a unique label). This check
   is always done. Requester-text shall be outside any block, because you
   don't want them to be checked against each other.

   * About file handling:
   If you need a text that contains more than one conversion
   specification, this may be a problem when translating to different
   languages. When creating an expressive text in a language different from
   yours, it will be a limitation to keep the conversion specifications in
   a fix order - remember that other languages may use a different "word
   order" (just look at the Germans :-). mktext provides a special "floating
   position" conversion code which has the syntax: %<any text>. When such a
   string is used in the program you must first call the
   RegisterConversionHandler, and then use PrintFloatingConversion (which
   similar to sprintf formats the input string into a destination string).

   Normal C-conversions (like %d, %s etc) can not be combined in such a
   string, they will be ignored (the point whith this "floating position"
   conversion is to get rid of all such codes).

   The semantics of this is a textual reference to a function, and thus is
   safe. If you mix things up and incidently pass the string to some standard
   function (like sprintf) the conversions codes will be ignored. If you
   forget to register the formatting functions (or miss-spel the text) the
   name of the conversion will be printed instead.

   See the cgui-help for details about RegisterConversionHandler
   and PrintFloatingConversion.

   Example:

   foo.t
<language=en>
[person_dialogue]
PERSON_DIALOGUE  "Dialogue for entering persons"
NAME             "Name:"
AREA             "Area:"
CANCEL           "Cancel"
OK               "OK"
REQ_WARNING_PERSON_EXISTS "The entered person %<name> has already been "
                          "entered (from %<area>).|Canel|OK"
<language=se>
[person_dialogue]
PERSON_DIALOGUE  "Dialog för att skriva in personer"
NAME             "Namn:"
AREA             "Distrikt:"
CANCEL           "Avbryt"
OK               "OK"
REQ_WARNING_PERSON_EXISTS "I distriktet %<area> finns det redan en person "
                          "med namnet %<name>.|Avbryt|OK"

   And in foo.c:
   . . .
   RegisterConversionHandler("<name>", format_name, new_person);
   RegisterConversionHandler("<area>", format_area, clatch_person);
   PrintFloatingConversion(s, text[REQ_WARNING_PERSON_EXISTS]);
   if (Req(s))
      /* enter person */
   else
      /* return to dialogue */

   . . .
void format_name(char *s, void *data)
{
   struct t_person *p = data;
   sprintf(s, "%s (%d)", p->name, p->number);
}

void format_area(char *s, void *data)
{
   struct t_person *p = data;
   sprintf(s, "%s", p->area->name);
}

   (For swedes: yes, in the above example the text would easyly have been
   re-written to keep the same order of the conversion specifications as
   in the english version - swedish and english are two very similar
   languages, but there are other...)

   The registration of conversion-handlers are permanent. In many cases
   this is not very useful since the data object may alter from time to time.
   A re-registration for each use (like in the example above) will however
   replace the previous one.
