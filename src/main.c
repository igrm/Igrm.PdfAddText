#include <qpdf/qpdf-c.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv) 
{
    unsigned char* bufp = NULL;
    size_t len = 0;

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
        printf("%d pages in total\n", total_pages);
        

        qpdf_oh page_object_handle = qpdf_get_page_n(qpdf, (atoi(argv[2])-1));
        printf("Page object handle: %d\n", page_object_handle);
        printf("Page object type name: %s\n", qpdf_oh_get_type_name(qpdf, page_object_handle));
        qpdf_oh_begin_dict_key_iter(qpdf,page_object_handle);
        
        const char* key = NULL;
        key = qpdf_oh_dict_next_key(qpdf);
        while(key!=NULL)
        {
            if (strcmp(key, "/Contents") == 0) 
            { 
               QPDF_BOOL filterable;
               qpdf_oh contents_object_handle = qpdf_oh_get_key(qpdf, page_object_handle, key);
               printf("Contents object handle: %d\n", contents_object_handle);
               printf("%s\n", qpdf_oh_get_type_name(qpdf, contents_object_handle));

               qpdf_oh_get_stream_data(qpdf, contents_object_handle, qpdf_dl_all,  &filterable, &bufp, &len);
               printf("%lu\n", len);
               printf("%s", bufp);
            }
            key = qpdf_oh_dict_next_key(qpdf);
        }
    }
    else
    {
        printf("Error occured while opening %s\n", argv[1]);
        printf( "%s\n",qpdf_get_error_full_text(qpdf, qpdf_get_error(qpdf)));
        exit(-1);
    }

    qpdf_cleanup(&qpdf);
    free(bufp);
    return 0;
}