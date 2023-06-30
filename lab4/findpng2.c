#include "findpng2.h"
#define NUM_EMPL 5000
typedef unsigned char U8;
typedef unsigned int  U32;
int global_test = 8;
pthread_mutex_t global_mutex;

static struct hsearch_data *visited_urls;
static struct hsearch_data *check_duplicate_png;
int visited_png = 0;

FILE *fpng;

FILE *fp;

typedef struct thread_arguments{
 struct Queue *url_frontier;
 int m;
} thread_arguments;

int process_html(CURL *curl_handle, RECV_BUF *p_recv_buf, struct Queue *url_frontier)
{
    char fname[256];
    int follow_relative_link = 1;
    char *url = NULL; 
    pid_t pid = getpid();

    curl_easy_getinfo(curl_handle, CURLINFO_EFFECTIVE_URL, &url);
    
    printf("this is what is inside the recv_buf %d\n",p_recv_buf->size);
    find_http(p_recv_buf->buf, p_recv_buf->size, follow_relative_link, url, url_frontier); 
    sprintf(fname, "./output_%d.html", pid);
    return 0;
}


int process_png(CURL *curl_handle, RECV_BUF *p_recv_buf, struct Queue *url_frontier)
{
    pid_t pid =getpid();
    char fname[256];
    char *eurl = NULL;          /* effective URL */
    curl_easy_getinfo(curl_handle, CURLINFO_EFFECTIVE_URL, &eurl);
    if ( eurl != NULL) {
        printf("The PNG url is: %s\n", eurl);
    }

    ENTRY item;
    item.key = strdup(eurl);
    item.data = strdup("Garbage");
    ENTRY *retval; 
    int x = hsearch_r(item, FIND, &retval, check_duplicate_png); //result is 1 if found
    if(x == 0){
        hsearch_r(item, ENTER, &retval, check_duplicate_png);
        fprintf(fpng, "%s\n", eurl);
        sprintf(fname, "./output_%d_%d.png", p_recv_buf->seq, pid);
        visited_png++;
    }

    return 0;    

}

int find_http(char *buf, int size, int follow_relative_links, const char *base_url, struct Queue *url_frontier)
{
    printf("Base url: %s\n", base_url);
    int i;
    htmlDocPtr doc;
    xmlChar *xpath = (xmlChar*) "//a/@href";
    xmlNodeSetPtr nodeset;
    xmlXPathObjectPtr result;
    xmlChar *href;
		
    if (buf == NULL) {
        return 1;
    }

    doc = mem_getdoc(buf, size, base_url);
    result = getnodeset (doc, xpath);
    if (result) {
        nodeset = result->nodesetval;
        for (i=0; i < nodeset->nodeNr; i++) {
            href = xmlNodeListGetString(doc, nodeset->nodeTab[i]->xmlChildrenNode, 1);
            if ( follow_relative_links ) {
                xmlChar *old = href;
                href = xmlBuildURI(href, (xmlChar *) base_url);
                xmlFree(old);
            }
            if ( href != NULL && !strncmp((const char *)href, "http", 4) ) {
                printf("Href link: %s\n", href);
                char *href_copy = strdup(href); //????????????? why
                // Add logic to add things inside the stack and hashmap

                //Add everything to hashmap
                ENTRY item;
                item.key = strdup(href_copy);
                item.data = strdup("Garbage");

                //Add to url_frontier
                ENTRY *retval; 
                int x = hsearch_r(item, FIND, &retval, visited_urls); //result is 1 if found
                if(x == 0){
                    hsearch_r(item, ENTER, &retval, visited_urls); //I think I have to move hsearch_r around
                    enqueue(url_frontier, href_copy); 
                    //Write to log.txt
                    CURL *curl_handle;
                    RECV_BUF recv_buf;
                    curl_handle = easy_handle_init(&recv_buf, href_copy);
                    CURLcode res;
                    res = curl_easy_perform(curl_handle);
                    if(res != CURLE_OK){
                        printf("Broken link\n");
                    }
                    else{
                        fprintf(fp, "%s\n", href_copy);
                    }
                }
            }
            xmlFree(href);
        }
        xmlXPathFreeObject (result);
    }
    xmlFreeDoc(doc);
    //xmlCleanupParser(); //remove later bc throws double free
    return 0;
}

