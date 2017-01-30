/* registration code for setting up the FLI object plugin */
{
   extern DATEDIT_OBJECT_INFO datfli_info;
   extern DATEDIT_GRABBER_INFO datfli_grabber;

   datedit_register_object(&datfli_info);
   datedit_register_grabber(&datfli_grabber);
}

/* registration code for setting up the font object plugin */
{
   extern DATEDIT_OBJECT_INFO datfont_info;
   extern DATEDIT_GRABBER_INFO datfont_grabber;

   datedit_register_object(&datfont_info);
   datedit_register_grabber(&datfont_grabber);
}

/* registration code for setting up the grab-from-grid plugin */
{
   extern void datgrid_init(void);

   datgrid_init();
}

/* registration code for setting up the bitmapped image plugin */
{
   extern DATEDIT_OBJECT_INFO datbitmap_info;
   extern DATEDIT_GRABBER_INFO datbitmap_grabber;

   extern DATEDIT_OBJECT_INFO datrlesprite_info;
   extern DATEDIT_GRABBER_INFO datrlesprite_grabber;

   extern DATEDIT_OBJECT_INFO datcsprite_info;
   extern DATEDIT_GRABBER_INFO datcsprite_grabber;

   extern DATEDIT_OBJECT_INFO datxcsprite_info;
   extern DATEDIT_GRABBER_INFO datxcsprite_grabber;

   datedit_register_object(&datbitmap_info);
   datedit_register_grabber(&datbitmap_grabber);

   datedit_register_object(&datrlesprite_info);
   datedit_register_grabber(&datrlesprite_grabber);

   datedit_register_object(&datcsprite_info);
   datedit_register_grabber(&datcsprite_grabber);

   datedit_register_object(&datxcsprite_info);
   datedit_register_grabber(&datxcsprite_grabber);
}

/* registration code for setting up the MIDI object plugin */
{
   extern DATEDIT_OBJECT_INFO datmidi_info;
   extern DATEDIT_GRABBER_INFO datmidi_grabber;

   datedit_register_object(&datmidi_info);
   datedit_register_grabber(&datmidi_grabber);
}

/* registration code for setting up the palette object plugin */
{
   extern DATEDIT_OBJECT_INFO datpal_info;
   extern DATEDIT_GRABBER_INFO datpal_grabber;

   datedit_register_object(&datpal_info);
   datedit_register_grabber(&datpal_grabber);
}

/* registration code for setting up the sample object plugin */
{
   extern DATEDIT_OBJECT_INFO datsample_info;
   extern DATEDIT_GRABBER_INFO datsample_grabber;

   extern DATEDIT_OBJECT_INFO datpatch_info;
   extern DATEDIT_GRABBER_INFO datpatch_grabber;

   datedit_register_object(&datsample_info);
   datedit_register_grabber(&datsample_grabber);

   datedit_register_object(&datpatch_info);
   datedit_register_grabber(&datpatch_grabber);
}
