#pragma once

#ifdef _WIN32
#include <windows.h>
#include <cstring>
#include <string>

inline std::wstring utf8_to_utf16(const std::string& s) {
    if (s.empty()) return L"";
    int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0);
    std::wstring w(n, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), &w[0], n);
    return w;
}

inline bool copy_to_clipboard(const std::string& utf8) {
    std::wstring w = utf8_to_utf16(utf8);
    if (!OpenClipboard(nullptr)) return false;
    EmptyClipboard();

    size_t bytes = (w.size() + 1) * sizeof(wchar_t);
    HGLOBAL h = GlobalAlloc(GMEM_MOVEABLE, bytes);
    if (!h) { CloseClipboard(); return false; }
    memcpy(GlobalLock(h), w.c_str(), bytes);
    GlobalUnlock(h);
    SetClipboardData(CF_UNICODETEXT, h);
    CloseClipboard();
    return true;
}
#else
inline bool copy_to_clipboard(const std::string&) { return false; }
#endif