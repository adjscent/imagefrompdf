#pragma once

extern "C" {
#include <mupdf/fitz.h>
}

#include <string>

class pdf
{
private:
    fz_context  *_ctx;
    fz_document *_doc;

    bool         _good;
    bool         _needs_password;
    char        *_image_name;
    int          _size;
    std::string  _filepath;
    std::string  _export_filename;

    void generate_image_name(int page, int width);

public:
    pdf(const char *filepath);
    ~pdf();

    bool needs_password();
    bool set_password(const char *password);

    int  size();
    bool good();
    bool render(const char *export_filename, int from, int to, int zoom = 100);
};