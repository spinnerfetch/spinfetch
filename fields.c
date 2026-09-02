#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#ifdef _WIN32
#include <windows.h>
#include <winternl.h>

#else
#include <unistd.h>
#include <sys/utsname.h>
#include <dirent.h>
#endif

#include "fields.h"

static const PackageManager packageManagers[] = {
        {"pacman -Qq", "pacman"},
        {"dpkg-query -f '.\\n' -W", "dpkg"},
        {"rpm -qa", "rpm"},
        {"apk info", "apk"},
        {"xbps-query -l", "xbps"},
        {"eopkg list-installed", "eopkg"},
        {"emerge --list-installed", "emerge"},
        {"nix-env -q", "nix"}
};

static const char *knownWMProcesses[] = {
    "i3", "sway", "bspwm", "openbox", "awesome", "dwm", "xmonad", "kwin_x11", "kwin_wayland", "mutter", "xfwm4", "marco", "compiz"
};


char *safeRun(const char *cmd) {
    #ifdef _WIN32
        // Opens pipe for the command, if pipe is null return null
        FILE *pipe = _popen(cmd, "r");
        if (pipe == NULL) {
            return NULL;
        }

        // Allocates memory for the output
        char line[1024];
        size_t capacity = 2048;
        size_t length = 0;

        char *output = malloc(capacity);
        if(output == NULL){
            _pclose(pipe);
            return NULL;
        }
        output[0] = '\0';

        // Reads return lines and adds them to outputs
        while(fgets(line, sizeof(line), pipe) != NULL){
            size_t lineLength = strlen(line);

            while(length + lineLength + 1 > capacity){
                capacity *= 2;
            }

            char *newOutput = realloc(output, capacity);

            if(newOutput == NULL){
                free(output);
                _pclose(pipe);
                return NULL;
            }

            output = newOutput;
            memcpy(output + length, line, lineLength);
            length += lineLength;
            output[length] = '\0';
        }

        // If command failed return null
        int status = _pclose(pipe);
        if(status != 0){
            free(output);
            return NULL;
        }

        return output;
    
    #else // Same code with Linux/macOS adjustments
        FILE *pipe = popen(cmd, "r");
        if (pipe == NULL) {
            return NULL;
        }

        char line[1024];
        size_t capacity = 2048;
        size_t length = 0;

        char *output = malloc(capacity);
        if(output == NULL){
            pclose(pipe);
            return NULL;
        }
        output[0] = '\0';

        while(fgets(line, sizeof(line), pipe) != NULL){
            size_t lineLength = strlen(line);

            while(length + lineLength + 1 > capacity){
                capacity *= 2;
            }

            char *newOutput = realloc(output, capacity);

            if(newOutput == NULL){
                free(output);
                pclose(pipe);
                return NULL;
            }

            output = newOutput;
            memcpy(output + length, line, lineLength);
            length += lineLength;
            output[length] = '\0';
        }

        int status = pclose(pipe);
        if(status != 0){
            free(output);
            return NULL;
        }

        return output;
    #endif
}

char *safeRead(const char *path)
{
    // Opens file, if file is NULL return NULL
    FILE *file = fopen(path, "r");
    if(file == NULL){
        return NULL;
    }

    // Allocates enough bytes to read the file
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *contents = malloc(size + 1);

    // If couldn't allocate memory close file and return null
    if(contents == NULL){
        fclose(file);
        return NULL;
    }

    // Tries to fill contents with file and returns it, otherwise return null
    if(fread(contents, 1, size, file) == size){
        contents[size] = '\0';
        fclose(file);
        return contents;
    } else {
        free(contents);
        fclose(file);
        return NULL;
    }
}

char *sendUnknown(void) {
    char *result = malloc(8);
    if(result == NULL) return NULL;
    strcpy(result, "Unknown");
    return result;
}


/*
                GET OS, HOST AND KERNEL
*/

