#ifndef FILEBUFF_H
#define FILEBUFF_H

typedef struct t_copy_buffer t_copy_buffer;
typedef struct t_copy_buffer_window t_copy_buffer_window;
struct t_dirinfo;
struct t_diskobj;

/* Returns a pointer to the global copy buffer. The memory pointed to is owned by the copy-buffer system itself, not the caller. */
extern t_copy_buffer *CopyBufferGetGlobal(void);

/* Returns a pointer to a new created copy buffer. The memory pointed to is owned by the caller. */
extern t_copy_buffer *CopyBufferCreate(void);

/* Destroys all memory assosiated with the copy buffer `copy_buffer'. */
extern void CopyBufferDestroy(t_copy_buffer *copy_buffer);

/* Removes all file entries in the copy buffer. */
extern void CopyBufferClean(t_copy_buffer *copy_buffer);

/* Removes all duplicates in the copy buffer. */
extern void CopyBufferFilterDuplicates(t_copy_buffer *copy_buffer);

/* Inserts ont file entry intor the copy  buffer. */
extern void CopyBufferInsertEntry(t_copy_buffer *copy_buffer, struct t_diskobj *entry, int cut);


/* Returns true if the copy buffer is empty. */
extern int CopyBufferIsEmpty(t_copy_buffer *copy_buffer);

/* Returns true if a file entry `path' is in the copy buffer. */
extern int CopyBufferEntryExists(t_copy_buffer *copy_buffer, const char *path);

/* Pastes the entire copy buffer into the specified destination. */
extern void CopyBufferPaste(t_copy_buffer *copy_buffer, struct t_dirinfo *dest_dirinfo);

/* Opens a window that simply shows the current contents of the global copy-buffer. Returns a pointer to a copy buffer window object.
   The window may be closed either by a user control-button in the window or by a call to `CopyBufferCloseWindow'.
   `isopen' must be a pointer to a persistent location of the caller. The value will reflect the open state of the window, and can
   thus be used by the caller to detect the actions of the user. */
extern t_copy_buffer_window *CopyBufferOpenWindow(int *isopen, int *addmode);

/* Closes an open copy buffer window. After the call `copy_buffer_window' is no longer a valid pointer. */
extern void CopyBufferCloseWindow(t_copy_buffer_window *copy_buffer_window);

#define UPD_BUFFER_FLAG 0x80000000

#endif
