// Filename: emoji_root.h

#ifndef EMOJI_ROOT_H
#define EMOJI_ROOT_H

#include <windows.h>
#include <stdio.h>

// Retrieves the full absolute path for a file relative to the running application directory
static inline void get_app_path(const WCHAR* filename, WCHAR* out_path, size_t max_path) {
    WCHAR exe_path[MAX_PATH];
    GetModuleFileNameW(NULL, exe_path, MAX_PATH);
    
    // Strip the executable name to isolate the root directory
    WCHAR* last_slash = wcsrchr(exe_path, L'\\');
    if (last_slash) {
        *(last_slash + 1) = L'\0';
    } else {
        exe_path[0] = L'\0';
    }

    swprintf_s(out_path, max_path, L"%s%s", exe_path, filename);
}

#endif // EMOJI_ROOT_H