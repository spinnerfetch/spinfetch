#ifndef FIELDS_H_
#define FIELDS_H_

typedef struct {
    const char *command;
    const char *name; 
} PackageManager;

char *safeRun(const char *cmd);
char *safeRead(const char *path);
char *sendUnknown(void);

char *getOs(void);
char *getHost(void);
char *getKernel(void);

char *formatUptime(unsigned long long totalSeconds);
char *getUptime(void);

char *getPackages(void);

char *getShell(void);

char *getResolution(void);

char *getDesktopEnvironment(void);
char *getWindowManager(void);

char *getTheme(void);
char *getWindowManagerTheme(void);
char *getIcons(void);
char *getFont(void);

char *getTerminal(void);

char *getCPU(void);
char *getGPU(void);

char *getMemoryUsage(void);

#endif