/**
 * @brief process teh download data by curl
 * @param CURL *curl_handle is the curl handler
 * @param RECV_BUF p_recv_buf contains the received data. 
 * @return 0 on success; non-zero otherwise
 */
//url_frontier not allowed to have duplicates
int process_data(CURL *curl_handle, RECV_BUF *p_recv_buf, struct Queue *url_frontier)
{
    CURLcode res;
    char fname[256];
    pid_t pid =getpid();
    long response_code;
    
    res = curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &response_code);
    if ( res == CURLE_OK ) {
	    printf("Response code: %ld\n", response_code);
    }

    //might put hashmap logic here

    if ( response_code >= 400 ) { 
    	fprintf(stderr, "Error.\n");
        return 1;
    }

    char *ct = NULL;
    res = curl_easy_getinfo(curl_handle, CURLINFO_CONTENT_TYPE, &ct);
    if ( res == CURLE_OK && ct != NULL ) {
    	printf("Content-Type: %s, len=%ld\n", ct, strlen(ct));
    } else {
        fprintf(stderr, "Failed obtain Content-Type\n");
        return 2;
    }

    if ( strstr(ct, CT_HTML) ) {
        return process_html(curl_handle, p_recv_buf, url_frontier);
    } else if ( strstr(ct, CT_PNG) ) {
        //Check if valid png (might be fake)

        //----------------------------check if the image is a png ------------------------
        U8 *giant_array = malloc(8);
        //write the header
        giant_array[0] = 0x89;
        giant_array[1] = 0x50;
        giant_array[2] = 0x4E;
        giant_array[3] = 0x47;
        giant_array[4] = 0x0D;
        giant_array[5] = 0x0A;
        giant_array[6] = 0x1A;
        giant_array[7] = 0x0A;
        int is_png = 1;
        U8 png_header[8];
        memcpy(png_header, p_recv_buf->buf,8);
        for(int i = 0; i < 8; ++i){
            if(giant_array[i] == png_header[i]){
                ;
            }
            else{
                //not a png
                is_png = 0;
            }
        }
        free(giant_array);
        //-------------------------------------------------------------------------------------

        if(is_png == 1){
            return process_png(curl_handle, p_recv_buf, url_frontier);
        }
    } else {
        sprintf(fname, "./output_%d", pid);
    }

    return 0;
}

