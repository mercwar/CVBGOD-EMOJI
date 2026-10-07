// Filename: emoji_core.h
#ifndef EMOJI_CORE_H
#define EMOJI_CORE_H

#include <windows.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
    Unicode range scanned by the pager.

    MAX_ID = 0x20000
    ITEMS_PER_PAGE = 256

    TOTAL_PAGES = 512
*/

#define MAX_ID         0x20000u
#define ITEMS_PER_PAGE 256u
#define TOTAL_PAGES \
    ((MAX_ID + ITEMS_PER_PAGE - 1u) / ITEMS_PER_PAGE)

/* Emoji detection */

int is_known_emoji(uint32_t cp);

int page_has_emoji(int page);

/* Unicode conversion */

void id_to_utf16(
    uint32_t cp,
    WCHAR* out
);

/* Bulk page text generation */

void get_page_text(
    int page_num,
    WCHAR* dest,
    size_t max_len
);

/* Reserved for edit-control mapping */

uint32_t get_id_at_char_index(
    HWND hEdit,
    int char_index,
    WCHAR* out_emoji,
    int emoji_max_len
);

/* HTML page generation consumed by IWebBrowser2 */

void generate_page_html(
    int page_num,
    int filter_emoji_only,
    const WCHAR* status_msg
);

#ifdef __cplusplus
}
#endif

#endif /* EMOJI_CORE_H */