char *getOs(void) {
    #ifdef _WIN32
        RTL_OSVERSIONINFOEXW info = {0};
        info.dwOSVersionInfoSize = sizeof(info);

        if(RtlGetVersion((PRTL_OSVERSIONINFOEXW)&info) != 0){
            return NULL;
        }

        char *result = malloc(11);
        if(result == NULL) return NULL;

        if(info.dwBuildNumber >= 22000){
            strcpy(result, "Windows 11");
        } else {
            strcpy(result, "Windows 10");
        }

        return result;

    #elif defined(__linux__)
        char *contents = safeRead("/etc/os-release");

        if(contents != NULL){
            char *line = strtok(contents, "\n");
            
            while(line != NULL){
                if(strncmp(line, "PRETTY_NAME=", 12) == 0) {
                    char *value = line + 12;

                    if(value[0] == '"') {
                        value++;
                    }

                    size_t length = strlen(value);

                    if(length > 0 && value[length - 1] == '"') {
                        value[length - 1] = '\0';
                    }

                    char *result = malloc(strlen(value) + 1);

                    if(result == NULL) {
                        free(contents);
                        return NULL;
                    }

                    strcpy(result, value);
                    free(contents);
                    return result;
                }

                line = strtok(NULL, "\n");
            }

            free(contents);
            return sendUnknown();
        }
        return sendUnknown();

    #elif defined(__APPLE__)
        char *version = safeRun("sw_vers -productVersion");

        if(version != NULL){
            size_t length = strlen(version);
            if(length > 0 && version[length - 1] == '\n'){
                version[length -1] = '\0';
            }

            char *result = malloc(strlen(version) + 7);
            if(result == NULL){
                free(version);
                return NULL;
            }

            strcpy(result, "macOS ");
            strcat(result, version);
            free(version);
            return result;
        }

        return sendUnknown();

    #else
        return sendUnknown();

    #endif
}


char *getHost(void) {
    char hostname[512];

    #ifdef _WIN32
        DWORD size = sizeof(hostname);

        if(GetComputerNameA(hostname, &size) == 0){
            strcpy(hostname, "Unknown");
        }

    #else
        if(gethostname(hostname, sizeof(hostname)) != 0){
            strcpy(hostname, "Unknown");
        }
    #endif

    char *result = malloc(strlen(hostname) + 1);
    if(result == NULL) return NULL;
    strcpy(result, hostname);
    return result;
}


char *getKernel(void) {
    #ifdef _WIN32
        RTL_OSVERSIONINFOEXW info = {0};
        info.dwOSVersionInfoSize = sizeof(info);

        if(RtlGetVersion((PRTL_OSVERSIONINFOEXW)&info) != 0){
            return sendUnknown();
        }

        char buffer[64];
        snprintf(buffer, sizeof(buffer), "%lu.%lu.%lu", info.dwMajorVersion, info.dwMinorVersion, info.dwBuildNumber);

        char *result = malloc(strlen(buffer) + 1);
        if(result == NULL) return NULL;
        strcpy(result, buffer);
        return result;

    #else
    struct utsname info;

    if (uname(&info) != 0){
        return sendUnknown();
    }

    char *result = malloc(strlen(info.release) + 1);
    if(result == NULL) return NULL;
    strcpy(result, info.release);
    return result;

    #endif
}


/*
                UPTIME
*/

char *formatUptime(unsigned long long totalSeconds) {
    unsigned long long days = totalSeconds / 86400;
    unsigned long long remainder = totalSeconds % 86400;

    unsigned long long hours = remainder / 3600;
    remainder %= 3600;

    unsigned long long minutes = remainder / 60;
    remainder %= 60;

    int seconds = remainder;

    char buffer[64];
    if(days == 0 && hours == 0 && minutes == 0){
        snprintf(buffer, sizeof(buffer), "%ds", seconds);
    }
    else if(days == 0 && hours == 0) {
        snprintf(buffer, sizeof(buffer), "%llumin %ds", minutes, seconds);
    }
    else if(days == 0) {
        snprintf(buffer, sizeof(buffer), "%lluh %llumin %ds", hours, minutes, seconds);
    }
    else {
        snprintf(buffer, sizeof(buffer), "%llud %lluh %llumin %ds", days, hours, minutes, seconds);
    }

    char *result = malloc(strlen(buffer) + 1);
    if(result == NULL) return NULL;
    strcpy(result, buffer);
    return result;
}


