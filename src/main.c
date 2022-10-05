#include <qpdf/qpdf-c.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv) 
{
    qpdf_data qpdf = qpdf_init();

    if (argc != 4)
    {
        printf("Usage: %s document page text\n", argv[0]);
        exit(0);
    }

    
    if ((qpdf_read(qpdf, argv[1], NULL) & QPDF_ERRORS) == 0)
    {
        printf("%s opened.\n", argv[1]);
        int total_pages = qpdf_get_num_pages(qpdf);
        printf("%d pages in total\n ", total_pages);
        
    }
    else
    {
        printf("Error occured while opening %s\n", argv[1]);
        printf( "%s\n",qpdf_get_error_full_text(qpdf, qpdf_get_error(qpdf)));
        exit(-1);
    }

    qpdf_cleanup(&qpdf);
    return 0;
}