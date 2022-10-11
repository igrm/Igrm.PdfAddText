#include <qpdf/qpdf-c.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct pair
{
    int length;
    qpdf_oh* oh;
};

typedef struct pair flatter_result;

flatter_result flattern(const char* type, qpdf_data qpdf, qpdf_oh oh)
{
    flatter_result result = {0, NULL};

    if (strcmp(type, "array") == 0)
    {
        int count = qpdf_oh_get_array_n_items(qpdf, oh);
        qpdf_oh* temp = malloc(count*sizeof(qpdf_oh));
        for (int i =0; i<count; i++)
        {
            temp[i] = qpdf_oh_get_array_item(qpdf, oh, i);
        }
        result = (flatter_result){.length = count, .oh = temp};
    }
    else if (strcmp(type, "stream") == 0)
    {
        qpdf_oh* temp = malloc(sizeof(qpdf_oh));
        temp[0] = oh;
        result = (flatter_result){.length = 1, .oh = temp};
    }

    return result;
}

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
               qpdf_oh contents_object_handle = qpdf_oh_get_key(qpdf, page_object_handle, key);
               printf("Contents object handle: %d\n", contents_object_handle);
               const char* type = qpdf_oh_get_type_name(qpdf, contents_object_handle);
               printf("%s\n", type);

               flatter_result multiple_object_handles = flattern(type, qpdf, contents_object_handle);
               printf("Length found:%d\n",multiple_object_handles.length);
               for (int i = 0; i < multiple_object_handles.length; i++)
               {
                  char* bufp = NULL;
                  size_t len = 0;
                  QPDF_BOOL filterable;

                  printf("Processing handle %d\n", multiple_object_handles.oh[i]);
                  qpdf_oh_get_stream_data(qpdf, multiple_object_handles.oh[i], qpdf_dl_all,  &filterable, &bufp, &len);
                  printf("%lu\n", len);
                  printf("%s\n", bufp);

                  char* newbuf = malloc(len+sizeof(char)*100+sizeof(char)*strlen(argv[3]));
                  strcpy(newbuf, bufp);
                  strcat(newbuf, "BT\n");
                  strcat(newbuf, "/TT0 1 Tf\n");
                  strcat(newbuf, "(");
                  strcat(newbuf, argv[3]);
                  strcat(newbuf, ") Tj\n");
                  strcat(newbuf, "ET\n");
                  
                 

                  qpdf_oh_replace_stream_data(qpdf, multiple_object_handles.oh[i], newbuf, strlen(newbuf), qpdf_oh_get_key(qpdf, page_object_handle, "/Filter"), qpdf_oh_get_key(qpdf, page_object_handle, "/DecodeParms"));
                  
                    if ((qpdf_init_write(qpdf, "result.pdf") & QPDF_ERRORS) == 0)
                    {
                        qpdf_set_static_ID(qpdf, QPDF_TRUE); /* for testing only */
                        qpdf_set_linearization(qpdf, QPDF_TRUE);
                        qpdf_set_stream_data_mode(qpdf, qpdf_s_uncompress);
                        qpdf_write(qpdf);
                    }
                    else 
                    {
                        printf("Write failed with the following:\n");
                        printf("%s\n",qpdf_get_error_full_text(qpdf, qpdf_get_error(qpdf)));
                    }

                  free(bufp);
                  free(newbuf);

                  break;
               }
               free(multiple_object_handles.oh);
            }
            key = qpdf_oh_dict_next_key(qpdf);
        }
    }
    else
    {
        printf("Error occured while saving document %s\n", argv[1]);
        fprintf(stderr, "%s\n",qpdf_get_error_full_text(qpdf, qpdf_get_error(qpdf)));
        exit(-1);
    }

    qpdf_cleanup(&qpdf);
    return 0;
}