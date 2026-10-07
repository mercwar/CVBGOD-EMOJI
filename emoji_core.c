// Filename: emoji_core.h / core logic updates in emoji_core.c
#include "emoji_core.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <stdint.h>

int is_known_emoji(uint32_t cp)
{
    return
    (cp >= 0x1F300 && cp <= 0x1FAFF) ||
    (cp >= 0x2600 && cp <= 0x27BF) ||
    (cp >= 0x1F000 && cp <= 0x1F2FF);
}

int page_has_emoji(int page)
{
    int start = page * ITEMS_PER_PAGE;
    int end = start + ITEMS_PER_PAGE;
     
    if (end > MAX_ID)
        end = MAX_ID;
     
    for (int i = start; i < end; ++i)
    {
        if (is_known_emoji((uint32_t)i))
            return 1;
    }
     
    return 0;
}

void id_to_utf16(uint32_t cp, WCHAR* out)
{
    if (!out)
        return;
     
    uint32_t original_cp = cp;
     
    if (cp > 0x10FFFF)
        cp = 0xFFFD;
     
    int len = 0;
     
    if (cp <= 0xFFFF)
    {
        if (cp >= 0xD800 && cp <= 0xDFFF)
            cp = 0xFFFD;
         
        out[0] = (WCHAR)cp;
        len = 1;
    }
    else
    {
        cp -= 0x10000;
         
        out[0] = (WCHAR)(0xD800 + (cp >> 10));
        out[1] = (WCHAR)(0xDC00 + (cp & 0x3FF));
         
        len = 2;
    }
     
    if (is_known_emoji(original_cp))
    {
        out[len++] = 0xFE0F;
    }
     
    out[len] = 0;
}

void get_page_text(int page_num, WCHAR* dest, size_t max_len)
{
    if (!dest || max_len == 0)
        return;
     
    dest[0] = 0;
     
    int start = page_num * ITEMS_PER_PAGE;
    int end = start + ITEMS_PER_PAGE;
     
    if (end > MAX_ID)
        end = MAX_ID;
     
    size_t pos = 0;
     
    for (int i = start; i < end; ++i)
    {
        WCHAR emoji[8];
        id_to_utf16((uint32_t)i, emoji);
        size_t len = wcslen(emoji);
         
        if ((pos + len + 1) >= max_len)
            break;
         
        wcscpy_s(dest + pos, max_len - pos, emoji);
        pos += len;
    }
}

uint32_t get_id_at_char_index(HWND hEdit, int char_index, WCHAR* out_emoji, int emoji_max_len)
{
    UNREFERENCED_PARAMETER(hEdit);
    UNREFERENCED_PARAMETER(char_index);
     
    if (out_emoji && emoji_max_len > 0)
        out_emoji[0] = 0;
     
    return 0;
}

static void html_escape(const WCHAR* src, WCHAR* dst, size_t dst_len)
{
    if (!dst || dst_len == 0)
        return;
     
    dst[0] = 0;
     
    if (!src)
        return;
     
    size_t dst_pos = 0;
    while (*src && dst_pos + 1 < dst_len)
    {
        WCHAR ch = *src;
        const WCHAR* entity = NULL;
        size_t entity_len = 0;

        if (ch == 0x0026)      { entity = L"&amp;";  entity_len = 5; }
        else if (ch == 0x003C) { entity = L"&lt;";   entity_len = 4; }
        else if (ch == 0x003E) { entity = L"&gt;";   entity_len = 4; }
        else if (ch == 0x0022) { entity = L"&quot;"; entity_len = 6; }
        else if (ch == 0x0027) { entity = L"&#39;";  entity_len = 5; }
         
        if (entity)
        {
            if (dst_pos + entity_len >= dst_len)
                break;
            wcscpy_s(dst + dst_pos, dst_len - dst_pos, entity);
            dst_pos += entity_len;
        }
        else
        {
            dst[dst_pos++] = ch;
        }
        src++;
    }
    dst[dst_pos] = 0;
}