char *getUptime(void) {
    #ifdef _WIN32
        ULONGLONG milliseconds = GetTickCount64();
        unsigned long long seconds = milliseconds / 1000;
        return formatUptime(seconds);

    #elif defined (__linux__)
        char *contents = safeRead("/proc/uptime");
        if(contents != NULL){
            double seconds = strtod(contents, NULL);
            free(contents);
            return formatUptime((unsigned long long)seconds);
        }

        return sendUnknown();

    #elif defined (__APPLE__)
        char *output = safeRun("sysctl -n kern.boottime");

        if(output != NULL){
            char *seconds = strstr(output, "sec = ");

            if(seconds != NULL){
                unsigned long long bootTime = strtoull(seconds + 6, NULL, 10);
                free(output);
                time_t currentTime = time(NULL);

                unsigned long long uptime = (unsigned long long)currentTime - bootTime;
                return formatUptime(uptime);
            }

            free(output);
        }

        return sendUnknown();

    #else
        return sendUnknown();
    #endif
}


/*
                PACKAGES
*/

char *getPackages(void) {
    #if defined (__linux__)
    size_t managerCount = sizeof(packageManagers)/sizeof(packageManagers[0]);

    for(size_t i = 0; i < managerCount; i++){
        char *output = safeRun(packageManagers[i].command);

        if(output != NULL){
            size_t packageCount = 0;
            char *line = strtok(output, "\n");

            while(line != NULL){
                packageCount++;
                line = strtok(NULL, "\n");
            }

            char buffer[64];

            snprintf(buffer, sizeof(buffer), "%zu (%s)", packageCount, packageManagers[i].name);

            char *result = malloc(strlen(buffer) + 1);

            if(result == NULL){
                free(output);
                return NULL;
            }

            strcpy(result, buffer);
            free(output);
            return result;
        }
    }
    return sendUnknown();
    
    #else
        return sendUnknown();

    #endif
}


/*
                SHELL
*/

char *getShell(void) {
    #ifdef _WIN32
        char *shellPath = getenv("COMSPEC");

        if(shellPath != NULL){
            char *shellName = strrchr(shellPath, '\\');

            if(shellName != NULL){
                shellName++;
            } else {
                shellName = shellPath;
            }

            char *result = malloc(strlen(shellName) + 1);
            if(result == NULL) return NULL;
            strcpy(result, shellName);
            return result;
        }

        return sendUnknown();

    #elif defined (__linux__) || defined(__APPLE__)
        char *shellPath = getenv("SHELL");
        
        if(shellPath != NULL){
            char *shellName = strrchr(shellPath, '/');

            if(shellName != NULL){
                shellName++;
            } else {
                shellName = shellPath;
            }

            char *result = malloc(strlen(shellName) + 1);
            if(result == NULL) return NULL;
            strcpy(result, shellName);
            return(result);
        }

        return sendUnknown();

    #else
        return sendUnknown();
    #endif
}

/*
            RESOLUTION
*/