void *thread_function(void *arg){
    thread_arguments *input = arg;
    struct Queue *url_frontier = input->url_frontier;
    int max_unique_png = input->m;

    if(max_unique_png == 0){
        //user gave input -m 0
        return 0;
    }
    while(1){
        //if queue is empty, there are no more links to find
        if(isEmpty(url_frontier) == 1){
            break;
        }
        if(visited_png == max_unique_png){
            break;
        }
        //On first run, this dequeues the seed url
        char *url_to_search = dequeue(url_frontier);
        
        CURL *curl_handle;
        CURLcode res;
        RECV_BUF recv_buf;
        curl_handle = easy_handle_init(&recv_buf, url_to_search);
        if ( curl_handle == NULL ) {
            fprintf(stderr, "Curl initialization failed. Exiting...\n");
            curl_global_cleanup();
            abort();
        }
        // get it
        res = curl_easy_perform(curl_handle);
        if( res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            cleanup(curl_handle, &recv_buf);
            //exit(1);
        } 
        else {
            process_data(curl_handle, &recv_buf, url_frontier);
            cleanup(curl_handle, &recv_buf);
        }
    }
    return 0;
}
//Visited urls is my hashmap. Url_frontier is my queue
//t is num of threads, m is # of png urls to find before exiting, v is a logfile to store all visited urls
int main( int argc, char** argv ) {
    //hashmap
    visited_urls = (struct hsearch_data *)calloc(1,sizeof(struct hsearch_data));
    check_duplicate_png = (struct hsearch_data *)calloc(1,sizeof(struct hsearch_data));
    hcreate_r(1000, visited_urls); 
    hcreate_r(1000, check_duplicate_png);

    fpng = fopen("png_urls.txt", "w"); 
    fp = fopen("log.txt","w");
    int opt;
    int t = -1;
    int m = -1;
    char *txt_file = NULL;
    int number_of_arguments = 0;
    pthread_mutex_init(&global_mutex, NULL);
    //timer info
    double times[2];
    struct timeval tv;
    CURL *curl_handle;
    CURLcode res;
    char *url = malloc(256);
    RECV_BUF recv_buf;
    if(argc == 1){
        printf("No server argument\n");
        exit(0);
    }

    while((opt = getopt(argc,argv, "t:m:v:")) != -1){
        switch(opt){
            case 't':
                t = strtoul(optarg, NULL, 10); 
                if(t > 0){
                    //printf("Number of threads is: %d\n", t);
                    break;
                }
            case 'm':
                m = strtoul(optarg, NULL, 10);
                if(m > 0){
                    //printf("Number of unique png_urls is: %d\n", m);
                    break;
                }
            case 'v':
                //printf("Logfile is: %s\n", optarg);
                txt_file = optarg;
                break;
        }
        number_of_arguments++;
    }

    if(t == -1){
        t = 1;
    }
    if(m == -1){
        m = 50;
    }
    if(txt_file == NULL){
        printf("No txt file specified\n\n");
    }
    printf("Number of threads is: %d\n", t);
    printf("Number of unique png_urls is: %d\n", m);
    printf("Logfile is: %s\n", txt_file);

    //if the timing module fails, we print the error
    if (gettimeofday(&tv, NULL) != 0) {
        perror("gettimeofday");
        abort();
    }
    //this is the intial time
    times[0] = (tv.tv_sec) + tv.tv_usec/1000000.;
    if (gettimeofday(&tv, NULL) != 0) {
        perror("gettimeofday");
        abort();
    }

    
    printf("Start of main program\n\n"); ///////////////////////////////

    memcpy(url, argv[(number_of_arguments*2) + 1], strlen(argv[(number_of_arguments*2) + 1]) + 1);
    printf("%s: URL is %s\n", argv[0], url);
    fprintf(fp, "%s\n", url);

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl_handle = easy_handle_init(&recv_buf, url);

    if ( curl_handle == NULL ) {
        fprintf(stderr, "Curl initialization failed. Exiting...\n");
        curl_global_cleanup();
        abort();
    }
    /* get it! */
    res = curl_easy_perform(curl_handle);

    if( res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        cleanup(curl_handle, &recv_buf);
        exit(1);
    } else {
	    printf("%lu bytes received in memory %p, seq=%d.\n", \
            recv_buf.size, recv_buf.buf, recv_buf.seq);
    }

    ENTRY item;
    ENTRY *retval;
    item.key = url;
    item.data = "Garbage";
    hsearch_r(item, ENTER, &retval, visited_urls);
    //Data structure for frontier
    struct Queue* url_frontier = createQueue(1000);
    printf("\n");
    //Push seed url to queue
    enqueue(url_frontier, url);

    xmlInitParser();
    //Create threads
    thread_arguments in_params;
    in_params.url_frontier = url_frontier;
    in_params.m = m;

    pthread_t *p_tids = malloc(sizeof(pthread_t) * t);
    for(int i = 0; i < t; ++i){
        pthread_create(p_tids+i, NULL, thread_function, &in_params);
    }

    //Join the threads
    for(int j = 0; j < t; ++j){
        pthread_join(p_tids[j], NULL);
    }  

    ////////////////////////////////////////////////////////////

    //Free variables
    free(url);
    free(p_tids);
    hdestroy_r(visited_urls);
    hdestroy_r(check_duplicate_png);
    fclose(fp);
    fclose(fpng);


    //end time
    times[1] = (tv.tv_sec) + tv.tv_usec/1000000.;
    // printing the difference which is the total time
    printf("paster2 execution time: %f seconds\n", times[1]-times[0]);
    return 0;
}