#include "pdf.h"

#include <sstream>
#include <algorithm>
#include <cmath>
#include <iostream>

pdf::pdf(const char* filepath)
{
    _filepath = filepath;
    _good = true;
    _image_name = nullptr;
    _needs_password = false;
    _size = 0;

    // Create a context to hold the exception stack and various caches.
    _ctx = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);

    if (!_ctx)
    {
        _good = false;
        return;
    }

    fz_try(_ctx)
    {
        // Register the default file types.
        fz_register_document_handlers(_ctx);

        // Open the PDF, XPS or CBZ document.
        _doc = fz_open_document(_ctx, filepath);

        if (fz_needs_password(_ctx, _doc))
        {
            _needs_password = true;
            _good = false;
        }
        else
        {
            // Retrieve the number of pages (not used in this example).
            _size = fz_count_pages(_ctx, _doc);
        }
    }
    fz_catch(_ctx)
    {
        _doc = 0;
        _good = false;
    }
}

pdf::~pdf()
{
    // Clean up.
    fz_try(_ctx)
    {
        if (_doc)
        {
            fz_drop_document(_ctx, _doc);
        }
    }
    fz_catch(_ctx)
    {
        // Nothing can be done here...
    }

    fz_drop_context(_ctx);
}

bool pdf::needs_password()
{
    return _needs_password;
}

bool pdf::set_password(const char* password)
{
    fz_try(_ctx)
    {
        if (!fz_authenticate_password(_ctx, _doc, password))
        {
            // _needs_password = true;
        }
        else
        {
            _needs_password = false;
            _good = true;

            // Retrieve the number of pages (not used in this example).
            _size = fz_count_pages(_ctx, _doc);
        }
    }
    fz_catch(_ctx)
    {
        _good = false;
    }

    return !_needs_password;
}

int pdf::size()
{
    return _size;
}

bool pdf::good()
{
    return _good;
}

void pdf::generate_image_name(int page, int width)
{
    std::stringstream ss;
    ss.width(width);
    ss.fill('0');
    ss << std::right << page;

    if (_image_name == nullptr)
    {
        std::string filename = _export_filename + '.' + ss.str() + ".png";
        _image_name = new char[filename.size() + 1];
        std::copy(filename.begin(), filename.end(), _image_name);
        _image_name[filename.size()] = '\0';
    }
    else
    {
        std::string number = ss.str();
        std::copy(number.begin(), number.end(),
            &_image_name[_export_filename.size() + 1]);
    }
}

bool pdf::render(const char* export_filename, int from, int to, int zoom)
{
    if (!_good) return false;

    _export_filename = export_filename;
    int width = std::log10(to) + 1;

    // reset _image_name, in case width is different.
    if (_image_name)
    {
        delete[] _image_name;
        _image_name = nullptr;
    }

    // Calculate a transform to use when rendering. This transform
    // contains the scale and rotation. Convert zoom percentage to a
    // scaling factor. Without scaling the resolution is 72 dpi.
    fz_matrix transform = fz_rotate(0);
    transform = fz_pre_scale(transform, zoom / 72.0f, zoom / 72.0f);

    for (int i = from; i <= to; ++i)
    {
        generate_image_name(i, width);

        fz_try(_ctx)
        {
            fz_page* page;
            fz_rect bounds;
            fz_irect bbox;
            fz_pixmap* pix;
            fz_device* dev;

            // Load the page we want. Page numbering starts from zero.
            page = fz_load_page(_ctx, _doc, i - 1);

            // Take the page bounds and transform them by the same matrix that
            // we will use to render the page.
            bounds = fz_bound_page(_ctx, page);
            bounds = fz_transform_rect(bounds, transform);

            // Create a blank pixmap to hold the result of rendering. The
            // pixmap bounds used here are the same as the transformed page
            // bounds, so it will contain the entire page. The page coordinate
            // space has the origin at the top left corner and the x axis
            // extends to the right and the y axis extends down.

            bbox = fz_round_rect(bounds);
            pix = fz_new_pixmap_with_bbox(_ctx, fz_device_rgb(_ctx), bbox, nullptr, 0);
            fz_clear_pixmap_with_value(_ctx, pix, 0xff);

            // Create a draw device with the pixmap as its target.
            // Run the page with the transform.
            // This part of the code needs a new transform for some reason. So we instantiate a fresh one.
            fz_matrix dev_transform = fz_rotate(0);
            dev = fz_new_draw_device(_ctx, dev_transform, pix);
            fz_run_page(_ctx, page, dev, transform, NULL);
            fz_close_device(_ctx, dev);
            fz_drop_device(_ctx, dev);

            // Save the pixmap to a file.
            fz_output* outputstream = fz_new_output_with_path(_ctx, _image_name, 0);
            fz_write_pixmap_as_png(_ctx, outputstream, pix);

            printf("%s\n", _image_name);
            // Clean up.
            fz_drop_output(_ctx, outputstream);
            fz_drop_pixmap(_ctx, pix);
            fz_drop_page(_ctx, page);
        }
        fz_catch(_ctx)
        {
            return false;
        }
    }

    return true;
}