char *getResolution(void) {
    #ifdef _WIN32
        int width = GetSystemMetrics(SM_CXSCREEN);
        int height = GetSystemMetrics(SM_CYSCREEN);

        if(width <= 0 || height <= 0){
            return sendUnknown();
        }

        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%dx%d", width, height);

        char *result = malloc(strlen(buffer) + 1);
        if(result == NULL) return NULL;
        strcpy(result, buffer);
        return result;

    #elif defined (__linux__)
        char *output = safeRun("xrandr --current");
        if(output != NULL){
            char *line = strtok(output, "\n");

            while(line != NULL){
                if(strchr(line, '*') != NULL){
                    char *x = strchr(line, 'x');

                    if(x != NULL){
                        char *start = x;
                        while(start > line && isdigit((unsigned char) * (start - 1))){
                            start--;
                        }

                        char *end = x + 1;
                        while(isdigit((unsigned char) * end)){
                            end++;
                        }
                        while(isspace((unsigned char) * end)){
                            end++;
                        }

                        char *refreshRate = strchr(end, '*');
                        if(refreshRate != NULL){
                            size_t resolutionLength = end - start;
                            size_t refreshRateLength = refreshRate - end;

                            char *result = malloc(resolutionLength + refreshRateLength + 8);
                            if(result != NULL){
                                snprintf(result, resolutionLength + refreshRateLength + 8, "%.*s @ %.*sHz", (int)resolutionLength, start, (int)refreshRateLength, end);
                                free(output);
                                return result;
                            }
                        }
                    }
                }
                line = strtok(NULL, "\n");
            }
            free(output);
        }
        return sendUnknown();

    #elif defined (__APPLE__)
        char *output = safeRun("system_profiler SPDisplaysDataType");

        if(output != NULL){
            char *resolution = strstr(output, "Resolution: ");
            if(resolution != NULL){
                resolution += strlen("Resolution: ");

                int width;
                int height;

                if(sscanf(resolution, "%d x %d", &width, &height) == 2){
                    char buffer[32];
                    snprintf(buffer, sizeof(buffer), "%dx%d", width, height);
                    free(output);
                    
                    char *result = malloc(strlen(buffer) + 1);
                    if(result == NULL) return NULL;
                    strcpy(result, buffer);
                    return result;
                }
            }
            free(output);
        }
        return sendUnknown();

    #else
        return sendUnknown();
    #endif
}


/*
            DESKTOP ENVIRONMENT
*/

char *getDesktopEnvironment(void) {
    #ifdef _WIN32
        return strdup("Windows Shell");

    #elif defined (__linux__)
        char *desktopEnv = getenv("XDG_CURRENT_DESKTOP");
        if(desktopEnv == NULL || desktopEnv[0] == '\0') desktopEnv = getenv("DESKTOP_SESSION");

        if(desktopEnv != NULL && desktopEnv[0] != '\0'){
            char *result = malloc(strlen(desktopEnv) + 1);
            if(result == NULL) return NULL;
            strcpy(result, desktopEnv);
            return result;
        }
        return sendUnknown();

    #elif defined (__APPLE__)
        return strdup("Aqua");

    #else
        return sendUnknown();
    #endif
}


char *getWindowManager(void) {
    #ifdef _WIN32
        return strdup("DWM");

    #elif defined (__linux__)
        DIR *proc = opendir("/proc");

        if(proc == NULL) return sendUnknown();

        struct dirent *entry;

        while((entry = readdir(proc)) != NULL){
            int isPID = 1;

            for(size_t i = 0; entry->d_name[i] != '\0'; i++){
                if(!isdigit((unsigned char)entry->d_name[i])){
                    isPID = 0;
                    break;
                }
            }

            if(!isPID) continue;

            char path[512];
            snprintf(path, sizeof(path), "/proc/%s/comm", entry->d_name);

            char *comm = safeRead(path);
            if(comm == NULL) continue;
            comm[strcspn(comm, "\n")] = '\0';

            for(size_t i = 0; i < sizeof(knownWMProcesses)/sizeof(knownWMProcesses[0]); i++){
                if(strcmp(comm, knownWMProcesses[i]) == 0){
                    char *result = malloc(strlen(knownWMProcesses[i]) + 1);
                    if(result == NULL){
                        free(comm);
                        closedir(proc);
                        return NULL;
                    }

                    strcpy(result, comm);
                    free(comm);
                    closedir(proc);
                    return result;
                }
            }
            free(comm);
        }
        closedir(proc);
        return sendUnknown();

    #elif defined (__APPLE__)
        return strdup("Quartz Compositor");

    #else
        return sendUnknown();
    #endif
}


/*
            THEME RELATED INFO
*/