void generate_page_html(int page_num, int filter_emoji_only, const WCHAR* status_msg)
{
    FILE* f = NULL;
     
    if (_wfopen_s(&f, L"page.html", L"w, ccs=UTF-8"))
        return;
     
    if (!f)
        return;
     
    fwprintf(f, L"<!DOCTYPE html>\n<html>\n<head>\n");
    fwprintf(f, L"<meta charset=\"UTF-8\">\n");
    fwprintf(f, L"<meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\n");
    fwprintf(f, L"<title>Emoji Page %d</title>\n", page_num);
    fwprintf(f, L"<style>\n");
    fwprintf(f, L"body { font-family: 'Segoe UI', Consolas, monospace; padding: 20px; background-color: #080c14; color: #c0d0e0; "
                L"background-image: linear-gradient(rgba(0, 240, 255, 0.03) 1px, transparent 1px), linear-gradient(90deg, rgba(0, 240, 255, 0.03) 1px, transparent 1px); "
                L"background-size: 20px 20px; margin: 0; user-select: none; }\n");
    fwprintf(f, L".status { background: rgba(0, 240, 255, 0.05); border-left: 4px solid #00f0ff; padding: 12px 18px; margin-bottom: 20px; "
                L"color: #00f0ff; font-weight: bold; font-family: 'Consolas', monospace; text-shadow: 0 0 6px rgba(0,240,255,0.4); "
                L"box-shadow: inset 0 0 10px rgba(0,240,255,0.1); border-radius: 0 4px 4px 0; }\n");
    fwprintf(f, L".emoji-grid { display: flex; flex-wrap: wrap; gap: 10px; background: rgba(12, 18, 30, 0.9); "
                L"border: 1px solid rgba(0, 240, 255, 0.3); padding: 25px; border-radius: 6px; box-shadow: 0 0 25px rgba(0, 240, 255, 0.12); }\n");
    fwprintf(f, L".emoji-grid span { display: inline-flex; width: 48px; height: 48px; align-items: center; justify-content: center; "
                L"font-family: 'Segoe UI Emoji', sans-serif; font-size: 26px; background: #111824; border: 1px solid #1e2c44; "
                L"border-radius: 4px; cursor: pointer; transition: all 0.2s ease; }\n");
    fwprintf(f, L".emoji-grid span:hover { background: #1a2a44; border-color: #00f0ff; box-shadow: 0 0 12px rgba(0, 240, 255, 0.6); transform: scale(1.1); }\n");
    
    // Context Menu Styling
    fwprintf(f, L"#ctx-menu { display: none; position: absolute; z-index: 1000; background: #0c121e; border: 1px solid #00f0ff; "
                L"box-shadow: 0 0 15px rgba(0, 240, 255, 0.4); border-radius: 4px; padding: 6px 0; min-width: 160px; font-size: 13px; }\n");
    fwprintf(f, L".ctx-item { padding: 9px 16px; color: #c0d0e0; cursor: pointer; font-family: 'Segoe UI', sans-serif; }\n");
    fwprintf(f, L".ctx-item:hover { background: #00f0ff; color: #080c14; font-weight: bold; }\n");
    fwprintf(f, L"</style>\n</head>\n<body>\n");
     
    if (status_msg && status_msg[0] != 0)
    {
        WCHAR escaped_msg[512];
        html_escape(status_msg, escaped_msg, 512);
        fwprintf(f, L"<div class=\"status\">[HUD STATUS]: %s</div>\n", escaped_msg);
    }
     
    fwprintf(f, L"<div class=\"emoji-grid\">\n");
     
    int start = page_num * ITEMS_PER_PAGE;
    int end = start + ITEMS_PER_PAGE;
    if (end > MAX_ID)
        end = MAX_ID;
         
    int count = 0;
    for (int i = start; i < end; ++i)
    {
        int is_emoji = is_known_emoji((uint32_t)i);
         
        if (filter_emoji_only && !is_emoji)
            continue;
             
        WCHAR emoji_str[8];
        id_to_utf16((uint32_t)i, emoji_str);
        
        fwprintf(f, L"  <span data-emoji=\"%s\" data-hex=\"U+%04X\">%s</span>\n", emoji_str, i, emoji_str);
        count++;
    }

    if (count == 0)
    {
        fwprintf(f, L"<p style=\"color: #6a82a0;\">[!] No tactical emoji data found on this sector.</p>\n");
    }
     
    fwprintf(f, L"</div>\n");

    // Context Menu Markup with HTML entities for icons
    fwprintf(f, L"<div id=\"ctx-menu\">\n");
    fwprintf(f, L"  <div class=\"ctx-item\" onclick=\"copyData('emoji')\">&#x1F4CB; Copy Emoji</div>\n");
    fwprintf(f, L"  <div class=\"ctx-item\" onclick=\"copyData('hex')\">&#x1F522; Copy Hex (U+...)</div>\n");
    fwprintf(f, L"</div>\n");

    fwprintf(f, L"<script>\n");
    fwprintf(f, L"  var menu = document.getElementById('ctx-menu');\n");
    fwprintf(f, L"  var currentEmoji = '';\n");
    fwprintf(f, L"  var currentHex = '';\n\n");
    fwprintf(f, L"  document.onclick = function() { menu.style.display = 'none'; };\n\n");
    fwprintf(f, L"  var spans = document.querySelectorAll('.emoji-grid span');\n");
    fwprintf(f, L"  for (var i = 0; i < spans.length; i++) {\n");
    fwprintf(f, L"    spans[i].oncontextmenu = function(e) {\n");
    fwprintf(f, L"      if (!e) e = window.event;\n");
    fwprintf(f, L"      if (e.preventDefault) { e.preventDefault(); }\n");
    fwprintf(f, L"      e.returnValue = false;\n");
    fwprintf(f, L"      currentEmoji = this.getAttribute('data-emoji');\n");
    fwprintf(f, L"      currentHex = this.getAttribute('data-hex');\n");
    fwprintf(f, L"      menu.style.left = (e.pageX || e.clientX) + 'px';\n");
    fwprintf(f, L"      menu.style.top = (e.pageY || e.clientY) + 'px';\n");
    fwprintf(f, L"      menu.style.display = 'block';\n");
    fwprintf(f, L"      return false;\n");
    fwprintf(f, L"    };\n");
    fwprintf(f, L"  }\n\n");
    fwprintf(f, L"  function copyData(type) {\n");
    fwprintf(f, L"    var val = (type === 'emoji') ? currentEmoji : currentHex;\n");
    fwprintf(f, L"    if (window.clipboardData && window.clipboardData.setData) {\n");
    fwprintf(f, L"      window.clipboardData.setData('Text', val);\n");
    fwprintf(f, L"    } else {\n");
    fwprintf(f, L"      var textarea = document.createElement('textarea');\n");
    fwprintf(f, L"      textarea.value = val;\n");
    fwprintf(f, L"      textarea.style.position = 'fixed';\n");
    fwprintf(f, L"      document.body.appendChild(textarea);\n");
    fwprintf(f, L"      textarea.focus();\n");
    fwprintf(f, L"      textarea.select();\n");
    fwprintf(f, L"      try { document.execCommand('copy'); } catch (err) {}\n");
    fwprintf(f, L"      document.body.removeChild(textarea);\n");
    fwprintf(f, L"    }\n");
    fwprintf(f, L"    menu.style.display = 'none';\n");
    fwprintf(f, L"  }\n");
    fwprintf(f, L"</script>\n");

    fwprintf(f, L"</body>\n</html>\n");
    fclose(f);
}