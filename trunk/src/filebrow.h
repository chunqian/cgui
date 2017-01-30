#ifndef FILEBROW_H
#define FILEBROW_H

typedef struct t_browser t_browser;

extern void StorePasswordSettings(void);
extern void InitFileBrowser(void);
extern void ReloadFileList(t_browser *br);

#define UPD_BROWSER_FLAG  0x40000000
#define UPD_BROWSER_FLAG2 0x20000000

#endif