char *getTheme(void) {
    #if defined (__linux__)
        char *desktopEnv = getDesktopEnvironment();

        if(desktopEnv == NULL) return sendUnknown();

        if(strcmp(desktopEnv, "GNOME") == 0){
            free(desktopEnv);

            char *output = safeRun("gsettings get org.gnome.desktop.interface gtk-theme");

            if(output != NULL){
                output[strcspn(output, "\n")] = '\0';

                if(output[0] == '\''){
                    memmove(output, output + 1, strlen(output));
                }

                size_t outputLen = strlen(output);
                if(outputLen > 0 && output[outputLen - 1] == '\''){
                    output[outputLen - 1 ] = '\0';
                }
                return output;
            }
            return sendUnknown();
        }

        if(strcmp(desktopEnv, "KDE") == 0){
            free(desktopEnv);

            char *output = safeRun("kreadconfig6 --file kdeglobals --group General --key ColorScheme");

            if(output != NULL){
                output[strcspn(output, "\n")] = '\0';
                return output;
            }
            return sendUnknown();
        }
        free(desktopEnv);
        return sendUnknown();

    #else
        return sendUnknown();
    #endif
}


char *getWindowManagerTheme(void) {
    #if defined (__linux__)
        char *desktopEnv = getDesktopEnvironment();

        if(desktopEnv == NULL) return sendUnknown();

        if(strcmp(desktopEnv, "GNOME") == 0){
            free(desktopEnv);

            char *output = safeRun("gsettings get org.gnome.desktop.wm.preferences theme");

            if(output != NULL){
                output[strcspn(output, "\n")] = '\0';

                if(output[0] == '\''){
                    memmove(output, output + 1, strlen(output));
                }

                size_t outputLen = strlen(output);
                if(outputLen > 0 && output[outputLen - 1] == '\''){
                    output[outputLen - 1 ] = '\0';
                }
                return output;
            }
            return sendUnknown();
        }

        if(strcmp(desktopEnv, "KDE") == 0){
            free(desktopEnv);

            char *output = safeRun("kreadconfig6 --file kwinrc --group org.kde.kdecoration2 --key theme");

            if(output != NULL){
                output[strcspn(output, "\n")] = '\0';
                return output;
            }
            return sendUnknown();
        }
        free(desktopEnv);
        return sendUnknown();

    #else
        return sendUnknown();
    #endif
}


char *getIcons(void) {
    #if defined (__linux__)
        char *desktopEnv = getDesktopEnvironment();

        if(desktopEnv == NULL) return sendUnknown();

        if(strcmp(desktopEnv, "GNOME") == 0){
            free(desktopEnv);

            char *output = safeRun("gsettings get org.gnome.desktop.interface icon-theme");

            if(output != NULL){
                output[strcspn(output, "\n")] = '\0';

                if(output[0] == '\''){
                    memmove(output, output + 1, strlen(output));
                }

                size_t outputLen = strlen(output);
                if(outputLen > 0 && output[outputLen - 1] == '\''){
                    output[outputLen - 1 ] = '\0';
                }
                return output;
            }
            return sendUnknown();
        }

        if(strcmp(desktopEnv, "KDE") == 0){
            free(desktopEnv);

            char *output = safeRun("kreadconfig6 --file kdeglobals --group Icons --key Theme");

            if(output != NULL){
                output[strcspn(output, "\n")] = '\0';
                return output;
            }
            return sendUnknown();
        }
        free(desktopEnv);
        return sendUnknown();

    #else
        return sendUnknown();
    #endif
}


char *getFont(void) {
    #if defined (__linux__)
        char *desktopEnv = getDesktopEnvironment();

        if(desktopEnv == NULL) return sendUnknown();

        if(strcmp(desktopEnv, "GNOME") == 0){
            free(desktopEnv);

            char *output = safeRun("gsettings get org.gnome.desktop.interface font-name");

            if(output != NULL){
                output[strcspn(output, "\n")] = '\0';

                if(output[0] == '\''){
                    memmove(output, output + 1, strlen(output));
                }

                size_t outputLen = strlen(output);
                if(outputLen > 0 && output[outputLen - 1] == '\''){
                    output[outputLen - 1 ] = '\0';
                }
                return output;
            }
            return sendUnknown();
        }

        if(strcmp(desktopEnv, "KDE") == 0){
            free(desktopEnv);

            char *output = safeRun("kreadconfig6 --file kdeglobals --group General --key font");

            if(output != NULL){
                output[strcspn(output, "\n")] = '\0';
                return output;
            }
            return sendUnknown();
        }
        free(desktopEnv);
        return sendUnknown();

    #else
        return sendUnknown();
    #endif
}


