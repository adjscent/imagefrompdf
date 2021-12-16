#include "pdf.h"
#include <iostream>

int main(int argc, char* argv[])
{
    if (argc == 1 || argc >= 4)
    {
        printf("\nThe syntax of this command is:\nimagefrompdf.exe pdf_file_path zoom_level(100)\n");
        printf("\nDescription:\n Converts all the pages in the specified pdf to individual png.\n");
    	return 1;
    }

    std::string filename = argv[1];
    int zoom = std::stoi(argv[2]);
    std::string directory;

    const size_t last_slash_idx = filename.rfind('\\');
    if (std::string::npos != last_slash_idx)
    {
        directory = filename.substr(0, last_slash_idx);
        printf("Filename is %s\n", filename.c_str());
        printf("Outputting to %s\n", directory.c_str());
    }
    else
    {
        printf("Error!");
    }

    pdf* _pdf = new pdf(filename.c_str());

    if (!(_pdf->good() && _pdf->size() != 0))
    {
        printf("Error loading pdf!\n");
        return 1;
    }

    int from = 1;
    int to = _pdf->size();

    // this will use the filename as a generator
    if (_pdf->render(filename.c_str(), from, to, zoom))
    {
        printf("Success!\n");
    }
    else 
    {
        printf("Error generating png!\n");
    }
}