/*
            TERMINAL
*/

char *getTerminal(void) {
    char *value = getenv("TERM_PROGRAM");
    if(value != NULL && value[0] != '\0') return strdup(value);
    
    value = getenv("TERMINAL_EMULATOR");
    if(value != NULL && value[0] != '\0') return strdup(value);

    value = getenv("TERM");
    if(value != NULL && value[0] != '\0') return strdup(value);

    return sendUnknown();
}


/*
            CPU & GPU
*/

char *getCPU(void) {
    #ifdef _WIN32
        char *cpu = getenv("PROCESSOR_IDENTIFIER");
        if(cpu != NULL && cpu[0] != '\0'){
            return strdup(cpu);
        }
        return sendUnknown();

    #elif defined (__linux__)
        char *contents = safeRead("/proc/cpuinfo");
        if(contents != NULL){
            char *model = strstr(contents, "model name");
            if(model != NULL){
                char *colon = strchr(model, ':');
                if(colon != NULL){
                    char *value = colon + 1;
                    while(isspace((unsigned char) * value)){
                        value++;
                    }

                    char *end = strchr(value, '\n');
                    if(end != NULL) *end = '\0';

                    char *result = malloc(strlen(value) + 1);
                    if(result == NULL){ 
                        free(contents);
                        return NULL;
                    }
                    strcpy(result, value);
                    free(contents);
                    return result;
                }
            }
            free(contents);
        }
        return sendUnknown();

    #elif defined (__APPLE__)
        char *output = safeRun("sysctl -n machdep.cpu.brand_string");

        if(output != NULL){
            size_t outputLen = strlen(output);
            if(outputLen > 0 && output[outputLen -1 ] == '\n') output[outputLen - 1] = '\0';
            return output;
        }
        return sendUnknown();

    #else
        return sendUnknown();
    #endif
}


char *getGPU(void) {
    #ifdef _WIN32
        char *output = safeRun("wmic path win32_VideoController get name");
        if(output != NULL){
            char *line = strtok(output, "\n");
            
            while(line != NULL){
                char *name = line;
                while(isspace((unsigned char)*name)){
                    name++;
                }
                name[strcspn(name, "\r\n")] = '\0';

                if(strcmp(name, "Name") != 0){
                    char *result = malloc(strlen(name) + 1);
                    if(result == NULL){
                        free(output);
                        return NULL;
                    }
                    strcpy(result, name);
                    free(output);
                    return result;
                }
                line = strtok(NULL, "\n");
            }
            free(output);
        }
        return sendUnknown();

    #elif defined (__linux__)
        char *output = safeRun("lspci");
        if(output != NULL){
            char *line = strtok(output, "\n");
            
            while(line != NULL){
                if(strstr(line, "VGA") != NULL || strstr(line, "3D controller") != NULL){
                    char *firstColon = strchr(line, ':');
                    if(firstColon == NULL){
                        free(output);
                        return sendUnknown();
                    }
                    char *secondColon = strchr(firstColon + 1, ':');
                    if(secondColon == NULL){
                        free(output);
                        return sendUnknown();
                    }

                    char *gpu = secondColon + 1;
                    while(isspace((unsigned char)*gpu)){
                        gpu++;
                    }

                    char *result = malloc(strlen(gpu) + 1);
                    if(result == NULL){
                        free(output);
                        return NULL;
                    }
                    strcpy(result, gpu);
                    free(output);
                    return result;
                }
                line = strtok(NULL, "\n");
            }
            free(output);
        }
        return sendUnknown();

    #elif defined (__APPLE__)
        char *output = safeRun("system_profiler SPDisplaysDataType");
        if(output != NULL){
            char *match = strstr(output, "Chipset Model: ");
            if(match != NULL){
                match += strlen("Chipset Model: ");

                char *end = strchr(match, '\n');
                if(end != NULL){
                    *end = '\0';
                }

                char *result = malloc(strlen(match) + 1);
                if(result == NULL){
                    free(output);
                    return NULL;
                }
                strcpy(result, match);
                free(output);
                return result;
            }
            free(output);
        }
        return sendUnknown();

    #else
        return sendUnknown();
    #endif
}

/*
            MEMORY USAGE
*/

char *getMemoryUsage(void){
    #ifdef _WIN32
        MEMORYSTATUSEX stat;
        stat.dwLength = sizeof(stat);

        if(GlobalMemoryStatusEx(&stat) != 0){
            double total_gb = (double)stat.ullTotalPhys / (1024.0 * 1024.0 * 1024.0);
            double avail_gb = (double)stat.ullAvailPhys / (1024.0 * 1024.0 * 1024.0);
            double used_gb = total_gb - avail_gb;

            char buffer[64];
            snprintf(buffer, sizeof(buffer), "%.1fGiB / %.1fGiB", used_gb, total_gb);

            char *result = malloc(strlen(buffer) + 1);
            if(result == NULL){
                return NULL;
            }
            strcpy(result, buffer);
            return result;
        }
        return sendUnknown();

    #elif defined (__linux__)
        char *contents = safeRead("/proc/meminfo");

        if(contents != NULL){
            char *totalMemory = strstr(contents, "MemTotal:");
            char *availableMemory = strstr(contents, "MemAvailable:");

            if(total != NULL && avail != NULL){
                totalMemory += strlen("MemTotal:");
                availableMemory += strlen("MemAvailable:");

                while(isspace((unsigned char)*totalMemory)){
                    totalMemory++;
                }
                while(isspace((unsigned char)*availableMemory)){
                    availableMemory++;
                }

                unsigned long long total_kb = strtoull(totalMemory, NULL, 10);
                unsigned long long avail_kb = strtoull(availableMemory, NULL, 10);
                unsigned long long used_kb = total_kb - avail_kb;

                double total_gb = (double)total_kb / 1048576.0;
                double used_gb = (double)used_kb / 1048576.0;

                char buffer[64];
                snprintf(buffer, sizeof(buffer), "%.1fGiB / %.1fGiB", used_gb, total_gb);

                char *result = malloc(strlen(buffer) + 1);
                if(result == NULL){
                    free(contents);
                    return NULL;
                }
                strcpy(result, buffer);
                free(contents);
                return result;
            }
            free(contents);
        }
        return sendUnknown();

    #elif defined (__APPLE__)
        char *total_output = safeRun("sysctl -n hw.memsize");
        char *vm_output = safeRun("vm_stat");

        if(total_output != NULL && vm_output != NULL){
            unsigned long long total_bytes = strtoull(total_output, NULL, 10);
            unsigned long long page_size = 4096;

            char *page_match = strstr(vm_output, "page size of ");
            if(page_match != NULL){
                page_match += strlen("page size of ");
                page_size = strtoull(page_match, NULL, 10);
            }

            unsigned long long free_pages = 0;

            char *free_match = strstr(vm_output, "Pages free:");
            if(free_match != NULL){
                free_match += strlen("Pages free:");
                while(isspace((unsigned char)*free_match)){
                    free_match++;
                }
                free_pages = strtoull(free_match, NULL, 10);
            }

            double total_gb = (double)total_bytes / (1024.0 * 1024.0 * 1024.0);
            double free_gb = ((double)free_pages * (double)page_size) / (1024.0 * 1024.0 * 1024.0);
            double used_gb = total_gb - free_gb;

            char buffer[64];
            snprintf(buffer, sizeof(buffer), "%.1fGiB / %.1fGiB", used_gb, total_gb);

            char *result = malloc(strlen(buffer) + 1);
            if(result == NULL){
                free(total_output);
                free(vm_output);
                return NULL;
            }

            strcpy(result, buffer);
            free(total_output);
            free(vm_output);
            return result;
        }
        free(total_output);
        free(vm_output);
        return sendUnknown();

    #else
        return sendUnknown();
    #endif
}